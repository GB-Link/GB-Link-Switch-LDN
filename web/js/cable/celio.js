// The Switch linked with another player over the Celio server: a Game Boy Advance on a
// GB-Link, an emulator, or another Switch through this page. The page stands in for a Celio
// client and its GB-Link adapter, and reports itself as one.
// The server seats the players in the order they press Start. Seat 0's adapter leaves its
// GBA to clock the link, the parent (player 0); the others clock theirs, children. So the
// page plays:
// - seat 0: the parent, as CableTranslator, with the Switch leading a group it joins;
// - seat 1: the child, as ReverseTranslator, leading a group the Switch joins.
// The page links two players; a session for more is left with a note.
//
// Celio protocol (Celio-Server 4_player session.ts, Celio-Client linkExchangeSession.ts and
// the GB-Link firmware's online mode): a session is made for a number of players, and every
// one of them hears partyReady once it is full. A client's link statuses go to the server
// only, in order and acknowledged, and it answers with device commands: the seat and player
// count (PlayerSettings), StartHandshake once every player's game is ready, and ConnectLink
// for the children once seat 0 has linked. The game's link moves an eight-word command per
// player each frame (a window); a client sends its own game's windows three to a packet of
// 51 bytes, each window as [seat, eight words little-endian], skipping packets with no
// command in them. Packets go as batches of { sequence, data }, numbered from 0 per client,
// each sent again until acknowledged with up to 64 in flight; the server puts them in order
// and relays them to every other player, numbered again for each, so they can arrive out of
// order and twice. Commands and session events carry a uuid and come again until
// acknowledged.

import { SocketIo } from './socketio.js';
import { CableTranslator, CMD_WORDS } from './translator.js';
import { ReverseTranslator } from './reverse.js';
import { RfuLeader } from './leader.js';
import { GB_CHANNEL, GbFrameParser } from '../wire.js';
import { FrameReader, RFU, clientFrame, command, toGbFrames } from '../trade/adapter.js';

// The server with the 2-4 player Celio protocol.
export const CELIO_SERVER = 'wss://celio-server.up.railway.app';

const STATUS = { AWAIT_PLAYER_SETTINGS: 0xff02, HANDSHAKE_RECEIVED: 0xff03, LINK_CONNECTED: 0xff05, LINK_RECONNECTING: 0xff06, LINK_CLOSED: 0xff07 };
const COMMAND = { PLAYER_SETTINGS: 0x10, START_HANDSHAKE: 0x12, CONNECT_LINK: 0x13 };
const PLAYERS = 2;

// One window per GBA frame, as a parent GBA clocks the link.
export const WINDOW_MS = 1000 / 59.7275;
const WINDOWS_PER_PACKET = 3;
const BLOCK_BYTES = 1 + CMD_WORDS * 2;
const PACKET_BYTES = WINDOWS_PER_PACKET * BLOCK_BYTES;
const AHEAD = WINDOWS_PER_PACKET;   // windows produced ahead of the clock
const REOPEN_MS = 400;              // the adapter's pause before a new section
const FRAME_MS = 1000 / 59.7275;
const DATA_IN_FLIGHT = 64;
// An adapter shows a GBA these statuses as the end of a section when they lead a command.
const CLOSING_STATUSES = new Set([0xff02, 0xff06, 0xff07]);

const uuid = () => crypto.randomUUID();
const isCloseKey = (words) => words[0] === 0xcafe && (words[1] & 0xff) === 0x17;

// The cable side over Celio. onRole(seat) is called once the server has seated the page,
// and returns the game side: { gameCommand, nextCommand, reset, ready }. onRefused(reason)
// tells of a session the page cannot play: 'players' (with the count) or 'old-server'.
export class CelioLink {
    constructor(socket, { onRole, onRefused = () => {}, log = () => {} }) {
        this.socket = socket;
        this.onRole = onRole;
        this.onRefused = onRefused;
        this.game = null;
        this.log = log;
        this.running = false;
        this.seat = null;             // null until the server has seated the page
        this.partner = false;
        this.connected = false;       // a section is up
        this.waiting = false;         // told the server our GBA is ready, awaiting the others
        this.handshake = false;       // a child: the handshake started, awaiting seat 0's link
        this.closed = false;          // the link ended for good (EXIT_ROOM)
        this.readyAfter = 0;
        this.sequence = 0;
        this.expected = 0;
        this.buffered = new Map();
        this.windows = [];            // this section's windows not yet sent
        this.cafeStreak = 0;
        this.since = 0;
        this.made = 0;                // windows made this section
        this.sentClose = false;
        this.gotClose = false;
        this.exitSeen = false;
        this.commands = new Set();
        this.outgoing = [];
        this.inFlight = 0;
        this.statuses = Promise.resolve();
        this.timer = null;
        this.onState = null;
        socket.on('deviceCommand', (packet) => this.onCommand(packet));
        socket.on('deviceData', (packets) => { if (Array.isArray(packets)) for (const packet of packets) this.onData(packet); });
    }

