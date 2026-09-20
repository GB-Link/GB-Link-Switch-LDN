// The page: three steps (ESP32 board, adapter, play) over the device modules. Each of
// the first two cards is drawn from one view of its state: a line of status, perhaps a
// hint, and at most one thing to press. Everything rarely needed sits under a fold.

import { EspDevice, ESP_FILTERS, FAST_BAUD, reopenPort, sleep } from './esp.js';
import { GbLinkSerial, GbLinkUsb, BOOTROM_VENDOR_ID, GBLINK_VENDOR_ID } from './gblink.js';
import { Bridge } from './bridge.js';
import { parseProdKeys } from './keys.js';
import { loadManifest, fetchBytes } from './manifest.js';

const $ = (id) => document.getElementById(id);

const CHIP_NAMES = { esp32: 'ESP32', esp32c3: 'ESP32-C3', esp32c6: 'ESP32-C6', esp32s3: 'ESP32-S3' };
// The board's pins for the standalone link, as [its transmit, to the adapter's GP9; its
// receive, from the adapter's GP8].
const LINK_PINS = { esp32: [17, 16], esp32c3: [5, 4], esp32c6: [2, 1], esp32s3: [2, 1] };
// What dev boards print beside those pins, where that is not the GPIO number.
const LINK_PIN_LABELS = { esp32: ['TX2', 'RX2'] };
const KEY_NAMES = {
    kek: 'aes_kek_generation_source',
    gen: 'aes_key_generation_source',
    master00: 'master_key_00',
    master12: 'master_key_12',
};
const POLL_MS = 5000;

const state = {
    manifest: null,

    esp: null,
    espPort: null,          // a port that was picked but would not attach
    espPhase: 'idle',       // idle | choosing | connecting | installing | starting
    espProblem: null,       // { code, text, hint } left by the last attempt
    espNote: '',            // what the installer is doing
    espProgress: null,
    askForEsp: false,       // a remembered board let us down: offer the list next time
    keys: null,
    keysNote: null,         // { text, tone } about the last thing done with the keys
    replacingKeys: false,
    session: null,
    polls: 0,
    pollTimer: null,

    adapter: null,
    adapterInfo: null,
    adapterPhase: 'idle',   // idle | choosing | connecting
    adapterProblem: null,
    askForAdapter: false,
    bootDevice: null,       // an adapter that was picked while already in its bootloader
    install: null,          // { step, manual, progress, image } while firmware goes on
    customUf2: null,
    uf2Note: null,

    resetLoop: false,       // the adapter reports the game resetting it over and over
    bridge: null,
    bridgeTimer: null,
    bridgeNote: null,
    wiringNote: null,
};

// ---------------------------------------------------------------- small helpers

function setLine(id, text, tone = '') {
    const element = $(id);
    element.textContent = text ?? '';
    element.className = `${element.dataset.base ?? element.className.split(' ')[0]} ${tone}`.trim();
}

function setProgress(id, fraction) {
    const element = $(id);
    if (id === 'esp-progress') element.hidden = fraction === null || fraction === undefined;
    element.firstElementChild.style.width = `${Math.round((fraction ?? 0) * 100)}%`;
}

// Consecutive repeats of a message share one line, with a count.
const logLines = [];
let lastLogged = { key: '', count: 0 };
function log(source, text) {
    const stamp = new Date().toLocaleTimeString([], { hour12: false });
    const key = `${source}\n${text}`;
    if (key === lastLogged.key && logLines.length) {
        lastLogged.count++;
        logLines[logLines.length - 1] = `${stamp}  ${source.padEnd(7)} ${text}  (×${lastLogged.count})`;
    } else {
        lastLogged = { key, count: 1 };
        logLines.push(`${stamp}  ${source.padEnd(7)} ${text}`);
        if (logLines.length > 600) logLines.splice(0, logLines.length - 600);
    }
    const view = $('log');
    const pinned = view.scrollTop + view.clientHeight >= view.scrollHeight - 8;
    view.textContent = logLines.join('\n');
    if (pinned) view.scrollTop = view.scrollHeight;
}

function describe(error) {
    return error?.message || String(error);
}

// Runs one of the browser's device choosers. Resolves with null when nothing was picked.
async function choose(request) {
    try {
        return await request();
    } catch (error) {
        if (error?.name !== 'NotFoundError') log('page', `the browser would not show its device list: ${describe(error)}`);
        return null;
    }
}

function newer(candidate, than) {
    const a = String(candidate).split('.').map(Number);
    const b = String(than).split('.').map(Number);
    for (let i = 0; i < Math.max(a.length, b.length); i++) {
        if ((a[i] ?? 0) !== (b[i] ?? 0)) return (a[i] ?? 0) > (b[i] ?? 0);
    }
    return false;
}

// ---------------------------------------------------------------- ESP32 board: what to show

