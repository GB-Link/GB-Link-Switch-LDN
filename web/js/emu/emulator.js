// Game Boy Advance emulator (mGBA's GBA core compiled to WebAssembly, web/emulator/mgba-gba.*)
// running in the page: screen, sound, keys, battery save and the link port, which has the
// wireless adapter or a link cable plugged in.
//
// The game runs by cycle count, in step with real time: paced by the sound output when sound
// is running (the audio thread keeps going in background tabs, which a link needs), otherwise
// by the clock.

export const GBA_KEYS = { A: 1, B: 2, SELECT: 4, START: 8, RIGHT: 16, LEFT: 32, UP: 64, DOWN: 128, R: 256, L: 512 };
export const SCREEN_WIDTH = 240;
export const SCREEN_HEIGHT = 160;
export const GBA_CYCLES_PER_SECOND = 16777216;
export const FRAME_CYCLES = 280896;

const LINK = { nothing: 0, wireless: 1, cable: 2 };
const ADAPTER_BUFFER = 65536;
// The most the game runs at once; a longer stall is skipped, not caught up
const MAX_CATCH_UP_CYCLES = 6 * FRAME_CYCLES;
// Without sound the game runs this far ahead of the clock
const CLOCK_LEAD_CYCLES = FRAME_CYCLES;
// With sound, this much is queued for playback
const AUDIO_LEAD_SECONDS = 0.07;
const SAVE_CHECK_FRAMES = 60;

export class GbaEmulator {
    // base: where mgba-gba.js and mgba-gba.wasm are served.
    static async create(base = new URL('../../emulator/', import.meta.url)) {
        const { default: createGbaEmu } = await import(new URL('mgba-gba.js', base).href);
        return new GbaEmulator(await createGbaEmu({ locateFile: (path) => new URL(path, base).href }));
    }

    constructor(m) {
        this.m = m;
        this.loaded = false;
        this.running = false;
        this.link = 'nothing';
        this.onSaveChange = null;     // (save) once the game has written to its battery save
        this.onFrame = null;          // (frames)
        this.afterRun = null;         // after every stretch the game runs
        this.onReset = null;          // the game was power-cycled
        this.keySources = new Map();
        this.lastSave = null;
        this.canvas = null;
        this.ctx = null;
        this.image = null;
        this.drawnFrame = -1;
        this.reportedFrame = 0;
        this.savedAtFrame = 0;
        this.audioPaced = false;
        this.rafId = null;
        this.timerId = null;
        this.clockStart = 0;
        this.clockCycles = 0;
        this.audio = null;
        this.volume = 1;
        this.transferHandler = null;
        this.onVisibility = () => this.schedule();
        this.adapterIn = m._emu_alloc(ADAPTER_BUFFER);
        this.adapterOut = m._emu_alloc(ADAPTER_BUFFER);
        m.onCableTransfer = (rx) => {
            try {
                this.transferHandler?.(rx);
            } catch (error) {
                console.error('Link cable:', error);
            }
        };
    }

    // Loads a ROM (and battery save) with what is plugged into its link port: 'wireless',
    // 'cable' or 'nothing'. Returns false when the core rejects it.
    load(rom, save = null, link = 'nothing') {
        const m = this.m;
        const romPtr = m._emu_alloc(rom.length);
        m.HEAPU8.set(rom, romPtr);
        let savePtr = 0;
        if (save?.length) {
            savePtr = m._emu_alloc(save.length);
            m.HEAPU8.set(save, savePtr);
        }
        const rc = m._emu_load(romPtr, rom.length, savePtr, save?.length ?? 0);
        if (savePtr) m._emu_free(savePtr);
        this.loaded = rc === 0;
        this.drawnFrame = -1;
        this.reportedFrame = 0;
        this.savedAtFrame = 0;
        if (this.audio) m._emu_set_sample_rate(this.audio.ctx.sampleRate);
        this.applyKeys();
        this.link = this.loaded ? link : 'nothing';
        if (this.loaded) m._emu_link_use(LINK[link] ?? 0);
        this.lastSave = this.loaded ? this.getSave() : null;
        return this.loaded;
    }

    unload() {
        this.stop();
        this.m._emu_unload();
        this.loaded = false;
        this.link = 'nothing';
    }

    // Power-cycles the game and what is plugged into it; the battery save is kept.
    reset() {
        if (!this.loaded) return;
        this.m._emu_reset();
        this.onReset?.();
    }

    get frames() {
        return this.loaded ? this.m._emu_frames() : 0;
    }

    getSave() {
        const m = this.m;
        const size = m._emu_save_size();
        if (size <= 0) return new Uint8Array(0);
        const ptr = m._emu_save_data();
        return m.HEAPU8.slice(ptr, ptr + size);
    }

    read8(address) { return this.m._emu_read8(address); }
    read16(address) { return this.m._emu_read16(address); }
    read32(address) { return this.m._emu_read32(address) >>> 0; }