    status(value) {
        const packet = { uuid: uuid(), linkStatus: value };
        this.statuses = this.statuses.then(() => this.socket.requestWithRetry('deviceStatus', packet))
            .then(([accepted] = []) => { if (accepted === false) this.log(`the server refused link status ${value.toString(16)}`); })
            .catch((error) => this.log(`the server did not take a link status: ${error.message}`));
    }

    sendData(packet) {
        this.outgoing.push(packet);
        this.fillData();
    }

    fillData() {
        while (this.running && this.inFlight < DATA_IN_FLIGHT && this.outgoing.length) {
            const next = this.outgoing.shift();
            this.inFlight++;
            // The other side plays sequences in order and cannot skip one: no giving up while
            // the socket is up.
            this.socket.requestWithRetry('deviceData', [next], { retries: Infinity, stop: () => !this.running })
                .then(([accepted] = []) => { if (accepted === false) this.log('the server refused a data packet'); })
                .catch((error) => this.log(`the server did not take data: ${error.message}`))
                .finally(() => { this.inFlight--; this.fillData(); });
        }
    }

    setConnected(value) {
        if (this.connected === value) return;
        this.connected = value;
        this.onState?.(value);
    }

    start() {
        if (this.running) return;
        this.running = true;
        this.timer = setInterval(() => this.pump(), 8);
        this.status(STATUS.AWAIT_PLAYER_SETTINGS);
    }

    stop() {
        this.running = false;
        this.outgoing = [];
        clearInterval(this.timer);
        this.setConnected(false);
    }

    // The server starts the handshake once every player in the session is ready, however
    // few they are, so our GBA is reported ready only while the session is full; as the
    // child, only once it can answer the parent (the Switch's player is known).
    setPartner(present) {
        this.partner = present;
        this.maybeReady();
    }

    maybeReady() {
        if (!this.running || !this.game || !this.partner || this.connected || this.waiting || this.handshake || this.closed) return;
        if (performance.now() < this.readyAfter || !this.game.ready()) return;
        this.waiting = true;
        this.status(STATUS.HANDSHAKE_RECEIVED);
        this.log('Ready for the other Game Boy Advance');
    }

    onCommand(packet) {
        if (!packet || this.commands.has(packet.uuid)) return;
        this.commands.add(packet.uuid);
        switch (packet.command) {
            case COMMAND.PLAYER_SETTINGS: {
                if (this.seat !== null) break;
                const count = packet.args?.playerCount, seat = packet.args?.playerIndex;
                // The two-player server's SetModeMaster has the same number and no seat.
                if (!Number.isInteger(count) || !Number.isInteger(seat)) { this.onRefused('old-server'); break; }
                if (count !== PLAYERS || seat < 0 || seat >= count) { this.onRefused('players', count); break; }
                this.seat = seat;
                this.game = this.onRole(seat);
                this.maybeReady();
                break;
            }
            case COMMAND.START_HANDSHAKE:
                if (!this.waiting) break;
                this.waiting = false;
                this.game.reset();
                this.sentClose = this.gotClose = this.exitSeen = false;
                this.windows = [];
                this.cafeStreak = 0;
                // Seat 0's game links at once; a child's once seat 0 has (ConnectLink).
                if (this.seat === 0) this.up();
                else this.handshake = true;
                break;
            case COMMAND.CONNECT_LINK:
                if (this.seat === 0 || !this.handshake) break;
                this.handshake = false;
                this.up();
                break;
        }
    }

    up() {
        this.since = performance.now();
        this.made = 0;
        this.setConnected(true);
        this.status(STATUS.LINK_CONNECTED);
        this.log('the other Game Boy Advance\'s link is open');
    }