const ESP_PROBLEMS = {
    'no-firmware': () => ({
        tone: 'warn',
        text: 'This board doesn’t have the bridge firmware yet.',
        primary: ['Install firmware', () => onEspInstall()],
        secondary: ['Pick another port', onEspPickAnother],
    }),
    'download-mode': () => ({
        tone: 'warn',
        text: 'The board is in its bootloader, ready for firmware.',
        hint: 'If it already has the firmware, press its reset (EN) button instead and connect again.',
        primary: ['Install firmware', () => onEspInstall()],
        secondary: ['Connect again', onEspConnect],
    }),
    'crash-loop': (problem) => ({
        tone: 'bad',
        text: 'The firmware on this board crashes as it starts.',
        hint: problem.hint ? `${problem.hint} (also in the log below)` : 'Installing it again usually fixes that.',
        primary: ['Install firmware again', () => onEspInstall()],
    }),
    'port-busy': () => ({
        tone: 'bad',
        text: 'Another program has this port open.',
        hint: 'Close any serial monitor or flashing tool, then try again.',
        primary: ['Try again', onEspConnect],
        secondary: ['Pick another port', onEspPickAnother],
    }),
    'port-lost': () => ({
        tone: 'bad',
        text: 'The browser lost the port as it opened it.',
        hint: 'Unplug the board and plug it back in, then connect again. On Linux this happens after another serial program has used the port.',
        primary: ['Connect', onEspConnect],
    }),
    'install-failed': (problem) => ({
        tone: 'bad',
        text: problem.text,
        hint: problem.hint,
        primary: ['Try again', () => onEspInstall()],
        secondary: ['Pick another port', onEspPickAnother],
    }),
    'silent-after-install': () => ({
        tone: 'warn',
        text: 'Installed, but the board hasn’t answered yet.',
        hint: 'Press its reset button, or unplug it and plug it back in, then connect.',
        primary: ['Connect', onEspConnect],
    }),
    gone: (problem) => ({ tone: problem.tone ?? '', text: problem.text, primary: ['Connect', onEspConnect] }),
};

function espView() {
    if (state.espPhase === 'choosing') {
        return {
            busy: true,
            text: 'Pick the board in the list the browser is showing.',
            hint: 'It is listed as “USB JTAG/serial debug unit”, or as “CP2102” or “CH340” on boards with a separate USB chip.',
        };
    }
    if (state.espPhase === 'connecting') return { busy: true, text: 'Looking for the bridge firmware…' };
    if (state.espPhase === 'installing') return { busy: true, text: state.espNote, progress: state.espProgress };
    if (state.espPhase === 'starting') return { busy: true, text: 'Installed. Waiting for the board to start…' };

    const esp = state.esp;
    if (state.espProblem) {
        const view = (ESP_PROBLEMS[state.espProblem.code] ?? ESP_PROBLEMS.gone)(state.espProblem);
        return { ...view, connected: Boolean(esp) };
    }
    if (!esp) return { text: 'Plug the board into this computer with a USB data cable.', primary: ['Connect', onEspConnect] };
    const name = CHIP_NAMES[esp.info?.chip] ?? esp.info?.chip ?? 'board';
    if (!esp.attached) {
        return { busy: true, connected: true, done: Boolean(state.keys?.complete), text: `${name} · restarting, as it does after every session…` };
    }
    const bundled = state.manifest?.bridge.version;
    if (!esp.info || (bundled && newer(bundled, esp.info.version))) {
        return {
            connected: true,
            tone: 'warn',
            text: esp.info ? `Firmware ${esp.info.version} is installed; ${bundled} is available.` : 'This board’s firmware is older than this page expects.',
            primary: ['Update firmware', () => onEspInstall()],
        };
    }
    if (!state.keys?.complete || state.replacingKeys) {
        return {
            connected: true,
            tone: state.keys?.complete ? '' : 'warn',
            dot: state.keys?.complete ? 'good' : 'warn',
            text: state.keys?.complete ? `${name} connected. Drop a prod.keys to replace the stored keys.` : `${name} connected. It still needs your console’s keys.`,
            keys: true,
        };
    }
    return { connected: true, done: true, tone: 'good', text: `${name} · firmware ${esp.info.version} · keys stored` };
}

// ---------------------------------------------------------------- ESP32 board: doing things

async function onEspConnect() {
    if (state.espPhase !== 'idle' || state.esp) return;
    let port = (await livePort(state.espPort)) ?? (await rememberedEspPort());
    if (!port) {
        state.espPhase = 'choosing';
        render();
        port = await choose(() => EspDevice.requestPort());
        state.espPhase = 'idle';
    }
    if (port) await attachEsp(port);
    else render();
}

async function onEspPickAnother() {
    if (state.espPhase !== 'idle') return;
    state.askForEsp = true;
    state.espPort = null;
    state.espProblem = null;
    await onEspConnect();
}

// A chip with USB of its own comes back from every reset as a new port object under the
// same permission, which leaves the object the page was holding dead.
async function livePort(port) {
    if (!port) return null;
    let ports = [];
    try { ports = await navigator.serial.getPorts(); } catch {}
    if (ports.includes(port)) return port;
    const was = port.getInfo();
    const same = ports.filter((other) => other.getInfo().usbVendorId === was.usbVendorId && other.getInfo().usbProductId === was.usbProductId);
    return same.length === 1 ? same[0] : null;
}

// A board this page was given access to before is used without asking again, as long as
// there is no doubt which one that is.
async function rememberedEspPort() {
    if (state.askForEsp) return null;
    let ports = [];
    try { ports = await navigator.serial.getPorts(); } catch {}
    ports = ports.filter((port) => ESP_FILTERS.some((filter) => filter.usbVendorId === port.getInfo().usbVendorId));
    return ports.length === 1 ? ports[0] : null;
}

async function attachEsp(port) {
    state.espPhase = 'connecting';
    state.espProblem = null;
    render();
    const device = new EspDevice();
    try {
        await device.open(port);
    } catch (error) {
        await device.close();
        state.espPort = error.code === 'port-lost' ? null : port;
        state.espProblem = { code: error.code ?? 'no-firmware', text: describe(error), hint: error.detail };
        if (error.code === 'crash-loop') log('board', `crashing at start-up: ${error.detail || 'no detail captured'}`);
        state.espPhase = 'idle';
        render();
        return;
    }
    state.askForEsp = false;
    state.espPhase = 'idle';
    await adoptEsp(device);
}

