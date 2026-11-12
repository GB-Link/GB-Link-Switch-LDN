/* Game Boy Advance emulator for the page: mGBA's GBA core, run by cycle count from
 * JavaScript so it keeps pace with real time. Its link port has a GBA Wireless Adapter
 * (rfu_emu.cpp) whose network side the page carries to the ESP32 board, or a link cable to
 * a player the page plays, for the games without wireless.
 *
 * The game talks to the adapter in normal 32-bit mode. As the clock master it sends a word
 * and gets back the one the adapter had ready; between words the two run librfu's
 * handshake: the adapter drives SI, the game SO. After a WAIT-class command the game turns
 * slave and the adapter clocks the event's words into it. The game resets the adapter by
 * taking the port into general-purpose mode.
 *
 * On the cable the game is in a multiplayer (MULTI) link, either the parent (seat 0, it
 * clocks every transfer and the link fills in the other players' words) or a child (the link
 * clocks the transfers as the parent and the game sends in its own slot). Every finished
 * transfer is handed to JavaScript (Module.onCableTransfer), which stages the words of the
 * next one and, for a child, when it starts. */
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/timing.h>
#include <mgba/gba/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/io.h>
#include <mgba/internal/gba/sio.h>
#include <mgba-util/vfs.h>

#include "rfu_emu.h"

#define VIDEO_WIDTH 240
#define VIDEO_HEIGHT 160
#define AUDIO_BUFFER_SAMPLES 4096
#define CYCLES_PER_SECOND 16777216

/* 32 bits at 2 MHz and at 256 kHz */
#define WORD_CYCLES_FAST 268
#define WORD_CYCLES_SLOW 2147
/* The adapter's upkeep runs about once a millisecond */
#define TICK_CYCLES 16777
/* A slave game that has not armed for the adapter's next word by then is not taking it */
#define ARM_TIMEOUT_MS 60
#define MAX_EVENT_WORDS 24

#define SIOCNT_INTERNAL_CLOCK 0x0001
#define SIOCNT_FAST 0x0002
#define SIOCNT_SI 0x0004
#define SIOCNT_SO 0x0008
#define SIOCNT_START 0x0080
#define SIOCNT_IRQ 0x4000

#define MAX_PLAYERS 4
#define BAUD_115200 3
/* The link waits 1 ms for a child that does not answer, then reads the idle line */
#define NO_REPLY_CYCLES 16777

/* What is plugged into the link port. */
enum LinkKind {
	LINK_NOTHING = 0,
	LINK_WIRELESS = 1,
	LINK_CABLE = 2,
};

enum CableRole {
	CABLE_NONE = 0,   /* no link: the game sees SD low (not every player is ready) and nobody clocks */
	CABLE_PARENT = 1, /* the game is the parent and clocks; the link answers in the other slots */
	CABLE_CHILD = 2,  /* the game is a child; the link clocks as the parent */
};

enum CablePhase {
	PHASE_IDLE = 0,
	PHASE_START,  /* CABLE_CHILD: a transfer starts when the event fires */
	PHASE_FINISH, /* a transfer finishes when the event fires */
};

static struct mCore* core = NULL;
static void* romData = NULL;
/* What the core draws into, and the last finished frame as shown */
static color_t videoBuffer[VIDEO_WIDTH * VIDEO_HEIGHT];
static color_t displayBuffer[VIDEO_WIDTH * VIDEO_HEIGHT];
static int framesDone = 0;
static int16_t audioOut[AUDIO_BUFFER_SAMPLES * 2];
static int audioRate = 48000;
static void* saveCopy = NULL;
static size_t saveCopySize = 0;