    // Keys from one input source (keyboard, gamepad, touch); all sources are OR'ed.
    setKeys(source, mask) {
        if (mask) this.keySources.set(source, mask);
        else this.keySources.delete(source);
        this.applyKeys();
    }

    applyKeys() {
        let mask = 0;
        for (const value of this.keySources.values()) mask |= value;
        // Opposite directions at once confuse some games
        const horizontal = GBA_KEYS.LEFT | GBA_KEYS.RIGHT;
        const vertical = GBA_KEYS.UP | GBA_KEYS.DOWN;
        if ((mask & horizontal) === horizontal) mask &= ~horizontal;
        if ((mask & vertical) === vertical) mask &= ~vertical;
        if (this.loaded) this.m._emu_set_keys(mask);
    }

    // ---- the wireless adapter's network side (GB-Link frames, as a GB-Link sends them)

    // The board's SetMode(wireless): a fresh adapter in that role (0 any).
    adapterEnterMode(role) {
        if (this.link === 'wireless') this.m._emu_rfu_enter_mode(role);
    }

    // A data-channel payload from the board.
    adapterFeed(payload) {
        if (this.link !== 'wireless') return;
        for (let at = 0; at < payload.length; at += ADAPTER_BUFFER) {
            const part = payload.subarray(at, at + ADAPTER_BUFFER);
            this.m.HEAPU8.set(part, this.adapterIn);
            this.m._emu_rfu_feed(this.adapterIn, part.length);
        }
    }

    // The adapter's GB-Link frames for the board since the last call, or null.
    adapterTake() {
        if (this.link !== 'wireless') return null;
        const n = this.m._emu_rfu_take(this.adapterOut, ADAPTER_BUFFER);
        return n > 0 ? this.m.HEAPU8.slice(this.adapterOut, this.adapterOut + n) : null;
    }

    // ---- the link cable, driven in the game's time from inside its run

    cablePort() {
        const m = this.m;
        return {
            // role: 0 none, 1 the game is the parent, 2 a child in slot seat (1-3).
            configure: (role, seat, players) => m._emu_cable_config(role, seat, players),
            // The other players' words for the next transfer, by slot (the game's own is ignored).
            stage: (slots) => m._emu_cable_stage(slots[0] ?? 0xffff, slots[1] ?? 0xffff, slots[2] ?? 0xffff, slots[3] ?? 0xffff),
            // A child game: the next transfer starts this many GBA cycles from now.
            clock: (cycles) => m._emu_cable_clock(Math.max(0, Math.round(cycles))),
            // Called after every finished transfer with the game's word (0xFFFF when it did not answer).
            setTransferHandler: (handler) => { this.transferHandler = handler; },
        };
    }

    // ---- clock

    start() {
        if (this.running || !this.loaded) return;
        this.running = true;
        this.resetClock();
        if (typeof document !== 'undefined') document.addEventListener('visibilitychange', this.onVisibility);
        this.schedule();
        if (this.audio) this.audio.ctx.resume().catch(() => {});
    }

    stop() {
        this.running = false;
        if (typeof document !== 'undefined') document.removeEventListener('visibilitychange', this.onVisibility);
        if (this.rafId !== null) cancelAnimationFrame(this.rafId);
        if (this.timerId !== null) clearTimeout(this.timerId);
        this.rafId = null;
        this.timerId = null;
        this.audio?.ctx.suspend().catch(() => {});
    }

    resetClock() {
        this.clockStart = performance.now();
        this.clockCycles = 0;
        if (this.audio) {
            this.audio.pushed = this.audio.taken;
            this.audio.takenAt = performance.now();
        }
    }

    audioClock() {
        return Boolean(this.audio?.ready && this.audio.ctx.state === 'running');
    }

    // Cycles the game should have run by now that it hasn't yet.
    due() {
        const now = performance.now();
        if (this.audioClock()) {
            const a = this.audio;
            const rate = a.ctx.sampleRate;
            // Played since the worklet's last report, estimated from the clock
            const taken = a.taken + Math.min((now - a.takenAt) / 1000, 0.02) * rate;
            return ((taken + AUDIO_LEAD_SECONDS * rate - a.pushed) * GBA_CYCLES_PER_SECOND) / rate;
        }
        return ((now - this.clockStart) * GBA_CYCLES_PER_SECOND) / 1000 + CLOCK_LEAD_CYCLES - this.clockCycles;
    }

    // Runs the game up to the present.
    advance() {
        if (!this.running) return;
        const audioPaced = this.audioClock();
        if (audioPaced !== this.audioPaced) {
            this.audioPaced = audioPaced;
            this.resetClock();
        }
        let due = Math.floor(this.due());
        if (due <= 0) return;
        if (due > MAX_CATCH_UP_CYCLES) {
            due = FRAME_CYCLES;
            this.resetClock();
        }
        while (due > 0 && this.running) {
            const ran = this.m._emu_run(Math.min(due, FRAME_CYCLES));
            this.ran(ran);
            due -= ran;
        }
    }