async function adoptEsp(device) {
    state.esp = device;
    state.espPort = null;
    state.espProblem = null;
    device.addEventListener('log', (event) => onEspLine(event.detail));
    device.addEventListener('restarted', () => {
        if (state.esp !== device) return;
        log('board', 'restarted');
        lastRoomLine = '';
        state.session = null;
        render();
    });
    device.addEventListener('reattached', () => { if (state.esp === device) refreshEsp(); });
    device.addEventListener('failed', (event) => { if (state.esp === device) dropEsp({ code: 'gone', tone: 'bad', text: `The board stopped answering (${describe(event.detail)}).` }); });
    device.addEventListener('disconnected', () => { if (state.esp === device) dropEsp({ code: 'gone', tone: 'warn', text: 'The board was unplugged.' }); });
    await refreshEsp();
    clearInterval(state.pollTimer);
    state.pollTimer = setInterval(pollSession, POLL_MS);
}

async function refreshEsp() {
    const esp = state.esp;
    if (!esp) return;
    const info = esp.info;
    const bundled = state.manifest?.bridge.version;
    $('esp-chip').textContent = CHIP_NAMES[info?.chip] ?? info?.chip ?? 'Unknown';
    $('esp-version').textContent = !info ? 'Older than 2.0' : bundled && newer(bundled, info.version) ? `${info.version} (${bundled} available)` : info.version;
    $('esp-transport').textContent = info ? (info.transport === 'UART' ? `UART, ${esp.baudRate} baud` : info.transport) : '–';
    try {
        // Left over from a page that was closed while it carried the link.
        if (!state.bridge && (await esp.adapterPort()) === 'host') await esp.setAdapterPort('uart');
        await refreshKeys();
    } catch (error) {
        log('page', describe(error));
    }
    render();
    pollSoon();
}

async function dropEsp(problem = null) {
    const device = state.esp;
    if (state.bridge) await stopBridge('The ESP32 board went away.');
    clearInterval(state.pollTimer);
    state.esp = null;
    state.keys = null;
    state.keysNote = null;
    state.replacingKeys = false;
    state.session = null;
    state.espProblem = problem;
    await device?.close();
    render();
}

// The board repeats what it hears while it scans: the raw advertisement four times a
// second, and the room it decodes from it. The log keeps the room, once per change.
let lastRoomLine = '';
function onEspLine(line) {
    if (line.startsWith('LDN_HELLO') || line.startsWith('LDN_ADV ')) return;
    if (line.startsWith('LDN_ROOM ')) {
        if (line === lastRoomLine) return;
        lastRoomLine = line;
    }
    log('board', line);
    if (line === 'LDN_BRIDGE no keys') refreshKeys();
    if (line.startsWith('LDN_BRIDGE ')) pollSoon();
}

let pollSoonTimer = null;
function pollSoon() {
    clearTimeout(pollSoonTimer);
    pollSoonTimer = setTimeout(pollSession, 400);
}

async function onEspInstall(eraseAll = false) {
    if (state.espPhase !== 'idle') return;
    if (!state.manifest) {
        state.espProblem = { code: 'install-failed', text: 'The firmware bundled with this page could not be loaded.' };
        render();
        return;
    }
    let port = state.esp?.port ?? (await livePort(state.espPort)) ?? (await rememberedEspPort());
    if (!port) {
        state.espPhase = 'choosing';
        render();
        port = await choose(() => EspDevice.requestPort());
        state.espPhase = 'idle';
        if (!port) { render(); return; }
    }
    $('esp-more').open = false;
    state.espPhase = 'installing';
    state.espNote = 'Preparing…';
    state.espProgress = 0;
    render();
    try {
        if (state.esp) await dropEsp();
        state.espProblem = null;
        const { flashBridge } = await import('./flash-esp.js');
        const done = await flashBridge(port, state.manifest, {
            eraseAll,
            onStatus: (text) => { state.espNote = text; render(); },
            onProgress: (fraction) => { state.espProgress = fraction; setProgress('esp-progress', fraction); },
            onLog: (text) => log('flasher', text),
        });
        log('page', `firmware ${done.version} installed on the ${done.chip}`);
        state.espPhase = 'starting';
        state.espProgress = null;
        render();
        let device = null;
        try { device = await reopenPort(port); } catch (error) { log('page', describe(error)); }
        state.espPhase = 'idle';
        if (device) await adoptEsp(device);
        else {
            state.espPort = port;
            state.espProblem = { code: 'silent-after-install' };
            render();
        }
    } catch (error) {
        log('flasher', describe(error));
        state.espPort = port;
        state.espPhase = 'idle';
        state.espProgress = null;
        state.espProblem = { code: 'install-failed', ...installAdvice(error) };
        render();
    }
}

function installAdvice(error) {
    const text = describe(error);
    if (/device has been lost/i.test(text)) {
        return { text: 'The browser lost the port as it opened it.', hint: 'Unplug the board and plug it back in, then try again.' };
    }
    if (/failed to connect|timed? ?out|no serial data|invalid head/i.test(text)) {
        return { text: 'The chip did not enter its bootloader.', hint: 'Hold the BOOT button, press and release RESET (or plug the board in with BOOT held), then try again.' };
    }
    if (/failed to open|already open/i.test(text)) return { text: 'Another program has this port open.', hint: 'Close it, then try again.' };
    return { text: 'Installing failed.', hint: text };
}

// Two clicks rather than a dialog: the first arms the button for a few seconds.
const armed = new Map();   // button -> { label, timer }
function twice(button, warning, action) {
    if (!armed.has(button)) {
        armed.set(button, { label: button.textContent, timer: setTimeout(() => disarm(button), 6000) });
        button.textContent = warning;
        return;
    }
    disarm(button);
    action();
}

function disarm(button) {
    const pending = armed.get(button);
    if (!pending) return;
    clearTimeout(pending.timer);
    armed.delete(button);
    button.textContent = pending.label;
}

// ---------------------------------------------------------------- keys

async function refreshKeys() {
    const esp = state.esp;
    if (!esp?.attached) return;
    state.keys = await esp.keyStatus();
    render();
}