static struct {
	struct GBASIODriver d;
	struct mTimingEvent word; /* a word on the wire ends */
	struct mTimingEvent tick;
	bool plugged;
	bool si;           /* the adapter's line to the game */
	bool gameWord;     /* a word the game clocks is on the wire */
	bool adapterWord;  /* a word the adapter clocks is on the wire */
	uint32_t sent;     /* the game's word on the wire */
	uint32_t reply;    /* the adapter's */
	bool delivering;   /* an event's words go to the game */
	int count;
	int next;
	uint32_t words[MAX_EVENT_WORDS];
	uint32_t waitingSince;
	bool resetting;    /* the game has the port in general-purpose mode */
} rfu;

EM_JS(int, js_random16, (void), {
	const value = new Uint16Array(1);
	crypto.getRandomValues(value);
	return value[0];
});

uint16_t rfu_random16(void) {
	return (uint16_t) js_random16();
}

static void _noLog(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args) {
	(void) logger; (void) category; (void) level; (void) format; (void) args;
}
static struct mLogger quietLogger = { .log = _noLog };

static struct GBA* _gba(void) {
	return core ? (struct GBA*) core->board : NULL;
}

static void _frameEnded(void* context) {
	(void) context;
	for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; ++i) {
		displayBuffer[i] = videoBuffer[i] | 0xFF000000;
	}
	++framesDone;
}

// ==================== Wireless adapter ====================

/* The game's time, counted here: mGBA keeps its 64-bit count only in debugger builds. The
 * 32-bit count is read at least every tick, so it never wraps between readings. */
static uint64_t clockCycles;
static uint32_t clockMark;

static void _markClock(struct GBA* gba) {
	clockMark = (uint32_t) mTimingCurrentTime(&gba->timing);
}

/* The game's time in milliseconds; never 0, which the adapter core reads as "not set". */
static uint32_t _now(struct GBA* gba) {
	uint32_t current = (uint32_t) mTimingCurrentTime(&gba->timing);
	clockCycles += (uint32_t) (current - clockMark);
	clockMark = current;
	return (uint32_t) (clockCycles * 1000 / CYCLES_PER_SECOND) + 1000;
}

static uint32_t _readData(struct GBA* gba) {
	return gba->memory.io[REG_SIODATA32_LO >> 1] | ((uint32_t) gba->memory.io[REG_SIODATA32_HI >> 1] << 16);
}

static void _setSiocnt(struct GBA* gba, uint16_t value) {
	value = (value & ~SIOCNT_SI) | (rfu.si ? SIOCNT_SI : 0);
	gba->sio.siocnt = value;
	gba->memory.io[REG_SIOCNT >> 1] = value;
}

/* A word on the wire ends: the game reads value, Start clears, its serial interrupt comes. */
static void _endWord(struct GBA* gba, uint32_t value, uint32_t cyclesLate) {
	gba->memory.io[REG_SIODATA32_LO >> 1] = value & 0xFFFF;
	gba->memory.io[REG_SIODATA32_HI >> 1] = value >> 16;
	_setSiocnt(gba, gba->sio.siocnt & ~SIOCNT_START);
	if (gba->sio.siocnt & SIOCNT_IRQ) {
		GBARaiseIRQ(gba, GBA_IRQ_SIO, cyclesLate);
	}
}

/* A slave game waiting for the adapter's next word: external clock, Start set, SI and SO low. */
static bool _armed(uint16_t siocnt) {
	return (siocnt & (SIOCNT_INTERNAL_CLOCK | SIOCNT_SI | SIOCNT_SO)) == 0 && (siocnt & SIOCNT_START);
}

static void _clockNextEventWord(struct GBA* gba) {
	if (!rfu.delivering || rfu.adapterWord || rfu.gameWord || !_armed(gba->sio.siocnt)) {
		return;
	}
	rfu.adapterWord = true;
	mTimingDeschedule(&gba->timing, &rfu.word);
	mTimingSchedule(&gba->timing, &rfu.word, WORD_CYCLES_FAST);
}

