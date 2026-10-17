#!/usr/bin/env node
// Writes predicted-roms.mjs: the addresses the cards use in the French, German,
// Italian and Spanish FireRed and LeafGreen of the Nintendo Switch, worked out
// from the English Switch builds and the European GBA ROMs, with the bytes the
// console must hold near each one. The page checks those bytes on the Switch
// (verify.s) before it sends a card built from them.
//
//   node cards/predict-roms.mjs <switch builds> <reference builds> <GBA ROMs> <GBA tables> [<Switch ROMs>]
//
// <switch builds>     pokefirered after `make firered_switch leafgreen_switch`,
//                     built without libisagbprn, as the European Switch ROMs
//                     are: AgbMain's `#if !defined(NDEBUG) || REVISION >= 0xA`
//                     around AGBPrintInit() made `#if !defined(NDEBUG)`, and
//                     src/isagbprn.o's two lines removed from ld_script_rev10.ld
//                     (newlib's vsprintf and what it pulls in then stay out)
// <reference builds>  pokefirered after `make firered_rev1 leafgreen_rev1`, built
//                     as the European GBA ROMs are: -DNDEBUG added to
//                     CPPFLAGS (no assertions, no AGBPrint) and the three
//                     `#if !defined(NDEBUG) || REVISION >= 0xA` in main.c's
//                     VBlankIntr and VCountIntr made `#if 1` (the VCOUNT stores
//                     stay)
// <GBA ROMs>          a folder with FireRed/FRA_1.0_FireRed.gba,
//                     LeafGreen/GER_1.0_LeafGreen.gba and the like
// <GBA tables>        gblink-wondercards' tools/native-cards/roms.mjs: the same
//                     symbols in the European GBA ROMs
// <Switch ROMs>       optional: a folder of European Switch ROMs, any file
//                     names; nothing is written unless every check holds in
//                     each of them
//
// The English Switch ROM links libisagbprn for its one AGBPrintInit() call, and
// with it newlib's vsprintf, whose code sits before the data and whose state
// sits in IWRAM before gMain; the European Switch ROMs leave it out. Otherwise
// the Switch build changed code, RAM layout and debug data, never a translated
// text, and a European GBA ROM differs from the reference only in its language
// (French and Spanish also lack 1.1's flash check in AgbMain, on the Switch
// too). A European Switch address is the Switch build's plus the European GBA
// ROM's distance from the reference at that symbol, except past the union
// room's Task_TryJoinLinkGroup: the European GBA ROMs lengthened it and the
// Switch build rewrote it the same way in every language, so the distance
// there is less by what they added. RAM is the Switch build's. A symbol only
// the Switch build has takes the distance at the next object both have.
//
// A routine is checked on 4 bytes of its code that the Switch build, the
// reference and the European ROM all hold; a table or script on 4 bytes the
// Switch build and the reference share, clear of anything that reads as an
// address, taken from the European ROM, or else on an address in it, predicted
// like the symbols. The 4 bytes appear nowhere else within SEARCH bytes, so a
// match the console finds off the prediction gives the distance. A RAM address
// is checked on a literal pool holding it, in a routine the Switch build didn't
// change, preferably before the union room; only the value is checked, so
// copies of it nearby don't matter.