async function onKeysFile(file) {
    const esp = state.esp;
    if (!file || !esp?.attached) return;
    const note = (text, tone) => { state.keysNote = { text, tone }; render(); };
    if (file.size > 1024 * 1024) { note('That file is too large to be a prod.keys.', 'bad'); return; }
    const parsed = parseProdKeys(await file.text());
    if (parsed.missing.length || parsed.malformed.length) {
        const problems = [];
        if (parsed.missing.length) problems.push(`not in the file: ${parsed.missing.join(', ')}`);
        if (parsed.malformed.length) problems.push(`not 32 hex digits: ${parsed.malformed.join(', ')}`);
        note(`That file cannot be used (${problems.join('; ')}).`, 'bad');
        return;
    }
    note('Storing the keys on the board…');
    try {
        const rejected = await esp.storeKeys(parsed.keys);
        if (rejected.length) { note(`The board did not accept: ${rejected.join(', ')}.`, 'bad'); return; }
        state.replacingKeys = false;
        state.keysNote = null;
        await refreshKeys();
        if (state.keys?.complete) {
            await esp.startBridge();
            log('page', 'keys stored; the board is looking for a room');
            pollSoon();
        }
    } catch (error) {
        note(`The keys could not be stored (${describe(error)}).`, 'bad');
    }
}

async function onKeysErase() {
    const esp = state.esp;
    if (!esp?.attached) return;
    try {
        await esp.eraseKeys();
        log('page', 'keys erased from the board');
        await refreshKeys();
    } catch (error) {
        state.keysNote = { text: describe(error), tone: 'bad' };
        render();
    }
}

function keysLine() {
    if (state.keysNote) return state.keysNote;
    const keys = state.keys;
    if (!keys || keys.complete) return { text: '' };
    const missing = Object.entries(KEY_NAMES).filter(([flag]) => !keys[flag]).map(([, name]) => name);
    return missing.length && missing.length < 4 ? { text: `Still missing: ${missing.join(', ')}.`, tone: 'warn' } : { text: '' };
}

// ---------------------------------------------------------------- session status

async function pollSession() {
    const esp = state.esp;
    if (!esp?.attached || state.espPhase !== 'idle') return;
    try {
        state.session = await esp.bridgeStatus();
        if (state.polls++ % 3 === 0) {
            const radio = await esp.radio();
            $('esp-signal').textContent = radio && radio.frames > 0 ? `${radio.average} dBm (${signalWord(radio.average)})` : 'No room in range';
        }
    } catch {
        return;   // restarting; the next poll will do
    }
    renderSession();
}

function signalWord(dbm) {
    return dbm >= -55 ? 'strong' : dbm >= -70 ? 'good' : dbm >= -80 ? 'weak' : 'very weak';
}

function renderSession() {
    const status = state.session;
    const box = $('session');
    box.hidden = !state.esp;
    if (!state.esp) { $('play-dot').className = 'dot'; return; }
    let headline = 'Waiting for the board…';
    let hint = '';
    let tone = '';
    if (state.resetLoop && state.adapter) {
        headline = 'The game keeps restarting the wireless adapter.';
        hint = 'On the GBA this looks like a freeze. It usually comes down to the link cable: try another one, and make sure the adapter has the firmware from step 2.';
        tone = 'warn';
    } else if (state.keys && !state.keys.complete) {
        headline = 'The board needs its keys.';
        hint = 'Without them it cannot read the Switch\'s wireless. Add your prod.keys in step 1.';
        tone = 'warn';
    } else if (status) {
        const running = status.state === 'run';
        if (status.state === 'stopped' || status.state === 'idle') {
            headline = 'The bridge is stopped.';
            hint = 'Unplug the board and plug it back in.';
            tone = 'warn';
        } else if (status.state === 'scan') {
            headline = 'Looking for a FireRed or LeafGreen room…';
            hint = 'On the Switch, open the Trade Center or Colosseum as the group leader.';
        } else if (!running) {
            headline = 'Joining the Switch’s room…';
        } else if (status.child === '1') {
            headline = 'The Game Boy Advance and the Switch are linked.';
            hint = 'Leave the room on both consoles when you are done; the board then gets ready for the next one.';
            tone = 'good';
        } else if (status.conn_state === '2') {
            headline = 'In the Switch’s room. Waiting for the Game Boy Advance.';
            hint = 'On the GBA, choose the same activity and join the group.';
            tone = 'good';
        } else {
            headline = 'In the Switch’s room, setting up the session…';
        }
    }
    $('session-headline').textContent = headline;
    $('session-hint').textContent = hint;
    box.className = `session ${tone}`.trim();
    $('play-dot').className = `dot ${tone === 'good' ? 'good' : tone === 'warn' ? 'warn' : 'busy'}`;
}

// ---------------------------------------------------------------- adapter: what to show

const ADAPTER_PROBLEMS = {
    denied: () => ({
        tone: 'bad',
        text: 'The browser was refused access to the adapter.',
        hint: 'Close other pages or programs that use it. On Linux it also needs a udev rule; connecting over serial, under More options, works without one.',
        primary: ['Try again', () => onAdapterConnect()],
    }),
    'port-lost': () => ({
        tone: 'bad',
        text: 'The browser lost the port as it opened it.',
        hint: 'Unplug the adapter and plug it back in, then connect again.',
        primary: ['Connect', () => onAdapterConnect()],
    }),
    'install-failed': (problem) => ({
        tone: 'bad',
        text: 'Installing failed.',
        hint: problem.hint,
        primary: ['Try again', onAdapterInstall],
    }),
    'no-webusb': () => ({
        tone: 'warn',
        text: 'This browser cannot reach the adapter’s bootloader.',
        hint: 'Use Chrome or Edge, or install by hand as described under More options.',
    }),
    cancelled: () => ({
        text: 'Cancelled.',
        hint: 'If the adapter is still in update mode, unplug it and plug it back in to use it as it was.',
        primary: ['Connect', () => onAdapterConnect()],
    }),
    gone: (problem) => ({ tone: problem.tone ?? '', text: problem.text, hint: problem.hint, primary: ['Connect', () => onAdapterConnect()] }),
};