static void _wordEvent(struct mTiming* timing, void* context, uint32_t cyclesLate) {
	(void) timing;
	(void) context;
	struct GBA* gba = _gba();
	if (!gba) {
		return;
	}
	uint32_t now = _now(gba);
	if (rfu.gameWord) {
		rfu.gameWord = false;
		/* The adapter holds SI high after each word until the game's SO says it is ready. */
		rfu.si = true;
		_endWord(gba, rfu.reply, cyclesLate);
		rfu_word(rfu.sent, now);
		return;
	}
	if (!rfu.adapterWord) {
		return;
	}
	rfu.adapterWord = false;
	if (!rfu.delivering || !rfu_delivering()) {
		/* The game reset the adapter meanwhile. */
		rfu.delivering = false;
		return;
	}
	uint32_t rx = _readData(gba);
	int index = rfu.next++;
	_endWord(gba, rfu.words[index], cyclesLate);
	if (index > 0 && rx == 0x996601EE) {
		/* The game rejected the event; it recovers on its own timer. */
		rfu.delivering = false;
		rfu_delivery_failed(now, true);
		return;
	}
	if (rfu.next == rfu.count) {
		rfu.delivering = false;
		rfu_delivered(now);
		return;
	}
	rfu.waitingSince = now;
}

static void _tickEvent(struct mTiming* timing, void* context, uint32_t cyclesLate) {
	(void) context;
	struct GBA* gba = _gba();
	if (!gba || !rfu.plugged) {
		return;
	}
	uint32_t now = _now(gba);
	int count = rfu_upkeep(now);
	if (count > 0 && !rfu.delivering) {
		rfu.delivering = true;
		rfu.count = count < MAX_EVENT_WORDS ? count : MAX_EVENT_WORDS;
		rfu.next = 0;
		for (int i = 0; i < rfu.count; ++i) {
			rfu.words[i] = rfu_event_word(i);
		}
		rfu.waitingSince = now;
	}
	if (rfu.delivering && !rfu.adapterWord) {
		if (_armed(gba->sio.siocnt) && !rfu.resetting) {
			_clockNextEventWord(gba);
		} else if (now - rfu.waitingSince > ARM_TIMEOUT_MS) {
			rfu.delivering = false;
			rfu_delivery_failed(now, rfu.next > 0);
		}
	}
	mTimingSchedule(timing, &rfu.tick, TICK_CYCLES - (int32_t) cyclesLate);
}

static bool _rfuLoad(struct GBASIODriver* driver) {
	(void) driver;
	struct GBA* gba = _gba();
	if (rfu.resetting && gba) {
		rfu.resetting = false;
		mTimingDeschedule(&gba->timing, &rfu.word);
		rfu.gameWord = false;
		rfu.adapterWord = false;
		rfu.delivering = false;
		rfu.si = true;
		rfu_sd_reset(_now(gba));
	}
	if (gba) {
		_setSiocnt(gba, gba->sio.siocnt);
	}
	return true;
}

/* Leaving normal mode for general-purpose mode is how librfu resets the adapter (its SD
 * pulse); the reset takes effect when the game comes back. */
static bool _rfuUnload(struct GBASIODriver* driver) {
	(void) driver;
	struct GBA* gba = _gba();
	if (gba && (gba->sio.rcnt & 0xC000) == 0x8000) {
		rfu.resetting = true;
	}
	return true;
}

