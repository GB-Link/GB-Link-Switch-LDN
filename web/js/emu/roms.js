// The games the emulator links: Ruby, Sapphire, Emerald, FireRed and LeafGreen in every
// language and revision, recognized by the CRC-32 of the whole ROM. Any ROM plays; these name
// the game and tell the player when it may not link. FireRed, LeafGreen and Emerald link
// through the wireless adapter; Ruby and Sapphire have none and link by cable.

const GAMES = { AXV: 'Ruby', AXP: 'Sapphire', BPE: 'Emerald', BPR: 'FireRed', BPG: 'LeafGreen' };
const LANGUAGES = { J: 'Japanese', E: 'English', F: 'French', D: 'German', I: 'Italian', S: 'Spanish' };
const CABLE_GAMES = new Set(['Ruby', 'Sapphire']);

// [CRC-32, game code, revision, name of the dump]
const DUMPS = [
    ['cee9471a', 'AXVJ', 0, 'Pocket Monsters - Ruby (Japan)'],
    ['b448a337', 'AXVJ', 1, 'Pocket Monsters - Ruby (Japan) (Rev 1)'],
    ['f0815ee7', 'AXVE', 0, 'Pokemon - Ruby Version (USA, Europe)'],
    ['61641576', 'AXVE', 1, 'Pokemon - Ruby Version (USA, Europe) (Rev 1)'],
    ['aeac73e6', 'AXVE', 2, 'Pokemon - Ruby Version (USA, Europe) (Rev 2)'],
    ['690fd310', 'AXVF', 0, 'Pokemon - Version Rubis (France)'],
    ['9f981f72', 'AXVF', 1, 'Pokemon - Version Rubis (France) (Rev 1)'],
    ['15e1e280', 'AXVD', 0, 'Pokemon - Rubin-Edition (Germany)'],
    ['cae89464', 'AXVD', 1, 'Pokemon - Rubin-Edition (Germany) (Rev 1)'],
    ['c18231a9', 'AXVI', 0, 'Pokemon - Versione Rubino (Italy)'],
    ['9305e793', 'AXVI', 1, 'Pokemon - Versione Rubino (Italy) (Rev 1)'],
    ['eb0729cf', 'AXVS', 0, 'Pokemon - Edicion Rubi (Spain)'],
    ['b980fff5', 'AXVS', 1, 'Pokemon - Edicion Rubi (Spain) (Rev 1)'],
    ['fd1eeb78', 'AXPJ', 0, 'Pocket Monsters - Sapphire (Japan)'],
    ['01bd60e3', 'AXPJ', 1, 'Pocket Monsters - Sapphire (Japan) (Rev 1)'],
    ['554dedc4', 'AXPE', 0, 'Pokemon - Sapphire Version (USA, Europe)'],
    ['bafedae5', 'AXPE', 1, 'Pokemon - Sapphire Version (USA, Europe) (Rev 1)'],
    ['9cc4410e', 'AXPE', 2, 'Pokemon - Sapphire Version (USA, Europe) (Rev 2)'],
    ['3581a05f', 'AXPF', 0, 'Pokemon - Version Saphir (France)'],
    ['2ae49146', 'AXPF', 1, 'Pokemon - Version Saphir (France) (Rev 1)'],
    ['6fcd7a98', 'AXPD', 0, 'Pokemon - Saphir-Edition (Germany)'],
    ['b0c40c7c', 'AXPD', 1, 'Pokemon - Saphir-Edition (Germany) (Rev 1)'],
    ['0c7992a9', 'AXPI', 0, 'Pokemon - Versione Zaffiro (Italy)'],
    ['5efe4493', 'AXPI', 1, 'Pokemon - Versione Zaffiro (Italy) (Rev 1)'],
    ['a04f5f0b', 'AXPS', 0, 'Pokemon - Edicion Zafiro (Spain)'],
    ['f2c88931', 'AXPS', 1, 'Pokemon - Edicion Zafiro (Spain) (Rev 1)'],
    ['4881f3f8', 'BPEJ', 0, 'Pocket Monsters - Emerald (Japan)'],
    ['1f1c08fb', 'BPEE', 0, 'Pokemon - Emerald Version (USA, Europe)'],
    ['a3fdccb1', 'BPEF', 0, 'Pokemon - Version Emeraude (France)'],
    ['34c9df89', 'BPED', 0, 'Pokemon - Smaragd-Edition (Germany)'],
    ['a0aec80a', 'BPEI', 0, 'Pokemon - Versione Smeraldo (Italy)'],
    ['8c4d3108', 'BPES', 0, 'Pokemon - Edicion Esmeralda (Spain)'],
    ['3b2056e9', 'BPRJ', 0, 'Pocket Monsters - FireRed (Japan)'],
    ['bb640df7', 'BPRJ', 1, 'Pocket Monsters - FireRed (Japan) (Rev 1)'],
    ['dd88761c', 'BPRE', 0, 'Pokemon - FireRed Version (USA, Europe)'],
    ['84ee4776', 'BPRE', 1, 'Pokemon - FireRed Version (USA, Europe) (Rev 1)'],
    ['5dc668f6', 'BPRF', 0, 'Pokemon - Version Rouge Feu (France)'],
    ['1a81eedf', 'BPRD', 0, 'Pokemon - Feuerrote Edition (Germany)'],
    ['73a72167', 'BPRI', 0, 'Pokemon - Versione Rosso Fuoco (Italy)'],
    ['9f08064e', 'BPRS', 0, 'Pokemon - Edicion Rojo Fuego (Spain)'],
    ['0a48556b', 'BPGJ', 0, 'Pocket Monsters - LeafGreen (Japan)'],
    ['8da4601a', 'BPGJ', 1, 'Pocket Monsters - LeafGreen (Japan) (Rev 1)'],
    ['d69c96cc', 'BPGE', 0, 'Pokemon - LeafGreen Version (USA, Europe)'],
    ['daffecec', 'BPGE', 1, 'Pokemon - LeafGreen Version (USA, Europe) (Rev 1)'],
    ['ba3285e3', 'BPGF', 0, 'Pokemon - Version Vert Feuille (France)'],
    ['d12f1fdd', 'BPGD', 0, 'Pokemon - Blattgruene Edition (Germany)'],
    ['16974506', 'BPGI', 0, 'Pokemon - Versione Verde Foglia (Italy)'],
    ['2ca11d59', 'BPGS', 0, 'Pokemon - Edicion Verde Hoja (Spain)'],
];