function adapterView() {
    if (state.install) {
        const waiting = state.install.step === 'restart' || state.install.step === 'choose';
        return { busy: true, steps: true, text: 'Installing the firmware…', secondary: waiting ? ['Cancel', cancelAdapterInstall] : null };
    }
    if (state.adapterPhase === 'choosing') {
        return { busy: true, text: 'Pick the adapter in the list the browser is showing.', hint: 'It is listed as “GBLink USB”.' };
    }
    if (state.adapterPhase === 'connecting') return { busy: true, text: 'Connecting…' };

    const adapter = state.adapter;
    if (state.adapterProblem) {
        const view = (ADAPTER_PROBLEMS[state.adapterProblem.code] ?? ADAPTER_PROBLEMS.gone)(state.adapterProblem);
        return { ...view, connected: Boolean(adapter) };
    }
    if (state.bootDevice) {
        return { tone: 'warn', text: 'The adapter is in update mode, ready for firmware.', primary: [installLabel(), onAdapterInstall] };
    }
    if (!adapter) return { text: 'Plug the adapter into this computer.', primary: ['Connect', () => onAdapterConnect()] };
    const info = state.adapterInfo;
    const bundled = state.manifest?.adapter.version;
    if (!info?.wireless) {
        return { connected: true, tone: 'warn', text: 'This adapter’s firmware doesn’t have the wireless adapter mode yet.', primary: [installLabel(), onAdapterInstall] };
    }
    if (bundled && info.version && newer(bundled, info.version)) {
        return { connected: true, tone: 'warn', text: `Firmware ${info.version} is installed; ${bundled} is available.`, primary: ['Update firmware', onAdapterInstall] };
    }
    if (state.customUf2) {
        return { connected: true, text: `${state.customUf2.name} is ready to install.`, primary: [installLabel(), onAdapterInstall] };
    }
    return { connected: true, done: true, tone: 'good', text: `GB-Link · firmware ${info.version ?? 'unknown'} · wireless adapter mode` };
}

function installLabel() {
    return state.customUf2 ? `Install ${state.customUf2.name}` : 'Install firmware';
}

// ---------------------------------------------------------------- adapter: doing things

async function onAdapterConnect(kind = GbLinkUsb.available() ? 'usb' : 'serial') {
    if (state.adapterPhase !== 'idle' || state.adapter || state.install) return;
    state.adapterProblem = null;
    if (kind === 'usb') {
        let device = await rememberedAdapter();
        if (!device) {
            state.adapterPhase = 'choosing';
            render();
            device = await choose(() => GbLinkUsb.requestDevice());
            state.adapterPhase = 'idle';
        }
        if (!device) { render(); return; }
        if (device.vendorId === BOOTROM_VENDOR_ID) {
            state.bootDevice = device;
            render();
            return;
        }
        await openAdapter(new GbLinkUsb(), device);
    } else {
        state.adapterPhase = 'choosing';
        render();
        const port = await choose(() => GbLinkSerial.requestPort());
        state.adapterPhase = 'idle';
        if (port) await openAdapter(new GbLinkSerial(), port);
        else render();
    }
}

async function rememberedAdapter() {
    if (state.askForAdapter) return null;
    let devices = [];
    try { devices = await navigator.usb.getDevices(); } catch {}
    devices = devices.filter((device) => device.vendorId === GBLINK_VENDOR_ID);
    return devices.length === 1 ? devices[0] : null;
}

async function openAdapter(adapter, handle) {
    state.adapterPhase = 'connecting';
    state.bootDevice = null;
    render();
    try {
        await adapter.open(handle);
        const info = await adapter.identify();
        state.adapter = adapter;
        state.adapterInfo = info;
        state.askForAdapter = false;
        state.adapterProblem = null;
        adapter.addEventListener('disconnected', () => { if (state.adapter === adapter) dropAdapter({ code: 'gone', tone: 'warn', text: 'The adapter was unplugged.' }); });
        adapter.addEventListener('resetloop', (event) => { if (state.adapter === adapter) onResetLoop(event.detail); });
        $('adapter-version').textContent = info.version ?? 'Unknown';
        $('adapter-wireless').textContent = info.wireless ? 'Yes' : 'No';
        $('adapter-kind').textContent = adapter.kind === 'usb' ? 'WebUSB' : 'Serial';
    } catch (error) {
        await adapter.close();
        state.askForAdapter = true;
        const denied = error?.name === 'SecurityError' || /access denied/i.test(describe(error));
        state.adapterProblem = denied ? { code: 'denied' }
            : error.code === 'port-lost' ? { code: 'port-lost' }
            : { code: 'gone', tone: 'bad', text: 'The adapter could not be opened.', hint: describe(error) };
    } finally {
        state.adapterPhase = 'idle';
        render();
    }
}

// The adapter noticed the game resetting it over and over. Shown where the player is
// looking, because from the GBA's side it is a freeze with no message.
function onResetLoop({ looping, startedUp }) {
    state.resetLoop = looping;
    if (looping) log('adapter', `the game keeps restarting the wireless adapter (${startedUp ? 'its commands are not getting through' : 'the adapter is not being recognised'})`);
    renderSession();
}

async function dropAdapter(problem = null) {
    const adapter = state.adapter;
    if (state.bridge) await stopBridge('The adapter went away.');
    state.adapter = null;
    state.adapterInfo = null;
    state.bootDevice = null;
    state.resetLoop = false;
    state.adapterProblem = problem;
    await adapter?.close();
    render();
}