static uint16_t _rfuWriteRegister(struct GBASIODriver* driver, uint32_t address, uint16_t value) {
	(void) driver;
	struct GBA* gba = _gba();
	if (address != REG_SIOCNT || !gba) {
		return value;
	}
	uint16_t old = gba->sio.siocnt;
	bool internal = value & SIOCNT_INTERNAL_CLOCK;
	bool soRose = (value & SIOCNT_SO) && !(old & SIOCNT_SO);
	bool soFell = !(value & SIOCNT_SO) && (old & SIOCNT_SO);
	if (internal) {
		/* The game as master raises SO once it is ready for the next word: the adapter, never
		 * busy for long, drops SI at once. */
		if (soRose) {
			rfu.si = false;
		}
	} else {
		/* As slave it raises SO while it takes a word and drops it to ask for the next. */
		if (soRose) {
			rfu.si = true;
		}
		if (soFell) {
			rfu.si = false;
		}
	}
	value = (value & ~SIOCNT_SI) | (rfu.si ? SIOCNT_SI : 0);

	/* A word the game clocks: Start set on the internal clock, by setting either. librfu's ID
	 * exchange sets Start on the external clock, then switches to the internal one. */
	bool start = value & SIOCNT_START;
	if (start && internal && (!(old & SIOCNT_START) || !(old & SIOCNT_INTERNAL_CLOCK)) && !rfu.gameWord && !rfu.adapterWord) {
		rfu.gameWord = true;
		rfu.sent = _readData(gba);
		rfu.reply = rfu_reply();
		mTimingDeschedule(&gba->timing, &rfu.word);
		mTimingSchedule(&gba->timing, &rfu.word, (value & SIOCNT_FAST) ? WORD_CYCLES_FAST : WORD_CYCLES_SLOW);
	}
	if (!internal && start && _armed(value)) {
		gba->sio.siocnt = value;
		_clockNextEventWord(gba);
	}
	return value;
}

static void _rfuReset(struct GBA* gba) {
	/* A reset starts the core's count over. */
	_markClock(gba);
	mTimingDeschedule(&gba->timing, &rfu.word);
	mTimingDeschedule(&gba->timing, &rfu.tick);
	rfu.si = true;
	rfu.gameWord = false;
	rfu.adapterWord = false;
	rfu.delivering = false;
	rfu.resetting = false;
	if (!rfu.plugged) {
		return;
	}
	rfu_power_on(_now(gba));
	mTimingSchedule(&gba->timing, &rfu.tick, TICK_CYCLES);
}

static void _rfuInstall(struct GBA* gba, bool plugged) {
	rfu.plugged = plugged;
	memset(&rfu.d, 0, sizeof(rfu.d));
	rfu.d.load = _rfuLoad;
	rfu.d.unload = _rfuUnload;
	rfu.d.writeRegister = _rfuWriteRegister;
	rfu.word.context = NULL;
	rfu.word.name = "Wireless adapter word";
	rfu.word.callback = _wordEvent;
	rfu.word.priority = 0x80;
	rfu.tick.context = NULL;
	rfu.tick.name = "Wireless adapter";
	rfu.tick.callback = _tickEvent;
	rfu.tick.priority = 0x81;
	GBASIOSetDriver(&gba->sio, plugged ? &rfu.d : NULL, SIO_NORMAL_32);
	_rfuReset(gba);
}

/* The board's SetMode(wireless): a fresh adapter. */
EMSCRIPTEN_KEEPALIVE void emu_rfu_enter_mode(int role) {
	struct GBA* gba = _gba();
	if (!gba || !rfu.plugged) {
		return;
	}
	mTimingDeschedule(&gba->timing, &rfu.word);
	rfu.gameWord = false;
	rfu.adapterWord = false;
	rfu.delivering = false;
	rfu.si = true;
	_setSiocnt(gba, gba->sio.siocnt);
	rfu_enter_mode(role);
}

/* Bytes from the board's data channel. */
EMSCRIPTEN_KEEPALIVE void emu_rfu_feed(const uint8_t* bytes, int length) {
	if (rfu.plugged) {
		rfu_feed(bytes, length);
	}
}

/* Takes the adapter's GB frames for the board; returns how many bytes it wrote. */
EMSCRIPTEN_KEEPALIVE int emu_rfu_take(uint8_t* out, int capacity) {
	return rfu.plugged ? rfu_take(out, capacity) : 0;
}

// ==================== Link cable ====================

