// The emulator's wireless adapter as the page's Bridge sees a GB-Link in its wireless adapter
// mode: GB-Link frames both ways. The board's SetMode starts the adapter afresh, its data
// chunks go to the adapter, and the adapter's frames come back after every stretch the game
// runs.

import { GB_CHANNEL, GbFrameParser } from '../wire.js';

const SET_MODE = 0x00;
const MODE_WIRELESS_ADAPTER = 0x07;

export class EmulatedAdapter {
    constructor(emulator) {
        this.emulator = emulator;
        this.onBytes = null;       // (Uint8Array) GB-Link frames from the adapter
        this.parser = new GbFrameParser(512);
        emulator.afterRun = () => {
            const bytes = emulator.adapterTake();
            if (bytes) this.onBytes?.(bytes);
        };
    }

    // Whole GB-Link frames from the bridge firmware.
    writeStream(bytes) {
        for (const frame of this.parser.push(bytes)) {
            const { channel, payload } = frame;
            if (channel === GB_CHANNEL.COMMAND && payload[0] === SET_MODE && payload[1] === MODE_WIRELESS_ADAPTER) this.emulator.adapterEnterMode(payload[2] ?? 0);
            else if (channel === GB_CHANNEL.DATA) this.emulator.adapterFeed(payload);
        }
    }

    // The adapter stays with the game: it is the game's own, not the page's.
    async leaveMode() {}
}