async function adapterImage() {
    if (state.customUf2) return state.customUf2.image;
    const { parseUf2 } = await import('./flash-pico.js');
    return parseUf2(await fetchBytes(state.manifest.base + state.manifest.adapter.path));
}

// Firmware goes on through the adapter's USB bootloader, which is a different USB device
// from the running adapter: the browser has to be given access a second time. The steps
// on the card walk through that.
async function onAdapterInstall() {
    if (state.install || state.adapterPhase !== 'idle') return;
    if (!GbLinkUsb.available()) {
        $('adapter-more').open = true;
        state.adapterProblem = { code: 'no-webusb' };
        render();
        return;
    }
    state.adapterProblem = null;
    $('adapter-more').open = false;
    let image;
    try {
        if (!state.manifest && !state.customUf2) throw new Error('the firmware bundled with this page could not be loaded');
        image = await adapterImage();
    } catch (error) {
        state.adapterProblem = { code: 'install-failed', hint: describe(error) };
        render();
        return;
    }
    if (state.bootDevice) {
        const { bootloaderFrom } = await import('./flash-pico.js');
        state.install = { step: 'write', manual: false, image };
        await flashBootloader(bootloaderFrom(state.bootDevice));
        return;
    }
    const adapter = state.adapter;
    const install = (state.install = { step: 'restart', manual: !adapter, image });
    render();
    if (!adapter) return;
    if (state.bridge) await stopBridge();
    // Let go of it first: it drops off the bus as it restarts, which is not an unplugging.
    state.adapter = null;
    state.adapterInfo = null;
    try {
        await adapter.rebootToBootloader();
        await sleep(200);
    } catch (error) {
        log('page', `the adapter would not restart: ${describe(error)}`);
        install.manual = true;
    }
    await adapter.close();
    if (state.install === install && !install.started) install.step = install.manual ? 'restart' : 'choose';
    render();
}

function cancelAdapterInstall() {
    const install = state.install;
    if (!install || install.started) return;
    state.install = null;
    state.adapterProblem = install.manual ? null : { code: 'cancelled' };
    render();
}

// A bootloader this page already has permission for announces itself; otherwise the
// button opens the chooser.
async function onUsbConnect(event) {
    const install = state.install;
    if (event.device?.vendorId !== BOOTROM_VENDOR_ID || !install || install.started) return;
    const { bootloaderFrom } = await import('./flash-pico.js');
    let picoboot;
    try { picoboot = bootloaderFrom(event.device); } catch { return; }
    await flashBootloader(picoboot);
}

async function onAdapterSelect() {
    const install = state.install;
    if (!install || install.started) return;
    const { chooseBootloader } = await import('./flash-pico.js');
    let picoboot;
    try {
        picoboot = await chooseBootloader();
    } catch (error) {
        if (error?.name !== 'NotFoundError') log('page', describe(error));
        return;
    }
    await flashBootloader(picoboot);
}

async function flashBootloader(picoboot) {
    const install = state.install;
    if (!install || install.started) return;
    install.started = true;
    install.step = 'write';
    render();
    try {
        const { flashAdapter } = await import('./flash-pico.js');
        await flashAdapter(picoboot, install.image, {
            onStatus: (text) => log('flasher', text),
            onProgress: (fraction) => setProgress('adapter-progress', fraction),
        });
        log('page', `adapter firmware installed: ${state.customUf2?.name ?? `bundled ${state.manifest?.adapter.version ?? ''}`.trim()}`);
        state.bootDevice = null;
        state.customUf2 = null;
        $('adapter-file').value = '';
        install.step = 'reconnect';
        render();
        await reconnectAdapter();
    } catch (error) {
        log('page', `adapter install failed: ${describe(error)}`);
        state.install = null;
        state.adapterProblem = { code: 'install-failed', hint: describe(error) };
        render();
    }
}

// The running firmware is a different USB device from the bootloader; permission for it
// exists if it was connected here before.
async function reconnectAdapter() {
    for (let attempt = 0; attempt < 8; attempt++) {
        await sleep(700);
        let devices = [];
        try { devices = await navigator.usb.getDevices(); } catch {}
        const device = devices.find((candidate) => candidate.vendorId === GBLINK_VENDOR_ID);
        let ports = [];
        try { ports = (await navigator.serial?.getPorts()) ?? []; } catch {}
        const port = ports.find((candidate) => candidate.getInfo().usbVendorId === GBLINK_VENDOR_ID);
        if (device || (port && attempt >= 3)) {
            await openAdapter(device ? new GbLinkUsb() : new GbLinkSerial(), device ?? port);
            state.install = null;
            render();
            return;
        }
    }
    state.install = null;
    state.adapterProblem = { code: 'gone', tone: 'good', text: 'Installed. Connect the adapter to carry on.' };
    render();
}

async function onAdapterFile(file) {
    if (!file) return;
    try {
        const { parseUf2 } = await import('./flash-pico.js');
        state.customUf2 = { name: file.name, image: parseUf2(new Uint8Array(await file.arrayBuffer())) };
        state.adapterProblem = null;
        $('adapter-more').open = false;
        state.uf2Note = null;
    } catch (error) {
        state.customUf2 = null;
        $('adapter-file').value = '';
        state.uf2Note = `That is not a usable .uf2 file (${describe(error)}).`;
    }
    render();
}

// ---------------------------------------------------------------- play

function bridgeBlocker() {
    if (!state.esp?.attached) return 'Connect the ESP32 board in step 1.';
    if (!state.adapter) return 'Connect the adapter in step 2.';
    if (state.esp.info?.transport === 'UART' && state.esp.baudRate < FAST_BAUD) return 'This board’s firmware runs its console at 115200 baud, which cannot carry the link. Update it in step 1.';
    if (!state.adapterInfo?.wireless) return 'The adapter needs the firmware from step 2.';
    if (!state.keys?.complete) return 'The board needs its keys from step 1.';
    return null;
}

