// Keyboard, gamepad and on-screen touch controls for the emulator. Keys are only taken while
// isActive() says so and the focus is not in a text field. Touch controls are the elements
// with a data-key attribute (A, B, L, R, START, SELECT, UP, DOWN, LEFT, RIGHT) inside
// touchArea.

import { GBA_KEYS } from './emulator.js';

const KEYBOARD = {
    ArrowUp: GBA_KEYS.UP, ArrowDown: GBA_KEYS.DOWN, ArrowLeft: GBA_KEYS.LEFT, ArrowRight: GBA_KEYS.RIGHT,
    KeyX: GBA_KEYS.A, KeyZ: GBA_KEYS.B, KeyA: GBA_KEYS.L, KeyS: GBA_KEYS.R,
    Enter: GBA_KEYS.START, Space: GBA_KEYS.SELECT, Backspace: GBA_KEYS.SELECT, ShiftLeft: GBA_KEYS.SELECT, ShiftRight: GBA_KEYS.SELECT,
};
// The letters also by what the key is labelled, so the hint holds on AZERTY or QWERTZ keyboards
const LETTERS = { x: GBA_KEYS.A, z: GBA_KEYS.B, a: GBA_KEYS.L, s: GBA_KEYS.R };

// Standard gamepad layout, by position: right face button = A, bottom = B
const GAMEPAD_BUTTONS = [
    [1, GBA_KEYS.A], [0, GBA_KEYS.B], [4, GBA_KEYS.L], [5, GBA_KEYS.R], [9, GBA_KEYS.START], [8, GBA_KEYS.SELECT],
    [12, GBA_KEYS.UP], [13, GBA_KEYS.DOWN], [14, GBA_KEYS.LEFT], [15, GBA_KEYS.RIGHT],
];

export class EmulatorInput {
    // onGesture: a key or touch of the player's, which lets the page start its sound.
    constructor(emulator, isActive, touchArea, onGesture) {
        this.emulator = emulator;
        this.isActive = isActive;
        this.touchArea = touchArea;
        this.onGesture = onGesture;
        this.held = new Map();         // the GBA key each held keyboard key presses, by its code
        this.pointers = new Map();
        this.padTimer = null;
        this.onKeyDown = (event) => this.key(event, true);
        this.onKeyUp = (event) => this.key(event, false);
        this.onBlur = () => { this.held.clear(); this.emulator.setKeys('keyboard', 0); };
        this.onPad = () => this.watchGamepads();
        this.onPointer = (event) => this.pointer(event);
        this.onPointerEnd = (event) => this.pointerEnd(event);
        this.onContextMenu = (event) => event.preventDefault();
    }

    attach() {
        // Capture, so the page's own key handlers see the event as handled
        document.addEventListener('keydown', this.onKeyDown, true);
        document.addEventListener('keyup', this.onKeyUp, true);
        window.addEventListener('blur', this.onBlur);
        window.addEventListener('gamepadconnected', this.onPad);
        this.watchGamepads();
        if (this.touchArea) {
            this.touchArea.addEventListener('pointerdown', this.onPointer);
            this.touchArea.addEventListener('pointermove', this.onPointer);
            for (const type of ['pointerup', 'pointercancel', 'pointerleave']) this.touchArea.addEventListener(type, this.onPointerEnd);
            this.touchArea.addEventListener('contextmenu', this.onContextMenu);
        }
    }

    detach() {
        document.removeEventListener('keydown', this.onKeyDown, true);
        document.removeEventListener('keyup', this.onKeyUp, true);
        window.removeEventListener('blur', this.onBlur);
        window.removeEventListener('gamepadconnected', this.onPad);
        if (this.padTimer !== null) clearInterval(this.padTimer);
        this.padTimer = null;
        if (this.touchArea) {
            this.touchArea.removeEventListener('pointerdown', this.onPointer);
            this.touchArea.removeEventListener('pointermove', this.onPointer);
            for (const type of ['pointerup', 'pointercancel', 'pointerleave']) this.touchArea.removeEventListener(type, this.onPointerEnd);
            this.touchArea.removeEventListener('contextmenu', this.onContextMenu);
        }
        this.onBlur();
        this.pointers.clear();
        this.emulator.setKeys('touch', 0);
        this.emulator.setKeys('gamepad', 0);
    }

    typing() {
        const element = document.activeElement;
        return Boolean(element) && (['INPUT', 'TEXTAREA', 'SELECT'].includes(element.tagName) || element.isContentEditable);
    }

    key(event, down) {
        if (!down) {
            if (!this.held.delete(event.code)) return;
            event.preventDefault();
            this.applyKeyboard();
            return;
        }
        // A shortcut: the keys held with it may never see their keyup (Cmd on macOS)
        if (event.ctrlKey || event.metaKey || event.altKey) {
            if (this.held.size) this.onBlur();
            return;
        }
        const key = LETTERS[event.key.toLowerCase()] ?? KEYBOARD[event.code];
        if (key === undefined || !this.isActive() || this.typing()) return;
        event.preventDefault();
        this.held.set(event.code, key);
        this.onGesture();
        this.applyKeyboard();
    }

    applyKeyboard() {
        let mask = 0;
        for (const key of this.held.values()) mask |= key;
        this.emulator.setKeys('keyboard', mask);
    }

    watchGamepads() {
        if (this.padTimer !== null || typeof navigator.getGamepads !== 'function') return;
        this.padTimer = setInterval(() => this.pollGamepads(), 16);
    }

    pollGamepads() {
        let mask = 0;
        let any = false;
        for (const pad of navigator.getGamepads()) {
            if (!pad) continue;
            any = true;
            for (const [index, key] of GAMEPAD_BUTTONS) if (pad.buttons[index]?.pressed) mask |= key;
            const [x = 0, y = 0] = pad.axes;
            if (x < -0.5) mask |= GBA_KEYS.LEFT;
            if (x > 0.5) mask |= GBA_KEYS.RIGHT;
            if (y < -0.5) mask |= GBA_KEYS.UP;
            if (y > 0.5) mask |= GBA_KEYS.DOWN;
        }
        if (!any && this.padTimer !== null) {
            clearInterval(this.padTimer);
            this.padTimer = null;
        }
        this.emulator.setKeys('gamepad', this.isActive() ? mask : 0);
    }

    // Each finger presses whatever control is under it, so a thumb can slide across the D-pad
    pointer(event) {
        if (event.type === 'pointermove' && !this.pointers.has(event.pointerId)) return;
        event.preventDefault();
        if (event.type === 'pointerdown') {
            this.onGesture();
            try { this.touchArea.setPointerCapture(event.pointerId); } catch {}
        }
        const target = document.elementFromPoint(event.clientX, event.clientY)?.closest('[data-key]');
        const name = target && this.touchArea.contains(target) ? target.dataset.key : undefined;
        this.pointers.set(event.pointerId, name ? GBA_KEYS[name] : 0);
        this.applyTouch();
    }

    pointerEnd(event) {
        if (!this.pointers.has(event.pointerId)) return;
        this.pointers.delete(event.pointerId);
        this.applyTouch();
        // Lifting a finger, unlike touching down, lets iOS start sound
        if (event.type === 'pointerup') this.onGesture();
    }

    applyTouch() {
        let mask = 0;
        for (const key of this.pointers.values()) mask |= key;
        for (const element of this.touchArea.querySelectorAll('[data-key]')) {
            element.classList.toggle('pressed', (mask & GBA_KEYS[element.dataset.key]) !== 0);
        }
        this.emulator.setKeys('touch', mask);
    }
}