static struct {
	struct GBASIODriver d;
	struct mTimingEvent event;
	bool plugged;
	int role;
	int seat;      /* the game's slot */
	int players;
	int phase;
	int baud;      /* of the transfer under way */
	bool answered; /* the game took part in the transfer under way */
	uint16_t rx;   /* the game's word in it */
	int id;        /* multiplayer id the game reads, set by its first transfer */
	uint16_t next[MAX_PLAYERS]; /* the other players' words for the next transfer, by slot */
} cable;

EM_JS(void, js_cable_transfer, (int rx), {
	if (Module.onCableTransfer) Module.onCableTransfer(rx);
});

static bool _cableActive(struct GBA* gba) {
	return gba->sio.mode == SIO_MULTI && gba->sio.activeDriver == &cable.d;
}

/* SIOCNT bits 2-7 as the game reads them in MULTI mode: SI, SD, id, error, busy. */
static uint16_t _cableStatusBits(void) {
	uint16_t bits = 0;
	/* SD high: every player ready. Without a link it stays low. */
	if (cable.role != CABLE_NONE) {
		bits |= 0x0008;
	}
	if (cable.role != CABLE_PARENT) {
		bits |= 0x0004; /* SI high: a child */
	}
	bits |= (cable.id & 3) << 4;
	if (cable.phase == PHASE_FINISH && cable.answered) {
		bits |= 0x0080;
	}
	return bits;
}

static void _cableSyncRegisters(void) {
	struct GBA* gba = _gba();
	if (!gba || !_cableActive(gba)) {
		return;
	}
	gba->sio.siocnt = (gba->sio.siocnt & ~0x00FC) | _cableStatusBits();
	if (cable.role == CABLE_PARENT) {
		gba->sio.rcnt &= ~4;
	} else {
		gba->sio.rcnt |= 4;
	}
}

static int _transferCycles(int baud) {
	return GBASIOCyclesPerTransfer[baud & 3][cable.players - 1];
}

/* The game's own transfer as the parent, or the link's as the parent of a child game. */
static void _cableBegin(struct GBA* gba, int baud) {
	cable.answered = _cableActive(gba);
	cable.baud = baud;
	cable.rx = 0xFFFF;
	if (cable.answered) {
		cable.rx = gba->memory.io[REG_SIOMLT_SEND >> 1];
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			gba->memory.io[(REG_SIOMULTI0 >> 1) + i] = 0xFFFF;
		}
		gba->sio.rcnt &= ~1;
	}
	cable.phase = PHASE_FINISH;
	_cableSyncRegisters();
	mTimingDeschedule(&gba->timing, &cable.event);
	mTimingSchedule(&gba->timing, &cable.event, _transferCycles(baud) + (cable.answered ? 0 : NO_REPLY_CYCLES));
}

static void _cableFinish(struct GBA* gba, uint32_t cyclesLate) {
	bool deliver = cable.answered && _cableActive(gba);
	/* A child sends in its own slot, later than the parent, so its serial interrupt for the last
	 * transfer may still set the word after this one started. */
	if (deliver && cable.role == CABLE_CHILD) {
		cable.rx = gba->memory.io[REG_SIOMLT_SEND >> 1];
	}
	uint16_t rx = cable.answered ? cable.rx : 0xFFFF;
	cable.phase = PHASE_IDLE;
	if (deliver) {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			uint16_t word = i < cable.players ? cable.next[i] : 0xFFFF;
			gba->memory.io[(REG_SIOMULTI0 >> 1) + i] = i == cable.seat ? rx : word;
		}
		cable.id = cable.seat;
		gba->sio.rcnt |= 1;
		_cableSyncRegisters();
		if (GBASIOMultiplayerIsIrq(gba->sio.siocnt)) {
			GBARaiseIRQ(gba, GBA_IRQ_SIO, cyclesLate);
		}
	}
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		cable.next[i] = 0xFFFF;
	}
	js_cable_transfer(rx);
}