async function onBridgeStart() {
    if (state.bridge || bridgeBlocker()) return;
    const bridge = new Bridge(state.esp, state.adapter);
    bridge.addEventListener('failed', (event) => stopBridge(`Stopped: ${describe(event.detail)}`, 'bad'));
    state.bridgeNote = null;
    setLine('bridge-status', 'Starting…');
    try {
        await bridge.start();
    } catch (error) {
        state.bridgeNote = { text: describe(error), tone: 'bad' };
        render();
        return;
    }
    state.bridge = bridge;
    state.bridgeTimer = setInterval(renderBridge, 1000);
    log('page', 'carrying the link between the boards');
    render();
    renderBridge();
    pollSoon();
}

async function stopBridge(message = null, tone = '') {
    const bridge = state.bridge;
    if (!bridge) return;
    state.bridge = null;
    clearInterval(state.bridgeTimer);
    await bridge.stop();
    state.bridgeNote = message ? { text: message, tone } : null;
    log('page', 'no longer carrying the link');
    render();
}

function renderBridge() {
    const stats = state.bridge?.stats;
    if (!stats) return;
    $('bridge-out').textContent = `${stats.toAdapterFrames.toLocaleString()} ${stats.toAdapterFrames === 1 ? 'frame' : 'frames'}`;
    $('bridge-in').textContent = `${(stats.fromAdapterBytes / 1024).toFixed(1)} KB`;
    $('bridge-sessions').textContent = String(stats.reattached);
}

// The adapter answers whichever side spoke to it last. One that is also connected to
// this page has been answering over USB, so a silent first listen is followed by asking
// the board to speak on the wires again, which is left alone while a GBA is linked.
async function onWiringCheck() {
    const esp = state.esp;
    if (!esp?.attached || state.bridge) return;
    const note = (text, tone) => { state.wiringNote = { text, tone }; render(); };
    note('Listening on the wires…');
    const frames = async () => {
        const lines = await esp.command('LDN_PICO_STATS');
        const match = lines.join(' ').match(/rx_frames=(\d+)/);
        return match ? Number(match[1]) : null;
    };
    const heard = async (milliseconds) => {
        const before = await frames();
        await sleep(milliseconds);
        const after = await frames();
        return before === null || after === null ? null : after > before;
    };
    try {
        let result = await heard(2000);
        if (result === false && state.session?.child !== '1') {
            await esp.command('LDN_PICO_MODE');
            result = await heard(1500);
        }
        if (result === null) note('The board did not report on its link.', 'warn');
        else if (result) note('The board hears the adapter over the wires.', 'good');
        else note('Nothing is arriving from the adapter. The two link wires may be the wrong way round: swap them and check again. Otherwise check ground, and that the adapter has power and the firmware from step 2.', 'bad');
    } catch (error) {
        note(describe(error), 'bad');
    }
}

// ---------------------------------------------------------------- rendering

let espActions = {};
let adapterActions = {};

function drawCard(prefix, view) {
    const card = $(`${prefix}-card`);
    card.classList.toggle('done', Boolean(view.done));
    $(`${prefix}-dot`).className = `dot ${view.busy ? 'busy' : view.dot ?? view.tone ?? ''}`.trim();
    setLine(`${prefix}-status`, view.text, view.tone);
    $(`${prefix}-hint`).textContent = view.hint ?? '';
    const primary = $(`${prefix}-primary`);
    primary.hidden = !view.primary;
    if (view.primary && !armed.has(primary)) primary.textContent = view.primary[0];
    const secondary = $(`${prefix}-secondary`);
    secondary.hidden = !view.secondary;
    if (view.secondary) secondary.textContent = view.secondary[0];
    $(`${prefix}-actions`).classList.toggle('empty', !view.primary && !view.secondary);
    return { primary: view.primary?.[1], secondary: view.secondary?.[1] };
}

function render() {
    const esp = espView();
    espActions = drawCard('esp', esp);
    setProgress('esp-progress', esp.progress ?? null);
    $('keys-panel').hidden = !esp.keys;
    const keys = keysLine();
    setLine('keys-status', keys.text, keys.tone);
    $('esp-details').hidden = !esp.connected;
    $('esp-more').hidden = !esp.connected || esp.busy;
    $('keys-replace').hidden = !state.keys?.complete;
    if (!armed.has($('keys-replace'))) $('keys-replace').textContent = state.replacingKeys ? 'Keep the stored keys' : 'Replace the keys';
    $('keys-erase').hidden = !state.keys || !Object.keys(KEY_NAMES).some((flag) => state.keys[flag]);

    const adapter = adapterView();
    adapterActions = drawCard('adapter', adapter);
    $('adapter-details').hidden = !adapter.connected;
    $('adapter-more').hidden = Boolean(adapter.busy);
    $('adapter-reinstall').hidden = !adapter.done;
    $('adapter-disconnect').hidden = !state.adapter && !state.bootDevice;
    $('adapter-connect-serial').hidden = Boolean(state.adapter) || !GbLinkSerial.available() || !GbLinkUsb.available();
    $('adapter-file-clear').hidden = !state.customUf2;
    setLine('adapter-file-status', state.uf2Note, 'bad');
    $('adapter-bootsel').hidden = !state.adapter;
    drawSteps();

    const pins = LINK_PINS[state.esp?.info?.chip];
    const labels = LINK_PIN_LABELS[state.esp?.info?.chip];
    const pin = (index) => `GPIO${pins[index]}${labels ? ` (${labels[index]})` : ''}`;
    $('wiring').hidden = Boolean(pins);
    $('wires').hidden = !pins;
    if (pins) {
        $('wire-tx').textContent = pin(0);
        $('wire-rx').textContent = pin(1);
    }
    $('wiring-check').disabled = !state.esp?.attached || Boolean(state.bridge);
    setLine('wiring-status', state.wiringNote?.text, state.wiringNote?.tone);

    const blocker = bridgeBlocker();
    $('bridge-start').hidden = Boolean(state.bridge);
    $('bridge-start').disabled = Boolean(blocker);
    $('bridge-stop').hidden = !state.bridge;
    $('bridge-facts').hidden = !state.bridge;
    if (state.bridge) setLine('bridge-status', 'Carrying the link. Keep this tab open and in view.', 'good');
    else if (state.bridgeNote) setLine('bridge-status', state.bridgeNote.text, state.bridgeNote.tone);
    else setLine('bridge-status', blocker ?? 'Ready.');
    renderSession();
}