    ran(cycles) {
        if (cycles <= 0) return;
        this.clockCycles += cycles;
        const m = this.m;
        const samples = m._emu_audio();
        if (samples > 0 && this.audioPaced) this.pushAudio(m._emu_audio_buffer() >> 1, samples);
        this.afterRun?.();
        const frames = m._emu_frames();
        if (frames - this.savedAtFrame >= SAVE_CHECK_FRAMES) {
            this.savedAtFrame = frames;
            this.checkSave();
        }
        if (frames !== this.reportedFrame) {
            this.reportedFrame = frames;
            this.onFrame?.(frames);
        }
    }

    // Reports a changed battery save to onSaveChange now (it is also checked every second).
    checkSave() {
        if (!this.onSaveChange || !this.loaded) return;
        const save = this.getSave();
        const last = this.lastSave;
        if (last && last.length === save.length && last.every((v, i) => v === save[i])) return;
        this.lastSave = save;
        this.onSaveChange(save);
    }

    schedule() {
        if (!this.running) return;
        const hidden = typeof document !== 'undefined' && document.hidden;
        if (typeof requestAnimationFrame === 'function' && !hidden) {
            if (this.rafId === null) this.rafId = requestAnimationFrame(() => { this.rafId = null; this.tick(); });
        } else if (this.timerId === null) {
            this.timerId = setTimeout(() => { this.timerId = null; this.tick(); }, 16);
        }
    }

    tick() {
        if (!this.running) return;
        if (!this.audioClock()) this.advance();
        this.draw();
        this.schedule();
    }

    // ---- screen

    attachCanvas(canvas) {
        this.canvas = canvas;
        this.ctx = canvas ? canvas.getContext('2d') : null;
        if (canvas) {
            canvas.width = SCREEN_WIDTH;
            canvas.height = SCREEN_HEIGHT;
        }
        this.image = null;
        this.drawnFrame = -1;
        this.draw();
    }

    draw() {
        if (!this.ctx || !this.loaded) return;
        const m = this.m;
        const frame = m._emu_frames();
        if (frame === this.drawnFrame) return;
        this.drawnFrame = frame;
        if (!this.image) this.image = this.ctx.createImageData(SCREEN_WIDTH, SCREEN_HEIGHT);
        const base = m._emu_video();
        this.image.data.set(m.HEAPU8.subarray(base, base + SCREEN_WIDTH * SCREEN_HEIGHT * 4));
        this.ctx.putImageData(this.image, 0, 0);
    }

    // ---- sound

    // Starts sound output; call from a user gesture. The game then follows the sound clock.
    async enableAudio(workletUrl = new URL('../../emulator/audio-worklet.js', import.meta.url).href) {
        if (this.audio || typeof AudioContext === 'undefined') {
            if (this.audio && this.running) await this.audio.ctx.resume().catch(() => {});
            return;
        }
        const ctx = new AudioContext({ latencyHint: 'interactive' });
        const gain = ctx.createGain();
        gain.gain.value = this.volume;
        gain.connect(ctx.destination);
        this.audio = { ctx, gain, node: null, ready: false, pushed: 0, taken: 0, takenAt: performance.now(), scratch: new Float32Array(8192) };
        this.m._emu_set_sample_rate(ctx.sampleRate);
        try {
            await ctx.audioWorklet.addModule(workletUrl);
            const node = new AudioWorkletNode(ctx, 'gba-audio', { outputChannelCount: [2] });
            node.port.onmessage = (event) => this.onAudioTaken(event.data);
            node.connect(gain);
            this.audio.node = node;
            this.audio.ready = true;
        } catch (error) {
            console.warn('Emulator sound unavailable:', error);
        }
        // Unless the game was stopped while the worklet loaded
        if (this.running) await ctx.resume().catch(() => {});
        else await ctx.suspend().catch(() => {});
    }

    // 0 mutes the sound; the game keeps following the sound clock.
    setVolume(volume) {
        this.volume = volume;
        if (this.audio) this.audio.gain.gain.value = volume;
    }

    onAudioTaken(taken) {
        const a = this.audio;
        a.taken = Math.max(a.taken, taken);
        a.takenAt = performance.now();
        if (this.running && this.audioClock()) this.advance();
    }

    pushAudio(offset, samples) {
        const a = this.audio;
        if (!a.ready || !a.node) return;
        const n = samples * 2;
        const out = n <= a.scratch.length ? a.scratch.subarray(0, n) : new Float32Array(n);
        const src = this.m.HEAP16;
        for (let i = 0; i < n; i++) out[i] = src[offset + i] / 32768;
        a.node.port.postMessage(out.slice());
        a.pushed += samples;
    }
}
