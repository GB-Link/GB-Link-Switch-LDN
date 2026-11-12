// The page as the parent (player 0) of the emulated game on its link cable, the way a GB-Link
// in its cable mode is the parent of a real Ruby or Sapphire: it clocks the game's transfers
// in the game's own time, runs the link-up handshake, then trades one eight-word command each
// way every frame. The commands come from and go to the translator, as with CableLink.
//
// Before the link is up, one handshake transfer a frame. After it, each frame a checksum
// transfer and eight command transfers, 197 x 64 cycles apart (Timer 3, as the games set it).

import { CMD_WORDS } from '../cable/translator.js';
import { FRAME_CYCLES, GBA_CYCLES_PER_SECOND } from './emulator.js';

const HANDSHAKE = { MASTER: 0x8fff, SLAVE: 0xb9a0, NOT_READY: 0xd15e };
const READY_CLOSE_LINK = 0x5fff;
const SEND_HELD_KEYS = 0xcafe;
const KEY_EXIT_ROOM = 0x17;

const ROLE_NONE = 0;
const ROLE_CHILD = 2;
const PLAYERS = 2;
const GAME_SEAT = 1;
// A transfer between two players at 115200 bps, as the emulator times it
const TRANSFER_CYCLES = 6075;
const COMMAND_GAP_CYCLES = 197 * 64;
const WINDOW_REST_CYCLES = FRAME_CYCLES - (CMD_WORDS + 1) * TRANSFER_CYCLES - CMD_WORDS * COMMAND_GAP_CYCLES;
// Slave handshakes before the master's once the game has sent its own: 150 ms, as CableLink
// gives a GB-Link
const SLAVE_HANDSHAKES = 9;
// A GB-Link's pause between one link and the next
const REOPEN_CYCLES = Math.round(0.4 * GBA_CYCLES_PER_SECOND);

// The cable's pace for the translator: one command each way per frame.
export const PACKET_MS = (FRAME_CYCLES / GBA_CYCLES_PER_SECOND) * 1000;

export class EmulatedCable {
    // gameCommand(words): a command from the game. nextCommand(): player 0's next command, or
    // null. reset(): the game opened a fresh link.
    constructor(emulator, { gameCommand, nextCommand, reset, log = () => {} }) {
        this.emulator = emulator;
        this.port = emulator.cablePort();
        this.gameCommand = gameCommand;
        this.nextCommand = nextCommand;
        this.reset = reset;
        this.log = log;
        this.running = false;
        this.connected = false;
        this.onState = null;       // (connected)
    }

    start() {
        if (this.running) return;
        this.running = true;
        this.port.setTransferHandler((rx) => this.transferDone(rx));
        // A power-cycled game has a cable with nobody on it until the next link opens.
        this.emulator.onReset = () => this.open(0);
        this.open(0);
    }

    async stop() {
        if (!this.running) return;
        this.running = false;
        this.port.setTransferHandler(null);
        this.emulator.onReset = null;
        this.port.configure(ROLE_NONE, 0, PLAYERS);
        this.setConnected(false);
    }

    setConnected(value) {
        if (this.connected === value) return;
        this.connected = value;
        this.onState?.(value);
    }

    // A new link: not-ready handshakes until the game sends its own, then the link-up.
    open(delayCycles) {
        this.setConnected(false);
        this.phase = 'waiting';     // waiting | enabled | connecting | linked
        this.state = 'handshake';   // handshake | checksum | command
        // The first checksum after the handshake is the handshake word.
        this.crc = HANDSHAKE.SLAVE;
        this.index = 0;
        this.received = new Array(CMD_WORDS).fill(0);
        this.outgoing = null;
        this.gameClosing = false;
        this.pageClosing = false;
        this.exiting = false;
        this.port.configure(ROLE_CHILD, GAME_SEAT, PLAYERS);
        this.stage();
        this.port.clock(delayCycles);
    }

    // The parent's word for the next transfer.
    stage() {
        if (this.state === 'handshake') {
            this.sent = this.phase === 'waiting' ? HANDSHAKE.NOT_READY : this.phase === 'enabled' ? HANDSHAKE.SLAVE : HANDSHAKE.MASTER;
        } else if (this.state === 'checksum') {
            this.sent = this.crc;
        } else {
            this.sent = this.outgoing ? this.outgoing[this.index] & 0xffff : 0;
        }
        this.port.stage([this.sent, 0xffff, 0xffff, 0xffff]);
    }

    transferDone(rx) {
        if (!this.running) return;
        let gap = COMMAND_GAP_CYCLES;
        if (this.state === 'handshake') {
            gap = FRAME_CYCLES - TRANSFER_CYCLES;
            this.handshake(rx);
        } else if (this.state === 'checksum') {
            // The game's checksum is not checked here: the parent's is the one that counts.
            this.state = 'command';
            this.crc = 0;
        } else {
            this.received[this.index++] = rx;
            this.crc = (this.crc + rx + this.sent) & 0xffff;
            if (this.index === CMD_WORDS) {
                gap = WINDOW_REST_CYCLES;
                this.index = 0;
                this.state = 'checksum';
                if (!this.window()) return;
            }
        }
        this.stage();
        this.port.clock(gap);
    }

    handshake(rx) {
        if (this.phase === 'waiting' && rx === HANDSHAKE.SLAVE) {
            this.reset();
            this.phase = 'enabled';
            this.slaveHandshakes = 0;
        } else if (this.phase === 'enabled' && this.sent === HANDSHAKE.SLAVE) {
            if (++this.slaveHandshakes >= SLAVE_HANDSHAKES) this.phase = 'connecting';
        } else if (this.phase === 'connecting' && this.sent === HANDSHAKE.MASTER) {
            this.phase = 'linked';
            this.state = 'checksum';
            this.outgoing = this.nextCommand() ?? null;
            this.setConnected(true);
            this.log('the game\'s cable link is open');
        }
    }

    // A finished window: the game's command goes to the translator, and the link closes once
    // both sides are ready to. Returns false when it closed.
    window() {
        const received = this.received.slice();
        const sent = this.outgoing;
        if (received.some((word) => word)) this.gameCommand(received);
        if (received[0] === SEND_HELD_KEYS && (received[1] & 0xff) === KEY_EXIT_ROOM) this.exiting = true;
        if (sent && sent[0] === SEND_HELD_KEYS && (sent[1] & 0xff) === KEY_EXIT_ROOM) this.exiting = true;
        if (received[0] === READY_CLOSE_LINK) this.gameClosing = true;
        if (sent && sent[0] === READY_CLOSE_LINK) this.pageClosing = true;
        if (this.gameClosing && this.pageClosing) {
            this.log(this.exiting ? 'the cable link ended' : 'the game closed its cable link');
            this.open(REOPEN_CYCLES);
            return false;
        }
        this.outgoing = this.nextCommand() ?? null;
        return true;
    }
}