    // Packets may arrive out of order and twice: played in sequence order, once.
    onData(packet) {
        if (!packet || !Number.isInteger(packet.sequence) || packet.sequence < this.expected) return;
        this.buffered.set(packet.sequence, packet.data);
        while (this.buffered.has(this.expected)) {
            const data = this.buffered.get(this.expected);
            this.buffered.delete(this.expected);
            this.expected++;
            this.deliver(data);
        }
    }

    // The other player's windows, each as its command for our game. An adapter drops what
    // comes between sections; a child's keeps what its parent sends during the handshake.
    deliver(data) {
        if ((!this.connected && !this.handshake) || !Array.isArray(data) || data.length !== PACKET_BYTES) return;
        for (let o = 0; o < PACKET_BYTES; o += BLOCK_BYTES) {
            if (data[o] === this.seat || data[o] >= PLAYERS) continue;
            const words = [];
            for (let i = 0; i < CMD_WORDS; i++) words.push((data[o + 1 + i * 2] & 0xff) | (data[o + 2 + i * 2] & 0xff) << 8);
            if (CLOSING_STATUSES.has(words[0])) words[0] = 0x5fff;
            if (!words.some((w) => w)) continue;
            if (words[0] === 0x5fff) this.gotClose = true;
            if (isCloseKey(words)) this.exitSeen = true;
            this.game.gameCommand(words);
        }
        this.checkClose();
    }

    pump() {
        if (!this.connected) { this.maybeReady(); return; }
        const due = Math.floor((performance.now() - this.since) / WINDOW_MS) + AHEAD;
        while (this.made < due) {
            let words = (this.game.nextCommand() ?? new Array(CMD_WORDS).fill(0)).map((w) => w & 0xffff);
            if (words[0] === 0x5fff) this.sentClose = true;
            if (isCloseKey(words)) this.exitSeen = true;
            // Like the adapter: from the second "no key" report in a row, the window goes empty.
            if (words[0] === 0xcafe && words[1] === 0x11) {
                if (this.cafeStreak++ > 0) words = new Array(CMD_WORDS).fill(0);
            } else this.cafeStreak = 0;
            this.windows.push(words);
            this.made++;
            if (this.windows.length === WINDOWS_PER_PACKET) this.flush();
            if (this.checkClose()) return;
        }
    }

    // A packet of our windows, those not filled left at zero (seat 0, no command), as the
    // adapter leaves them; one with no command in it is not sent.
    flush() {
        const windows = this.windows;
        this.windows = [];
        if (!windows.some((words) => words.some((w) => w))) return;
        const data = new Array(PACKET_BYTES).fill(0);
        windows.forEach((words, b) => {
            const o = b * BLOCK_BYTES;
            data[o] = this.seat;
            words.forEach((w, i) => { data[o + 1 + i * 2] = w & 0xff; data[o + 2 + i * 2] = w >> 8; });
        });
        this.sendData({ sequence: this.sequence++, data });
    }

    // A command whose first word is 0x5FFF, sent and received, ends the section, as the
    // adapter does; EXIT_ROOM before it ends the link.
    checkClose() {
        if (!this.connected || !this.sentClose || !this.gotClose) return false;
        this.flush();
        this.setConnected(false);
        if (this.exitSeen) {
            this.closed = true;
            this.status(STATUS.LINK_CLOSED);
            this.log('the other Game Boy Advance left the link');
        } else {
            this.status(STATUS.LINK_RECONNECTING);
            this.readyAfter = performance.now() + REOPEN_MS;
        }
        return true;
    }
}

// A Celio session: the server connection, the session id, and once started, the board's
// room and the translator for the role the server chose. As the child the page leads a group
// for the room the Switch goes to, chosen once that role is known (chooseRoom).
export class CelioSession extends EventTarget {
    constructor(esp, { server = CELIO_SERVER, bypassNationally = false } = {}) {
        super();
        this.esp = esp;
        this.server = server;
        this.bypassNationally = bypassNationally;
        this.activity = null;             // the room of the page's group: 4 trade, 1 single, 2 double
        this.socket = null;
        this.sessionId = null;
        this.partner = false;
        this.running = false;
        this.role = null;             // 'parent' (the Switch leads) | 'child' (the page leads)
        this.translator = null;
        this.leader = null;
        this.board = null;            // where the board's RFU frames go
        this.link = null;
        this.timers = [];
        this.onReattached = () => this.claim().catch((error) => this.fail(error));
    }