const KNOWN = new Map(DUMPS.map(([crc, code, revision, name]) => {
    const game = GAMES[code.slice(0, 3)];
    const language = LANGUAGES[code[3]];
    return [crc, {
        crc, code, revision, name, game, language,
        title: language === 'English' ? `Pokémon ${game}` : `Pokémon ${game} (${language})`,
        link: CABLE_GAMES.has(game) ? 'cable' : 'wireless',
        known: true, linkChoice: false,
    }];
}));

const CRC_TABLE = (() => {
    const table = new Uint32Array(256);
    for (let i = 0; i < 256; i++) {
        let c = i;
        for (let k = 0; k < 8; k++) c = c & 1 ? (c >>> 1) ^ 0xedb88320 : c >>> 1;
        table[i] = c >>> 0;
    }
    return table;
})();

export function crc32(bytes) {
    let c = 0xffffffff;
    for (let i = 0; i < bytes.length; i++) c = CRC_TABLE[(c ^ bytes[i]) & 0xff] ^ (c >>> 8);
    return ((c ^ 0xffffffff) >>> 0).toString(16).padStart(8, '0');
}

/** A known dump by its CRC-32: { crc, code, revision, name, game, language, title, link, known }. */
export function knownRom(crc) {
    return KNOWN.get(crc) ?? null;
}

/** A ROM file's name without its extension, for naming a ROM that isn't a known dump. */
export function romFileName(fileName) {
    return fileName.replace(/\.(gba|agb|bin)$/i, '').trim() || 'Game';
}

/**
 * The game a ROM is, by its CRC-32: a known dump, or one named `name` that links like the game
 * its header names. `linkChoice`: its header names none of the games, so the player chooses how
 * it links.
 */
export function describeRom(crc, bytes, name) {
    const known = knownRom(crc);
    if (known) return known;
    const code = bytes.length >= 0xb0 ? String.fromCharCode(...bytes.subarray(0xac, 0xb0)) : '';
    const game = GAMES[code.slice(0, 3)];
    return {
        crc, code, name, game, language: LANGUAGES[code[3]], title: name,
        link: CABLE_GAMES.has(game) ? 'cable' : 'wireless',
        known: false, linkChoice: !game,
    };
}

/** The game a ROM file is, and what the player should know about it: { rom, notice }. */
export function identifyRom(bytes, fileName) {
    const rom = describeRom(crc32(bytes), bytes, romFileName(fileName));
    let notice = null;
    if (!rom.known) {
        notice = rom.game
            ? `This Pokémon ${rom.game} ROM isn't a known cartridge dump. It plays, but linking may not work.`
            : 'This isn\'t a known Pokémon Ruby, Sapphire, Emerald, FireRed or LeafGreen ROM. It plays, but linking may not work.';
    }
    return { rom, notice };
}