function drawSteps() {
    const list = $('adapter-steps');
    const install = state.install;
    list.hidden = !install;
    if (!install) return;
    const order = ['restart', 'choose', 'write', 'reconnect'];
    const at = order.indexOf(install.step);
    $('adapter-step-restart').textContent = install.manual
        ? 'Hold the adapter’s BOOTSEL button while plugging it in'
        : 'Restart the adapter in update mode';
    for (const item of list.children) {
        const index = order.indexOf(item.dataset.step);
        // By hand, the first two steps are both the user's and both wait on the button.
        const current = index === at || (install.manual && install.step === 'restart' && index <= 1);
        item.className = current ? 'current' : index < at ? 'done' : '';
    }
}

// ---------------------------------------------------------------- start-up

function wireUp() {
    for (const id of ['esp-status', 'keys-status', 'adapter-status', 'adapter-file-status', 'bridge-status', 'wiring-status']) $(id).dataset.base = 'status';

    $('esp-primary').addEventListener('click', () => espActions.primary?.());
    $('esp-secondary').addEventListener('click', () => espActions.secondary?.());
    $('esp-reinstall').addEventListener('click', () => onEspInstall());
    $('esp-wipe').addEventListener('click', (event) => twice(event.currentTarget, 'Click again: this erases the keys too', () => onEspInstall(true)));
    $('esp-disconnect').addEventListener('click', () => dropEsp());
    $('keys-replace').addEventListener('click', () => { state.replacingKeys = !state.replacingKeys; state.keysNote = null; render(); });
    $('keys-erase').addEventListener('click', (event) => twice(event.currentTarget, 'Click again to erase the keys', onKeysErase));
    $('keys-file').addEventListener('change', (event) => {
        onKeysFile(event.target.files[0]);
        event.target.value = '';
    });
    const drop = $('keys-drop');
    for (const name of ['dragenter', 'dragover']) drop.addEventListener(name, (event) => { event.preventDefault(); drop.classList.add('over'); });
    for (const name of ['dragleave', 'drop']) drop.addEventListener(name, (event) => { event.preventDefault(); drop.classList.remove('over'); });
    drop.addEventListener('drop', (event) => onKeysFile(event.dataTransfer?.files?.[0]));
    // A file dropped beside the target would otherwise replace the page.
    for (const name of ['dragover', 'drop']) window.addEventListener(name, (event) => event.preventDefault());

    $('adapter-primary').addEventListener('click', () => adapterActions.primary?.());
    $('adapter-secondary').addEventListener('click', () => adapterActions.secondary?.());
    $('adapter-select').addEventListener('click', onAdapterSelect);
    $('adapter-reinstall').addEventListener('click', onAdapterInstall);
    $('adapter-connect-serial').addEventListener('click', () => onAdapterConnect('serial'));
    $('adapter-disconnect').addEventListener('click', () => dropAdapter());
    $('adapter-file').addEventListener('change', (event) => onAdapterFile(event.target.files[0]));
    $('adapter-file-clear').addEventListener('click', () => {
        state.customUf2 = null;
        $('adapter-file').value = '';
        render();
    });
    $('adapter-bootsel').addEventListener('click', async () => {
        const adapter = state.adapter;
        if (!adapter) return;
        if (state.bridge) await stopBridge();
        await adapter.rebootToBootloader();
        await sleep(200);
        await dropAdapter({ code: 'gone', text: 'The adapter is restarting in update mode; an RPI-RP2 drive should appear.' });
    });

    $('bridge-start').addEventListener('click', onBridgeStart);
    $('bridge-stop').addEventListener('click', () => stopBridge());
    $('wiring-check').addEventListener('click', onWiringCheck);

    $('log-clear').addEventListener('click', () => {
        logLines.length = 0;
        lastLogged = { key: '', count: 0 };
        $('log').textContent = '';
    });
    $('log-copy').addEventListener('click', () => navigator.clipboard?.writeText(logLines.join('\n')));

    navigator.usb?.addEventListener('connect', onUsbConnect);
    // Hand the adapter port back if the page goes away while it carries the link.
    window.addEventListener('pagehide', () => { if (state.bridge) state.bridge.stop(); });
}

async function start() {
    wireUp();
    const serial = EspDevice.available();
    if (!serial || !window.isSecureContext) {
        const notice = $('unsupported');
        notice.hidden = false;
        notice.textContent = serial
            ? 'Browsers only allow access to USB devices from https:// pages or from localhost.'
            : 'This browser has no Web Serial, which this page needs to reach the boards. Use Chrome, Edge or another Chromium browser on a computer.';
    }
    try {
        state.manifest = await loadManifest();
        const adapter = state.manifest.adapter;
        $('adapter-download').href = state.manifest.base + adapter.path;
        $('adapter-download').textContent = `the firmware file (${adapter.version})`;
    } catch (error) {
        log('page', describe(error));
    }
    render();
    if (!serial) for (const id of ['esp-primary', 'adapter-primary']) $(id).disabled = true;
}

start();