static void _cableEvent(struct mTiming* timing, void* context, uint32_t cyclesLate) {
	(void) timing;
	(void) context;
	struct GBA* gba = _gba();
	if (!gba) {
		return;
	}
	if (cable.phase == PHASE_START) {
		_cableBegin(gba, BAUD_115200);
	} else if (cable.phase == PHASE_FINISH) {
		_cableFinish(gba, cyclesLate);
	}
}

static bool _cableLoad(struct GBASIODriver* driver) {
	(void) driver;
	_cableSyncRegisters();
	return true;
}

static uint16_t _cableWriteRegister(struct GBASIODriver* driver, uint32_t address, uint16_t value) {
	(void) driver;
	struct GBA* gba = _gba();
	if (address != REG_SIOCNT || !gba) {
		return value;
	}
	if (cable.role == CABLE_PARENT && (value & 0x0080) && cable.phase == PHASE_IDLE) {
		_cableBegin(gba, GBASIOMultiplayerGetBaud(value));
	}
	return (value & 0xFF03) | _cableStatusBits();
}

static void _cableStop(void) {
	struct GBA* gba = _gba();
	if (gba) {
		mTimingDeschedule(&gba->timing, &cable.event);
	}
	cable.phase = PHASE_IDLE;
	cable.answered = false;
}

/* role: enum CableRole. seat: the game's slot as a child (1-3). players: 2-4.
 * Stops a transfer under way; the words for the next one start out as 0xFFFF. */
EMSCRIPTEN_KEEPALIVE void emu_cable_config(int role, int seat, int players) {
	_cableStop();
	cable.role = cable.plugged && (role == CABLE_PARENT || role == CABLE_CHILD) ? role : CABLE_NONE;
	cable.players = players < 2 ? 2 : players > MAX_PLAYERS ? MAX_PLAYERS : players;
	cable.seat = cable.role == CABLE_CHILD && seat > 0 && seat < cable.players ? seat : 0;
	cable.id = 0;
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		cable.next[i] = 0xFFFF;
	}
	_cableSyncRegisters();
}

/* The other players' words for the next transfer, by slot; the game's own slot is ignored. */
EMSCRIPTEN_KEEPALIVE void emu_cable_stage(int slot0, int slot1, int slot2, int slot3) {
	cable.next[0] = (uint16_t) slot0;
	cable.next[1] = (uint16_t) slot1;
	cable.next[2] = (uint16_t) slot2;
	cable.next[3] = (uint16_t) slot3;
}

/* CABLE_CHILD: the next transfer starts this many cycles from now. */
EMSCRIPTEN_KEEPALIVE void emu_cable_clock(int cycles) {
	struct GBA* gba = _gba();
	if (!gba || cable.role != CABLE_CHILD || cable.phase == PHASE_FINISH) {
		return;
	}
	cable.phase = PHASE_START;
	mTimingDeschedule(&gba->timing, &cable.event);
	mTimingSchedule(&gba->timing, &cable.event, cycles > 0 ? cycles : 0);
}

/* A cable with nobody on the other end yet (CABLE_NONE). */
static void _cableInstall(struct GBA* gba, bool plugged) {
	cable.plugged = plugged;
	memset(&cable.d, 0, sizeof(cable.d));
	cable.d.load = _cableLoad;
	cable.d.writeRegister = _cableWriteRegister;
	cable.event.context = NULL;
	cable.event.name = "Link cable";
	cable.event.callback = _cableEvent;
	cable.event.priority = 0x80;
	GBASIOSetDriver(&gba->sio, plugged ? &cable.d : NULL, SIO_MULTI);
	emu_cable_config(CABLE_NONE, 0, 2);
}

// ==================== Link port ====================

static int linkKind = LINK_NOTHING;

/* What is plugged in starts over, as when the GBA is switched on. */
static void _installLink(struct GBA* gba) {
	_rfuInstall(gba, linkKind == LINK_WIRELESS);
	_cableInstall(gba, linkKind == LINK_CABLE);
}