    get leading() { return this.role === 'child'; }
    get needsRoom() { return this.leading && this.activity === null; }
    get linked() { return this.leading ? Boolean(this.leader?.linked && this.translator?.parentLP) : Boolean(this.translator?.linked); }
    get cableOpen() { return Boolean(this.link?.connected); }
    get switchNotReady() { return !this.leading && Boolean(this.translator?.hostNotReady); }
    get switchGroupOpen() { return !this.leading && Boolean(this.translator?.groupSeen); }
    get switchJoining() { return !this.leading && Boolean(this.translator?.joined); }
    get switchJoined() { return Boolean(this.leader?.joined); }
    get switchKnown() { return this.leading && Boolean(this.translator?.ready); }
    get switchLeft() { return this.leader?.state === 'closed'; }
    get otherChoice() { return this.leading ? this.translator?.otherChoice ?? null : null; }
    get refused() { return this.leading ? this.translator?.refused ?? null : null; }

    log(message) { this.dispatchEvent(new CustomEvent('log', { detail: message })); }
    changed() { this.dispatchEvent(new Event('change')); }
    fail(error) { this.dispatchEvent(new CustomEvent('failed', { detail: error })); }
    notice(text, tone = 'warn') { this.dispatchEvent(new CustomEvent('notice', { detail: { text, tone } })); }

    async open() {
        const socket = this.socket = new SocketIo(this.server, { clientId: uuid() });
        // Session events come again until acknowledged: each acted on once, by uuid.
        const seen = new Set();
        const once = (handler) => (event) => {
            if (event?.uuid) {
                if (seen.has(event.uuid)) return;
                seen.add(event.uuid);
            }
            handler();
        };
        // partyReady can come before the answer to the join that filled the session.
        socket.on('partyReady', once(() => { this.partner = true; this.link?.setPartner(true); this.changed(); }));
        socket.on('partyNotReady', once(() => { this.partner = false; this.link?.setPartner(false); this.notice('The other player left the session.'); this.changed(); }));
        socket.on('sessionClose', once(() => { this.notice('The session has ended.', ''); this.end(); }));
        // Only the two-player server sends this.
        socket.on('partnerJoined', once(() => this.refuse('old-server')));
        socket.addEventListener('disconnect', () => { this.notice('Lost the connection to the Celio server.', 'bad'); this.end(); });
        await socket.connect();
    }

    async create() { return this.enter('sessionCreate', PLAYERS); }

    async join(id) {
        if (!/^\d{4}$/.test(id)) throw new Error('A Session Id is four digits.');
        return this.enter('sessionJoin', id);
    }

    async enter(event, arg) {
        if (!this.socket?.connected) await this.open();
        const [result] = await this.socket.request(event, arg);
        if (result?.variant !== 'Ok') throw new Error(result?.error ?? 'The Server refused');
        // A two-player session is full once the second player is in.
        if (event === 'sessionJoin' && !result.value.full) {
            try { this.socket.emit('sessionLeft'); } catch {}
            throw new Error('This session is for more than two players. The Switch plays in two-player sessions.');
        }
        this.sessionId = result.value.id;
        this.partner = this.partner || Boolean(result.value.full);
        this.changed();
        return this.sessionId;
    }

    // A session this page cannot play: said, and left.
    refuse(reason, count) {
        const text = reason === 'players'
            ? `This session is for ${count} players. The Switch plays in two-player sessions.`
            : 'This Celio server speaks the earlier two-player protocol. Choose one with the 2–4 player protocol under Options.';
        this.notice(text, 'bad');
        this.leave();
    }

    // Takes the board's adapter port and starts linking.
    async start() {
        if (this.running) return;
        const esp = this.esp;
        const gb = new GbFrameParser(512);
        const reader = new FrameReader();
        esp.onAdapterFrame = (bytes) => {
            try {
                for (const frame of gb.push(bytes)) {
                    if (frame.channel !== GB_CHANNEL.DATA) continue;
                    for (const rfu of reader.push(frame.payload)) this.board?.(rfu);
                }
            } catch (error) { this.fail(error); }
        };
        this.link = new CelioLink(this.socket, {
            onRole: (seat) => this.setRole(seat !== 0),
            onRefused: (reason, count) => this.refuse(reason, count),
            log: (message) => this.log(message),
        });
        this.link.onState = () => this.changed();
        this.running = true;
        esp.addEventListener('reattached', this.onReattached);
        try {
            await this.claim();
        } catch (error) {
            await this.stop();
            throw error;
        }
        this.link.setPartner(this.partner);
        this.link.start();
        this.changed();
    }

