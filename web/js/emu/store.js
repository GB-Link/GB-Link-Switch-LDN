// The emulator's games kept in this browser's IndexedDB: each ROM with its battery save, so
// they can be played again without choosing the files. Nothing leaves the device. Every call
// fails soft: private windows and blocked storage just keep nothing.

const DB_NAME = 'gblink-switch-emulator';
const DB_VERSION = 1;
const GAMES = 'games';
// ROM bytes are kept apart so listing the games doesn't read them
const ROMS = 'roms';

// A kept game: { id (the ROM's CRC-32), title, name, save, unsaved (saved since its save was
// last downloaded), addedAt, playedAt }.
export class GameStore {
    constructor() {
        this.queue = Promise.resolve();
    }

    open() {
        return new Promise((resolve, reject) => {
            const request = indexedDB.open(DB_NAME, DB_VERSION);
            request.onupgradeneeded = () => {
                request.result.createObjectStore(GAMES, { keyPath: 'id' });
                request.result.createObjectStore(ROMS);
            };
            request.onsuccess = () => resolve(request.result);
            request.onerror = () => reject(request.error);
        });
    }

    async run(stores, mode, work) {
        const db = await this.open();
        try {
            return await new Promise((resolve, reject) => {
                const tx = db.transaction(stores, mode);
                let result;
                work(tx, (value) => { result = value; });
                tx.oncomplete = () => resolve(result);
                tx.onerror = () => reject(tx.error);
                tx.onabort = () => reject(tx.error);
            });
        } finally {
            db.close();
        }
    }

    // Writes run in order, so an older save can't overwrite a newer one.
    write(job) {
        const next = this.queue.then(job, job);
        this.queue = next.catch(() => undefined);
        return next;
    }

    // The kept games without their ROMs, last played first.
    async list() {
        try {
            const games = await this.run([GAMES], 'readonly', (tx, done) => {
                tx.objectStore(GAMES).getAll().onsuccess = (event) => done(event.target.result);
            });
            return (games ?? []).sort((a, b) => b.playedAt - a.playedAt);
        } catch {
            return [];
        }
    }

    // One kept game with its ROM, or null.
    async get(id) {
        try {
            const game = await this.run([GAMES, ROMS], 'readonly', (tx, done) => {
                tx.objectStore(GAMES).get(id).onsuccess = (event) => {
                    const stored = event.target.result;
                    if (!stored) return;
                    tx.objectStore(ROMS).get(id).onsuccess = (r) => {
                        const rom = r.target.result;
                        if (rom) done({ ...stored, rom: new Uint8Array(rom) });
                    };
                };
            });
            return game ?? null;
        } catch {
            return null;
        }
    }

    // Keeps a game that was just started (its ROM is stored once).
    put({ id, title, name, rom, save, unsaved }) {
        return this.write(async () => {
            try {
                const now = Date.now();
                await this.run([GAMES, ROMS], 'readwrite', (tx) => {
                    const games = tx.objectStore(GAMES);
                    games.get(id).onsuccess = (event) => {
                        const old = event.target.result;
                        if (!old) tx.objectStore(ROMS).put(rom.slice().buffer, id);
                        games.put({ id, title, name, save, unsaved, addedAt: old?.addedAt ?? now, playedAt: now });
                    };
                });
                return true;
            } catch {
                return false;
            }
        });
    }

    // The game saved: keeps the new save, marked as not downloaded yet.
    updateSave(id, save) {
        return this.update(id, (game) => { game.save = save; game.unsaved = true; });
    }

    markDownloaded(id) {
        return this.update(id, (game) => { game.unsaved = false; });
    }

    update(id, change) {
        return this.write(async () => {
            try {
                await this.run([GAMES], 'readwrite', (tx) => {
                    const games = tx.objectStore(GAMES);
                    games.get(id).onsuccess = (event) => {
                        const game = event.target.result;
                        if (!game) return;
                        change(game);
                        games.put(game);
                    };
                });
                return true;
            } catch {
                return false;
            }
        });
    }

    remove(id) {
        return this.write(async () => {
            try {
                await this.run([GAMES, ROMS], 'readwrite', (tx) => {
                    tx.objectStore(GAMES).delete(id);
                    tx.objectStore(ROMS).delete(id);
                });
                return true;
            } catch {
                return false;
            }
        });
    }
}