/* What is plugged into the link port: 0 nothing, 1 the wireless adapter, 2 a link cable. A
 * game starts out with nothing. */
EMSCRIPTEN_KEEPALIVE void emu_link_use(int kind) {
	linkKind = kind == LINK_WIRELESS || kind == LINK_CABLE ? kind : LINK_NOTHING;
	struct GBA* gba = _gba();
	if (gba) {
		_installLink(gba);
	}
}

// ==================== Game ====================

EMSCRIPTEN_KEEPALIVE void emu_set_sample_rate(int rate) {
	audioRate = rate > 0 ? rate : 48000;
	if (core) {
		blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), audioRate);
		blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), audioRate);
	}
}

EMSCRIPTEN_KEEPALIVE void emu_unload(void) {
	if (!core) {
		return;
	}
	struct GBA* gba = _gba();
	mTimingDeschedule(&gba->timing, &rfu.word);
	mTimingDeschedule(&gba->timing, &rfu.tick);
	mTimingDeschedule(&gba->timing, &cable.event);
	core->unloadROM(core);
	mCoreConfigDeinit(&core->config);
	core->deinit(core);
	core = NULL;
	free(romData);
	romData = NULL;
}

EMSCRIPTEN_KEEPALIVE void* emu_alloc(size_t size) {
	return malloc(size);
}

EMSCRIPTEN_KEEPALIVE void emu_free(void* ptr) {
	free(ptr);
}

/* Takes ownership of rom (from emu_alloc); save may be NULL. The save buffer is
 * copied. Returns 0 on success. Nothing is plugged into the link port yet (emu_link_use). */
EMSCRIPTEN_KEEPALIVE int emu_load(void* rom, size_t romSize, const void* save, size_t saveSize) {
	emu_unload();
	mLogSetDefaultLogger(&quietLogger);
	romData = rom;
	struct VFile* vf = VFileFromMemory(romData, romSize);
	if (!vf) {
		free(romData);
		romData = NULL;
		return 1;
	}
	core = GBACoreCreate();
	if (!core || !core->isROM(vf)) {
		vf->close(vf);
		free(core);
		core = NULL;
		free(romData);
		romData = NULL;
		return 2;
	}
	mCoreInitConfig(core, NULL);
	core->init(core);
	mCoreConfigSetDefaultValue(&core->config, "idleOptimization", "ignore");
	mCoreConfigSetDefaultIntValue(&core->config, "useBios", 0);
	mCoreConfigSetDefaultIntValue(&core->config, "volume", 0x100);
	mCoreLoadConfig(core);
	core->setVideoBuffer(core, videoBuffer, VIDEO_WIDTH);
	core->setAudioBufferSize(core, AUDIO_BUFFER_SAMPLES);
	if (!core->loadROM(core, vf)) {
		mCoreConfigDeinit(&core->config);
		core->deinit(core);
		core = NULL;
		free(romData);
		romData = NULL;
		return 3;
	}
	struct VFile* sv = VFileMemChunk(save, save ? saveSize : 0);
	core->loadSave(core, sv);
	emu_set_sample_rate(audioRate);
	core->addCoreCallbacks(core, &(struct mCoreCallbacks) { .videoFrameEnded = _frameEnded });

	core->reset(core);
	memset(&rfu, 0, sizeof(rfu));
	memset(&cable, 0, sizeof(cable));
	linkKind = LINK_NOTHING;
	_installLink(_gba());
	framesDone = 0;
	memset(displayBuffer, 0, sizeof(displayBuffer));
	return 0;
}

/* Power-cycles the game and what is plugged into its link port; the battery save is kept. */
EMSCRIPTEN_KEEPALIVE void emu_reset(void) {
	if (!core) {
		return;
	}
	core->reset(core);
	/* The core's reset empties its timing queue and resets the port. */
	_installLink(_gba());
}