    // The page's seat: in seat 0 it is the parent and joins the Switch's group; in seat 1 it
    // is the child and leads a group the Switch joins.
    setRole(child) {
        this.role = child ? 'child' : 'parent';
        this.log(child ? 'the other Game Boy Advance is the parent: the Switch joins this page\'s group' : 'the other Game Boy Advance is the child: this page joins the Switch\'s group');
        if (!child) {
            const toBoard = (frame) => this.esp.sendAdapter(toGbFrames(frame));
            const translator = new CableTranslator({
                send: (payload) => toBoard(clientFrame(payload)),
                connect: (devid) => toBoard(command(RFU.CONNECT_REQ, devid)),
                disconnect: () => toBoard(command(RFU.DISCONNECT, 0)),
                log: (message) => this.log(message),
                holdJoin: true,
                packetMs: WINDOW_MS,
            });
            translator.onLinked = () => this.changed();
            translator.bypassNationally = this.bypassNationally;
            this.translator = translator;
            this.board = (rfu) => translator.boardFrame(rfu);
            this.timers.push(setInterval(() => translator.frame(), FRAME_MS));
        }
        this.changed();
        // As the child nothing links until the room is chosen and the Switch has joined.
        return {
            gameCommand: (words) => this.translator?.gameCommand(words),
            nextCommand: () => this.translator?.nextCommand() ?? null,
            reset: () => this.translator?.cableReset(),
            // Each side is ready once its Switch is: leading a group the page has seen, or
            // waiting in the page's group. Joining, and the player exchange, come after.
            ready: () => Boolean(child ? this.translator?.ready : this.translator?.readyToLink),
        };
    }

    // The room of the group the page leads as the child: the Switch joins it there.
    chooseRoom(activity) {
        if (!this.running || !this.needsRoom) return;
        this.activity = activity;
        const toBoard = (frame) => this.esp.sendAdapter(toGbFrames(frame));
        const log = (message) => this.log(message);
        const leader = this.leader = new RfuLeader({ send: toBoard, log });
        const translator = new ReverseTranslator({ leader, activity, log, packetMs: WINDOW_MS });
        translator.onReady = () => this.changed();
        translator.onOtherChoice = () => this.changed();
        translator.onRefused = (reason) => {
            this.changed();
            if (reason) this.dispatchEvent(new CustomEvent('refused', { detail: reason }));
        };
        const joined = leader.onJoined;
        leader.onJoined = () => { joined?.(); this.changed(); };
        leader.onClosed = () => { log('the Switch left the group'); this.changed(); };
        translator.bypassNationally = this.bypassNationally;
        this.translator = translator;
        this.board = (rfu) => leader.boardFrame(rfu);
        translator.openGroup();
        this.timers.push(setInterval(() => leader.tick(), FRAME_MS));
        this.changed();
    }

    async claim() {
        if (!this.running) return;
        if (!(await this.esp.setAdapterPort('host'))) throw new Error('The ESP32 board did not hand over its adapter port.');
    }

    setBypass(value) {
        this.bypassNationally = value;
        if (this.translator) this.translator.bypassNationally = value;
    }

    async stop() {
        if (!this.running) return;
        this.running = false;
        for (const t of this.timers) clearInterval(t);
        this.timers = [];
        this.link?.stop();
        this.esp.removeEventListener('reattached', this.onReattached);
        try {
            if (this.leader) this.leader.close();
            else if (this.translator?.joined) this.esp.sendAdapter(toGbFrames(command(RFU.DISCONNECT, 0)));
        } catch {}
        this.board = null;
        this.esp.onAdapterFrame = null;
        await new Promise((resolve) => setTimeout(resolve, 150));
        try { if (this.esp.attached) await this.esp.setAdapterPort('uart'); } catch {}
    }

    // Leaves the session; after the link has started this ends it for both.
    async leave() {
        try { this.socket?.emit('sessionLeft'); } catch {}
        await this.end();
    }

    async end() {
        if (this.ended) return;
        this.ended = true;
        await this.stop();
        this.socket?.close();
        this.sessionId = null;
        this.partner = false;
        this.dispatchEvent(new Event('ended'));
    }
}
