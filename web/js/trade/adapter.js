// The wireless adapter's own frames, as the board's bridge speaks them to whatever is
// playing the adapter. The board runs the room, decrypts everything and hands over
// these plain frames, so nothing here needs a key.
//
// A frame is "RFU1", a 4-byte type and a 4-byte header, both big-endian, then its
// payload. Sizes are fixed per type. They travel as GB-Link data frames in 64-byte
// pieces, and a receiver resynchronises on the magic.

import { b32, join, wb32 } from './bytes.js';
import { GB_CHANNEL, buildGbFrame } from '../wire.js';

const MAGIC = Uint8Array.of(0x52, 0x46, 0x55, 0x31);
const CHUNK = 64;

export const RFU = {
    BROADCAST: 0,     // the board offers a room
    CONNECT_REQ: 1,   // this side asks to join one
    CONNECT_ACK: 2,   // the board takes it in
    DISCONNECT: 4,
    HOST_SEND: 5,     // a frame from the game on the Switch
    CLIENT_SEND: 6,   // a frame from the game this page plays
};

export function frameSize(type) {
    if (type === RFU.BROADCAST) return 36;
    if (type === RFU.HOST_SEND || type === RFU.CLIENT_SEND) return 104;
    return 16;
}

export function command(type, header) {
    const frame = new Uint8Array(16);
    frame.set(MAGIC);
    wb32(frame, 4, type);
    wb32(frame, 8, header);
    return frame;
}

// The board reads this side's length from the top byte of the header.
export function clientFrame(payload) {
    const frame = new Uint8Array(104);
    frame.set(MAGIC);
    wb32(frame, 4, RFU.CLIENT_SEND);
    wb32(frame, 8, Math.min(payload.length, 92) << 24);
    frame.set(payload.subarray(0, 92), 12);
    return frame;
}

// The board's own length sits in the low bits of the header.
export function hostPayload(frame) {
    const length = Math.min(b32(frame, 8) & 0x7f, 92);
    return frame.subarray(12, 12 + length);
}

// Whole frames out of a stream that arrives in pieces and may be padded.
export class FrameReader {
    constructor() {
        this.buffer = new Uint8Array(0);
    }

    push(bytes) {
        this.buffer = join(this.buffer, bytes);
        const frames = [];
        let at = 0;
        while (this.buffer.length - at >= 12) {
            if (!startsWithMagic(this.buffer, at)) { at++; continue; }
            const size = frameSize(b32(this.buffer, at + 4));
            if (this.buffer.length - at < size) break;
            frames.push({ type: b32(this.buffer, at + 4), header: b32(this.buffer, at + 8), frame: this.buffer.slice(at, at + size) });
            at += size;
        }
        this.buffer = this.buffer.slice(at);
        if (this.buffer.length > 4096) this.buffer = this.buffer.slice(-256);
        return frames;
    }
}

function startsWithMagic(bytes, at) {
    for (let i = 0; i < 4; i++) if (bytes[at + i] !== MAGIC[i]) return false;
    return true;
}

// One frame as the GB-Link data frames the board expects: 64 bytes at a time, the last
// piece padded, because a shorter piece is read as the adapter's own telemetry.
export function toGbFrames(frame) {
    const pieces = [];
    for (let at = 0; at < frame.length; at += CHUNK) {
        const piece = new Uint8Array(CHUNK);
        piece.set(frame.subarray(at, at + CHUNK));
        pieces.push(buildGbFrame(GB_CHANNEL.DATA, piece));
    }
    return join(...pieces);
}