import { execFileSync } from 'node:child_process';
import { readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { SYMBOLS, cardSymbols, symbolTable } from './symbols.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROM_BASE = 0x08000000;
const ROM_END = 0x0a000000;
// How far either side of a predicted address the check looks (verify.s).
const SEARCH = 16;
// Farther than this from a routine's start, a check would prove less about it.
const ROUTINE_CHECK_SPAN = 32;
const DATA_CHECK_SPAN = 96;
// The alignment's window and the share of compared halfwords that must match.
const ALIGN_WINDOW = 0x800;
const ALIGN_DIRECT = 0.9;
const ALIGN_SEARCHED = 0.8;
// The routine the European GBA ROMs lengthened and the Switch build rewrote.
const UNION_ROOM = 'src/union_room.o';
const UNION_ROUTINE = 'Task_TryJoinLinkGroup';

const GAMES = [
  { game: 'R', name: 'FireRed', build: 'pokefirered' },
  { game: 'G', name: 'LeafGreen', build: 'pokeleafgreen' },
];
const LANGUAGES = { F: 'FRA', D: 'GER', I: 'ITA', S: 'SPA' };

const [switchDir, referenceDir, romDir, tablesPath, switchRomDir] = process.argv.slice(2);
if (!tablesPath) {
  console.error('usage: predict-roms.mjs <switch builds> <reference builds> <GBA ROMs> <GBA tables> [<Switch ROMs>]');
  process.exit(1);
}
const { ROMS: GBA_ROMS } = await import(pathToFileURL(resolve(tablesPath)).href);

// The European Switch ROMs to check against, by header: 'BPGI 1.10'.
const SWITCH_ROMS = new Map();
for (const file of switchRomDir ? readdirSync(switchRomDir).filter((f) => /\.gba$/i.test(f)) : []) {
  const rom = readFileSync(join(switchRomDir, file));
  SWITCH_ROMS.set(`${String.fromCharCode(...rom.subarray(0xac, 0xb0))} 1.${rom[0xbc]}`, { file, rom });
}

// Routines with their sizes, by address.
function routines(elf) {
  const out = [];
  for (const line of execFileSync('arm-none-eabi-nm', ['-S', '-n', '--defined-only', elf], { encoding: 'utf8', maxBuffer: 1 << 28 }).split('\n')) {
    const [value, size, kind, name] = line.split(' ');
    if (!name || !'tT'.includes(kind)) continue;
    const addr = parseInt(value, 16) & ~1;
    if (addr >= ROM_BASE && addr < ROM_END && parseInt(size, 16) > 0) out.push({ addr, size: parseInt(size, 16), name });
  }
  return out;
}

// The objects of the map's .text, in link order.
function objects(map) {
  const out = [];
  for (const line of readFileSync(map, 'utf8').split('\n')) {
    const match = /^ \.text\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(\S+)/.exec(line);
    if (match && parseInt(match[2], 16)) out.push({ addr: parseInt(match[1], 16), size: parseInt(match[2], 16), name: match[3] });
  }
  return out;
}

const word = (rom, addr) => {
  const at = addr - ROM_BASE;
  return (rom[at] | (rom[at + 1] << 8) | (rom[at + 2] << 16) | (rom[at + 3] << 24)) >>> 0;
};
const looksLikeAddress = (value) => [0x02, 0x03, 0x08, 0x09].includes(value >>> 24);
const isBlHalf = (half) => (half >>> 12) === 0xf;
// Whether any 4 bytes overlapping the word at `addr`, at any alignment, read as
// a ROM address (script pointers are unaligned).
const nearPointer = (rom, addr) => {
  for (let at = addr - 3; at <= addr + 3; at++) if (word(rom, at) >= ROM_BASE && word(rom, at) < ROM_END) return true;
  return false;
};

// Where each reference routine sits in `rom`: address -> distance, for the ones
// found. Compared halfword by halfword, leaving out BL pairs and aligned words
// that look like addresses (literal pools); a routine is looked for near where
// the one before it was found.
function align(ref, fns, rom) {
  const placed = new Map();
  let delta = 0;
  for (const fn of fns) {
    if (fn.size < 16) continue;
    const off = fn.addr - ROM_BASE;
    const halves = fn.size >> 1;
    const skip = new Uint8Array(halves);
    for (let i = 0; i + 1 < halves; i++) {
      const a = ref[off + 2 * i] | (ref[off + 2 * i + 1] << 8);
      const b = ref[off + 2 * i + 2] | (ref[off + 2 * i + 3] << 8);
      if ((a & 0xf800) === 0xf000 && (b & 0xf800) === 0xf800) skip[i] = skip[i + 1] = 1;
    }
    for (let at = (fn.addr + 3) & ~3; at + 4 <= fn.addr + fn.size; at += 4) {
      if (looksLikeAddress(word(ref, at))) {
        const i = (at - fn.addr) >> 1;
        skip[i] = 1;
        if (i + 1 < halves) skip[i + 1] = 1;
      }
    }
    const keep = [];
    for (let i = 0; i < halves; i++) if (!skip[i]) keep.push(2 * i);
    if (keep.length < 8) continue;
    const score = (pos, best = -1) => {
      if (pos < 0 || pos + fn.size > rom.length) return 0;
      let n = 0;
      for (let k = 0; k < keep.length; k++) {
        const i = keep[k];
        if (rom[pos + i] === ref[off + i] && rom[pos + i + 1] === ref[off + i + 1]) n++;
        else if (n + (keep.length - k - 1) <= best) return n;
      }
      return n;
    };
    const guess = off + delta;
    if (score(guess) >= ALIGN_DIRECT * keep.length) {
      placed.set(fn.addr, delta);
      continue;
    }
    let best = -1;
    let bestPos = 0;
    for (let pos = Math.max(0, guess - ALIGN_WINDOW) & ~1; pos <= guess + ALIGN_WINDOW; pos += 2) {
      const s = score(pos, best);
      if (s > best) { best = s; bestPos = pos; }
    }
    if (best < ALIGN_SEARCHED * keep.length) {
      // Past a stretch whose size differs a lot (the scripts hold texts): the
      // longest run of compared halfwords, looked up anywhere in the ROM.
      let run = { at: 0, length: 0 };
      for (let k = 0, start = 0; k <= keep.length; k++) {
        if (k === keep.length || (k > start && keep[k] !== keep[k - 1] + 2)) {
          if (k - start > run.length) run = { at: keep[start], length: k - start };
          start = k;
        }
      }
      if (run.length < 8) continue;
      const needle = ref.subarray(off + run.at, off + run.at + 2 * run.length);
      for (let hit = rom.indexOf(needle); hit >= 0 && best < ALIGN_SEARCHED * keep.length; hit = rom.indexOf(needle, hit + 2)) {
        const pos = hit - run.at;
        if (pos & 1) continue;
        const s = score(pos);
        if (s > best) { best = s; bestPos = pos; }
      }
      if (best < ALIGN_SEARCHED * keep.length) continue;
    }
    delta = bestPos - off;
    placed.set(fn.addr, delta);
  }
  return placed;
}

const hex = (value) => `0x${(value >>> 0).toString(16).padStart(8, '0')}`;
const specOf = (spec) => (Array.isArray(spec) ? spec : spec.frlg) ?? null;

// The distance at the start of the first object from `index` on in `order`
// that the reference has and the alignment found a routine of.
function distanceFrom(order, index, refObjects, refFns, placed) {
  for (const obj of order.slice(index)) {
    const ref = refObjects.find((o) => o.name === obj.name);
    if (!ref) continue;
    const first = refFns.find((fn) => fn.addr >= ref.addr && fn.addr < ref.addr + ref.size && placed.has(fn.addr));
    if (first) return placed.get(first.addr);
  }
  throw new Error(`no object from ${order[index]?.name} on that the reference shares`);
}

// No other copy of `value` within SEARCH bytes of `addr` in `rom`.
function unique(rom, addr, value) {
  for (let d = -SEARCH; d <= SEARCH; d++) if (d && word(rom, addr + d) === value) return false;
  return true;
}

function predict(game, language) {
  const sw = {
    elf: join(switchDir, `${game.build}_switch.elf`), map: join(switchDir, `${game.build}_switch.map`),
    rom: readFileSync(join(switchDir, `${game.build}_switch.gba`)),
  };
  const ref = {
    elf: join(referenceDir, `${game.build}_rev1.elf`), map: join(referenceDir, `${game.build}_rev1.map`),
    rom: readFileSync(join(referenceDir, `${game.build}_rev1.gba`)),
  };
  const romPath = join(romDir, game.name, `${LANGUAGES[language]}_1.0_${game.name}.gba`);
  const gba = readFileSync(romPath);
  const code = String.fromCharCode(...gba.subarray(0xac, 0xb0));
  if (code !== `BP${game.game}${language}`) throw new Error(`${romPath} is ${code}, not BP${game.game}${language}`);
  const tables = GBA_ROMS[`${code} 1.0`]?.symbols;
  if (!tables) throw new Error(`no ${code} 1.0 in ${tablesPath}`);

  const swTable = symbolTable(sw.elf);
  const refTable = symbolTable(ref.elf);
  const swCard = cardSymbols(swTable, 'frlg');
  const refCard = cardSymbols(refTable, 'frlg');
  const swFns = routines(sw.elf);
  const refFns = routines(ref.elf);
  const refObjects = objects(ref.map);
  const swObjects = objects(sw.map);
  const swSize = new Map(swFns.map((fn) => [fn.name, fn]));
  const placed = align(ref.rom, refFns, gba);
  // A routine too short to align sits at the distance of the aligned ones
  // either side of it, when they agree.
  const alignedAt = refFns.filter((fn) => placed.has(fn.addr)).map((fn) => fn.addr);
  for (const fn of refFns) {
    if (placed.has(fn.addr)) continue;
    let lo = 0;
    let hi = alignedAt.length;
    while (lo < hi) {
      const mid = (lo + hi) >> 1;
      if (alignedAt[mid] < fn.addr) lo = mid + 1; else hi = mid;
    }
    if (lo > 0 && lo < alignedAt.length && placed.get(alignedAt[lo - 1]) === placed.get(alignedAt[lo])) {
      placed.set(fn.addr, placed.get(alignedAt[lo]));
    }
  }
  const id = `${code} 1.10`;

  // Without 1.1's flash check in AgbMain, VBlankIntr is 0x14 early.
  const vblank = refTable.get('VBlankIntr') & ~1;
  if (!placed.has(vblank)) throw new Error(`${id}: VBlankIntr not found`);
  const flashCheck = placed.get(vblank) === 0;

  // The bytes the European ROM added to the union room routine, which the
  // Switch build doesn't have: taken off every distance past it.
  const unionRoutine = refFns.find((fn) => fn.name === UNION_ROUTINE);
  const before = refFns.filter((fn) => fn.addr < unionRoutine.addr && placed.has(fn.addr)).at(-1);
  const after = refFns.find((fn) => fn.addr > unionRoutine.addr && placed.has(fn.addr));
  const unionGrowth = placed.get(after.addr) - placed.get(before.addr);
  const pastUnion = (refAddr) => (refAddr > unionRoutine.addr ? unionGrowth : 0);
  const swUnion = swTable.get(UNION_ROUTINE) & ~1;

  // The library code the Switch build moved forward must be the same size.
  for (const moved of ['src/librfu_stwi.o', 'src/librfu_rfu.o']) {
    const index = refObjects.findIndex((o) => o.name === moved);
    if (distanceFrom(refObjects, index, refObjects, refFns, placed) !== distanceFrom(refObjects, index + 1, refObjects, refFns, placed)) {
      throw new Error(`${id}: ${moved} differs in size in the European ROM`);
    }
  }

  // The GBA table's routines where the alignment put them.
  for (const [name, spec] of Object.entries(SYMBOLS)) {
    const [, , kind] = specOf(spec) ?? [];
    if (kind !== 'fn' || !refCard[name] || !tables[name]) continue;
    const at = refCard[name] & ~1;
    if (placed.has(at) && at + placed.get(at) !== (tables[name] & ~1)) {
      throw new Error(`${id}: ${name} is at ${hex(tables[name])} in the GBA table, the alignment says ${hex(at + placed.get(at))}`);
    }
  }

  // Where a symbol is in the European GBA ROM: its table's address, or for a
  // routine the table doesn't have, where the alignment found it.
  const gbaAddress = (name, kind) => {
    const at = refCard[name] & ~1;
    if (tables[name]) return tables[name];
    if (kind === 'fn' && placed.has(at)) return refCard[name] + placed.get(at);
    throw new Error(`${id}: no ${name} in the GBA table, nor where the alignment could place it`);
  };
  const symbols = {};
  const kinds = {};
  for (const [name, spec] of Object.entries(SYMBOLS)) {
    const [, , kind] = specOf(spec) ?? [];
    const value = swCard[name];
    if (!value) { symbols[name] = 0; continue; }
    kinds[name] = value < ROM_BASE ? 'ram' : kind === 'fn' ? 'routine' : 'data';
    if (value < ROM_BASE) {
      if (tables[name] !== refCard[name]) throw new Error(`${id}: ${name} is ${hex(tables[name])} in the European ROM, ${hex(refCard[name])} in the reference`);
      symbols[name] = value;
    } else if (!refCard[name]) {
      const index = swObjects.findIndex((o) => value >= o.addr && value < o.addr + o.size);
      symbols[name] = value + distanceFrom(swObjects, index + 1, refObjects, refFns, placed) - (value > swUnion ? unionGrowth : 0);
    } else {
      symbols[name] = value + (gbaAddress(name, kind) - refCard[name]) - pastUnion(refCard[name]);
    }
  }

  // What the console must hold, by symbol: [address, the 4 bytes as a word].
  const checks = {};
  const unionRoom = refObjects.find((o) => o.name === UNION_ROOM).addr;
  const literals = new Map();
  // Routines before the union room first.
  const byPlace = [...refFns.filter((fn) => fn.addr < unionRoom), ...refFns.filter((fn) => fn.addr >= unionRoom)];
  for (const fn of byPlace) {
    for (let at = (fn.addr + 3) & ~3; at + 4 <= fn.addr + fn.size; at += 4) {
      const value = word(ref.rom, at);
      if (!literals.has(value)) literals.set(value, []);
      literals.get(value).push({ at, fn });
    }
  }
  for (const [name, spec] of Object.entries(SYMBOLS)) {
    if (!kinds[name]) continue;
    const [symbol, offset = 0] = specOf(spec);
    if (kinds[name] === 'ram') {
      // A pool holds the variable's address, or the field's.
      let found = null;
      let swValue = 0;
      for (const extra of [0, offset]) {
        swValue = swTable.get(symbol) + extra;
        const refValue = refTable.get(symbol) + extra;
        found = (literals.get(refValue) ?? []).find(({ at, fn }) => {
          const swFn = swSize.get(fn.name);
          if (!swFn || swFn.size !== fn.size || !placed.has(fn.addr)) return false;
          const gl = at + placed.get(fn.addr);
          return word(sw.rom, swFn.addr + (at - fn.addr)) === swValue && word(gba, gl) === refValue;
        });
        if (found) break;
      }
      if (!found) throw new Error(`${id}: no literal pool to check ${name} (${symbol}) in`);
      const swFn = swSize.get(found.fn.name);
      checks[name] = [swFn.addr + placed.get(found.fn.addr) - pastUnion(found.fn.addr) + (found.at - found.fn.addr), swValue];
      continue;
    }
    const swAt = swCard[name] & ~1;
    const refAt = refCard[name] & ~1 || null;
    const glAt = refAt && gbaAddress(name, kinds[name] === 'routine' ? 'fn' : null) & ~1;
    const predicted = symbols[name] & ~1;
    let check = null;
    if (kinds[name] === 'routine') {
      for (let k = 0; k <= ROUTINE_CHECK_SPAN && !check; k += 2) {
        const value = word(sw.rom, swAt + k);
        if (isBlHalf(value & 0xffff) || isBlHalf(value >>> 16)) continue;
        // A routine only the Switch build has is checked on its own code.
        if (refAt && (word(ref.rom, refAt + k) !== value || word(gba, glAt + k) !== value || !unique(gba, glAt + k, value))) continue;
        if (!refAt && !unique(sw.rom, swAt + k, value)) continue;
        check = [predicted + k, value];
      }
    } else {
      if (!refAt) throw new Error(`${id}: ${name} is data only the Switch build has`);
      for (let k = 0; k <= DATA_CHECK_SPAN && !check; k++) {
        if (word(sw.rom, swAt + k) !== word(ref.rom, refAt + k)) continue;
        if (nearPointer(sw.rom, swAt + k) || nearPointer(ref.rom, refAt + k) || nearPointer(gba, glAt + k)) continue;
        const value = word(gba, glAt + k);
        if (unique(gba, glAt + k, value)) check = [predicted + k, value];
      }
      // Only addresses (a table of pointers): one of them, predicted like any
      // address in the ROM.
      for (let k = -swAt & 3; k <= DATA_CHECK_SPAN && !check; k += 4) {
        const [swValue, refValue, glValue] = [word(sw.rom, swAt + k), word(ref.rom, refAt + k), word(gba, glAt + k)];
        if (![swValue, refValue, glValue].every((v) => v >= ROM_BASE && v < ROM_END) || !unique(gba, glAt + k, glValue)) continue;
        check = [predicted + k, (swValue + (glValue - refValue) - pastUnion(refValue)) >>> 0];
      }
    }
    if (!check) throw new Error(`${id}: nothing to check ${name} on`);
    checks[name] = check;
  }
  symbols.EMERALD = 0;
  const real = SWITCH_ROMS.get(id);
  if (real) {
    const wrong = Object.entries(checks).filter(([, [at, value]]) => word(real.rom, at) !== value).map(([name]) => name);
    if (wrong.length) throw new Error(`${id}: ${real.file} does not hold ${wrong.join(', ')} where predicted`);
  }
  const shift = symbols.CB2_UPDATE_PARTY_MENU - swCard.CB2_UPDATE_PARTY_MENU;
  console.log(`${id}: ${Object.keys(checks).length} checks, flash check ${flashCheck ? 'present' : 'absent'}, `
    + `${placed.size}/${refFns.length} routines aligned, ${shift < 0 ? '-' : '+'}0x${Math.abs(shift).toString(16)} past the union room`
    + (real ? `, every check holds in ${real.file}` : ''));
  return [id, { family: 'frlg', game: game.game, language, revision: 10, symbols, checks }];
}

const roms = GAMES.flatMap((game) => Object.keys(LANGUAGES).map((language) => predict(game, language)));
const body = roms.map(([id, rom]) => {
  const symbols = Object.entries(rom.symbols).map(([name, value]) => `      ${name}: ${hex(value)},`).join('\n');
  const checks = Object.entries(rom.checks).map(([name, [at, value]]) => `      ${name}: [${hex(at)}, ${hex(value)}],`).join('\n');
  return `  '${id}': {\n    family: '${rom.family}', game: '${rom.game}', language: '${rom.language}', revision: ${rom.revision},\n`
    + `    symbols: {\n${symbols}\n    },\n    checks: {\n${checks}\n    },\n  },`;
}).join('\n');
writeFileSync(join(HERE, 'predicted-roms.mjs'), `// Written by predict-roms.mjs: the addresses the cards use in the European
// FireRed and LeafGreen of the Nintendo Switch (revision 10), and for each a
// check: [address, the 4 bytes there as a little-endian word], to be found
// within SEARCH bytes of it.

export const SEARCH = ${SEARCH};

export const PREDICTED_ROMS = {
${body}
};
`);