/* GBA keys: A 1, B 2, Select 4, Start 8, Right 16, Left 32, Up 64, Down 128, R 256, L 512. */
EMSCRIPTEN_KEEPALIVE void emu_set_keys(int keys) {
	if (core) {
		core->setKeys(core, (uint32_t) keys);
	}
}

/* Runs the game for at least the given number of cycles (16777216 a second);
 * returns how many it ran. */
EMSCRIPTEN_KEEPALIVE int emu_run(int cycles) {
	struct GBA* gba = _gba();
	if (!gba || cycles <= 0) {
		return 0;
	}
	uint32_t start = (uint32_t) mTimingCurrentTime(&gba->timing);
	uint32_t ran = 0;
	while (ran < (uint32_t) cycles) {
		core->runLoop(core);
		ran = (uint32_t) mTimingCurrentTime(&gba->timing) - start;
	}
	return (int) ran;
}

/* Frames finished since the game was loaded. */
EMSCRIPTEN_KEEPALIVE int emu_frames(void) {
	return framesDone;
}

/* The last finished frame, 240x160 RGBA. */
EMSCRIPTEN_KEEPALIVE color_t* emu_video(void) {
	return displayBuffer;
}

/* Interleaved stereo samples; returns the number of sample frames written. */
EMSCRIPTEN_KEEPALIVE int emu_audio(void) {
	if (!core) {
		return 0;
	}
	blip_t* left = core->getAudioChannel(core, 0);
	blip_t* right = core->getAudioChannel(core, 1);
	int avail = blip_samples_avail(left);
	if (avail > AUDIO_BUFFER_SAMPLES) {
		avail = AUDIO_BUFFER_SAMPLES;
	}
	int produced = blip_read_samples(left, audioOut, avail, true);
	blip_read_samples(right, audioOut + 1, avail, true);
	return produced;
}

EMSCRIPTEN_KEEPALIVE int16_t* emu_audio_buffer(void) {
	return audioOut;
}

/* Copies the battery save (the cartridge's flash, without the clock); read it
 * from emu_save_data(). */
EMSCRIPTEN_KEEPALIVE int emu_save_size(void) {
	free(saveCopy);
	saveCopy = NULL;
	saveCopySize = 0;
	if (!core) {
		return 0;
	}
	saveCopySize = core->savedataClone(core, &saveCopy);
	return (int) saveCopySize;
}

EMSCRIPTEN_KEEPALIVE void* emu_save_data(void) {
	return saveCopy;
}

/* Memory as the game sees it, without side effects; -1 with no game. */
EMSCRIPTEN_KEEPALIVE int emu_read8(int address) {
	return core ? (int) core->rawRead8(core, (uint32_t) address, -1) : -1;
}

EMSCRIPTEN_KEEPALIVE int emu_read16(int address) {
	return core ? (int) core->rawRead16(core, (uint32_t) address, -1) : -1;
}

EMSCRIPTEN_KEEPALIVE int emu_read32(int address) {
	return core ? (int) core->rawRead32(core, (uint32_t) address, -1) : 0;
}

EMSCRIPTEN_KEEPALIVE void emu_write8(int address, int value) {
	if (core) {
		core->rawWrite8(core, (uint32_t) address, -1, (uint8_t) value);
	}
}

EMSCRIPTEN_KEEPALIVE void emu_write16(int address, int value) {
	if (core) {
		core->rawWrite16(core, (uint32_t) address, -1, (uint16_t) value);
	}
}

EMSCRIPTEN_KEEPALIVE void emu_write32(int address, int value) {
	if (core) {
		core->rawWrite32(core, (uint32_t) address, -1, (uint32_t) value);
	}
}

EMSCRIPTEN_KEEPALIVE int emu_cpu_pc(void) {
	return core ? ((struct ARMCore*) core->cpu)->gprs[ARM_PC] : -1;
}
