// The GB-Link Team cards from gblink-wondercards, built for the Switch's
// FireRed and LeafGreen (revision 10) in English, French, German, Italian and
// Spanish by cards/build.mjs, which writes the payloads: English FireRed's, and
// every other ROM's as the bytes that differ from it. The Master Ball is a
// plain script for every ROM.

import { decodeBase64, romPayloads } from './payloads.js';

export const TEAM_EVENTS = [
    {
        id: 'custom-speed-0-5',
        label: 'Slow Down 0.5× (Press R)',
        description: 'Press R and the whole game runs at half speed; press R again to play normally. R works anywhere, and the speed stays as you set it until you press it again. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset. Original speed-up event by Decryptu.',
        payloads: {
            ...romPayloads(decodeBase64(`6gNlAAIAAAAAAMK7xsAAzcq/v77////////////////////////////////////////MAOjp5uLn
AN3oAOPiANXi2ADj2tqr////////////////////////uwDn5NnX3dXgAOjm3dffAN3nAOvV3ejd
4tv//////////////////97p5+gA2uPmAO3j6av////////////////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IA6NzZ////////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADdwC9XgAACGZtaGwCvZ0AAAhmbWhsAsrm2efnAMwA6OMA5ODV7QDV6ADc1eDaAOfk
2dnYq/7K5tnn5wDMANXb1d3iAOjjAOTg1e0A4uPm4dXg4O2t/87c3ecA293a6ADd5+K06ADa4+b+
6Nzd5wDb1eHZrf9wtQDwHvgeiAAgGIAQTRSkEEgEOALUIVgpUPrnDkgBaCofSxubCgHREWgA4BFg
ACkD0ApLGWBpHAFgAPAC+B6AcL2CI5sABCISBpsYcEfARgD8AwIMAgAA4CcAA7j9AwIwtXNIBIh0
SAFoATEBYHpIACgB0AEhAXAA8H34APCI+GVLAPB3+AEgBEIn0WxMZKUA8Cf4a0xkpQDwI/hsSAB4
ACgb0GVIICHCfgAqB9CCeQN6mkID0cJ5Q3qaQg7QJDABOfHRXEhBaAExQWBZTAAsBNBSSwDwTvgB
PPjnMLwBvABHALVaSAB4AChB0AAsP9BVSEFoamiRQjrRAWgqaJFCNtFSScmIyQsy0QDwfvhTSQAp
ENBIStJoUwAbGItCCtlFSAFpATEBYdEIATFSGgDVACLCYBzgAbRDSAAhwYUBhitoAPAX+EBIQWhr
aJlCDtEA8BD4APBZ+AK8QBoA1eQwNUrQYJFoATGRYAE8vucBsAG8AEcYRzZKNkgAiMBDwAXAD1F4
UHCIQxF4QUARcHBHELUpSEFpACkG0CtKEWCBaVFgACFBYYFhLkkAKSnQKEgCeAAqJdAgSMJpATKK
QgDTACLCYQAqHNEfSxxoFEqUQgPRXGgTSpRCB9AcaBJKlEIP0VxoEUqUQgvRGErSiNILB9ERSBxo
RGFcaIRhGkwcYFxgELwBvABHcEcVSACIoDgA1eQwcEfARjUHAAhpLQAI7ZwFCG2dBQiBWwEInUgB
CJwjAAMAAAAAYP8DAjQAAgIAAAAAAAAAAIAjAAO0egMCgP8DAjABAARx8QMC5AAAAAYAAAQCAAAA
qf0DAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmQCATAkBBodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIlQEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGQCATAkBA4xBwAIZS0ACImdBQgJnjwEAuwiVAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWQCATAkBA0xBwAIZS0ACHWdBQj1PAQC7CJUBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2QCATAkBBodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIlQEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZkAgEwJAQaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CJUBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURkAgEwJAQOMQcACGUtAAiJnQUICZ48BALsIlQEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlkAgEwJAQNMQcACGUtAAh1nQUI9TwEAuwiVAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNkAgEwJAQaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CJUBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-speed-0-75',
        label: 'Slow Down 0.75× (Press R)',
        description: 'Press R and the whole game runs at three-quarter speed; press R again to play normally. R works anywhere, and the speed stays as you set it until you press it again. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset. Original speed-up event by Decryptu.',
        payloads: {
            ...romPayloads(decodeBase64(`6wNlAAMAAAAAAKGtqKa5AM3Kv7++///////////////////////////////////////MAOjp5uLn
AN3oAOPiANXi2ADj2tqr////////////////////////uwDn5NnX3dXgAOjm3dffAN3nAOvV3ejd
4tv//////////////////97p5+gA2uPmAO3j6av////////////////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IA6NzZ////////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADewC9XgAACGZtaGwCvZ8AAAhmbWhsAsrm2efnAMwA6OMA5ODV7QDVAODd6Ojg2QDn
4OPr2ear/srm2efnAMwA1dvV3eIA6OMA5ODV7QDi4+bh1eDg7a3/ztzd5wDb3droAN3n4rToANrj
5v7o3N3nANvV4dmt/wAAcLUA8B74HogAIBiAEE0UpBBIBDgC1CFYKVD65w5IAWgqH0sbmwoB0RFo
AOARYAApA9AKSxlgaRwBYADwAvgegHC9giObAAQiEgabGHBHwEYA/AMCDAIAAOAnAAO4/QMCMLVz
SASIdEgBaAExAWB6SAAoAdABIQFwAPB9+ADwiPhlSwDwd/gBIARCJ9FsTGSlAPAn+GtMZKUA8CP4
bEgAeAAoG9BlSCAhwn4AKgfQgnkDeppCA9HCeUN6mkIO0CQwATnx0VxIQWgBMUFgWUwALATQUksA
8E74ATz45zC8AbwARwC1WkgAeAAoQdAALD/QVUhBaGpokUI60QFoKmiRQjbRUknJiMkLMtEA8H74
U0kAKRDQSErSaFMAGxiLQgrZRUgBaQExAWHRCAExUhoA1QAiwmAc4AG0Q0gAIcGFAYYraADwF/hA
SEFoa2iZQg7RAPAQ+ADwWfgCvEAaANXkMDVK0GCRaAExkWABPL7nAbABvABHGEc2SjZIAIjAQ8AF
wA9ReFBwiEMReEFAEXBwRxC1KUhBaQApBtArShFggWlRYAAhQWGBYS5JACkp0ChIAngAKiXQIEjC
aQEyikIA0wAiwmEAKhzRH0scaBRKlEID0VxoE0qUQgfQHGgSSpRCD9FcaBFKlEIL0RhK0ojSCwfR
EUgcaERhXGiEYRpMHGBcYBC8AbwAR3BHFUgAiKA4ANXkMHBHwEY1BwAIaS0ACO2cBQhtnQUIgVsB
CJ1IAQicIwADAAAAAGD/AwI0AAICAAAAAAAAAACAIwADtHoDAoD/AwIwAQAEcfEDAuQAAAAGAAAE
BAAAAKn9AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmgCATAoBBodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIlgEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGgCATAoBA4xBwAIZS0ACImdBQgJnkAEAuwiWAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWgCATAoBA0xBwAIZS0ACHWdBQj1QAQC7CJYBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2gCATAoBBodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIlgEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZoAgEwKAQaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CJYBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURoAgEwKAQOMQcACGUtAAiJnQUICZ5ABALsIlgEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUloAgEwKAQNMQcACGUtAAh1nQUI9UAEAuwiWAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNoAgEwKAQaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CJYBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-speed-2',
        label: 'Fast Forward 2× (Press R)',
        description: 'Press R and the game runs at double speed, including walking, battles and text; press R again to play normally. R works anywhere, and the speed stays as you set it until you press it again. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset. Original speed-up event by Decryptu.',
        payloads: {
            ...romPayloads(decodeBase64(`7gNlAAYAAAAAAL7Jz7zGvwDNyr+/vv/////////////////////////////////////MAOjp5uLn
AN3oAOPiANXi2ADj2tqr////////////////////////uwDn5NnX3dXgAOjm3dffAN3nAOvV3ejd
4tv//////////////////97p5+gA2uPmAO3j6av////////////////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IA6NzZ////////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADcwC9XgAACGZtaGwCvZgAAAhmbWhsAsrm2efnAMwA2uPmANjj6dbg2QDn5NnZ2Kv+
yubZ5+cAzADV29Xd4gDo4wDk4NXtAOLj5uHV4ODtrf/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA
29Xh2a3/AHC1APAe+B6IACAYgBBNFKQQSAQ4AtQhWClQ+ucOSAFoKh9LG5sKAdERaADgEWAAKQPQ
CksZYGkcAWAA8AL4HoBwvYIjmwAEIhIGmxhwR8BGAPwDAgwCAADgJwADuP0DAjC1c0gEiHRIAWgB
MQFgekgAKAHQASEBcADwffgA8Ij4ZUsA8Hf4ASAEQifRbExkpQDwJ/hrTGSlAPAj+GxIAHgAKBvQ
ZUggIcJ+ACoH0IJ5A3qaQgPRwnlDeppCDtAkMAE58dFcSEFoATFBYFlMACwE0FJLAPBO+AE8+Ocw
vAG8AEcAtVpIAHgAKEHQACw/0FVIQWhqaJFCOtEBaCpokUI20VJJyYjJCzLRAPB++FNJACkQ0EhK
0mhTABsYi0IK2UVIAWkBMQFh0QgBMVIaANUAIsJgHOABtENIACHBhQGGK2gA8Bf4QEhBaGtomUIO
0QDwEPgA8Fn4ArxAGgDV5DA1StBgkWgBMZFgATy+5wGwAbwARxhHNko2SACIwEPABcAPUXhQcIhD
EXhBQBFwcEcQtSlIQWkAKQbQK0oRYIFpUWAAIUFhgWEuSQApKdAoSAJ4ACol0CBIwmkBMopCANMA
IsJhACoc0R9LHGgUSpRCA9FcaBNKlEIH0BxoEkqUQg/RXGgRSpRCC9EYStKI0gsH0RFIHGhEYVxo
hGEaTBxgXGAQvAG8AEdwRxVIAIigOADV5DBwR8BGNQcACGktAAjtnAUIbZ0FCIFbAQidSAEInCMA
AwEAAABg/wMCNAACAgEAAAABAAAAgCMAA7R6AwKA/wMCMAEABHHxAwLkAAAABgAABAAAAACp/QMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmACATAgBBodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIlAEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGACATAgBA4xBwAIZS0ACImdBQgJnjgEAuwiUAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWACATAgBA0xBwAIZS0ACHWdBQj1OAQC7CJQBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2ACATAgBBodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIlAEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZgAgEwIAQaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CJQBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURgAgEwIAQOMQcACGUtAAiJnQUICZ44BALsIlAEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlgAgEwIAQNMQcACGUtAAh1nQUI9TgEAuwiUAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNgAgEwIAQaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CJQBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-speed-3',
        label: 'Fast Forward 3× (Press R)',
        description: 'Press R and the game runs up to three times as fast, including walking, battles and text; press R again to play normally. R works anywhere, and the speed stays as you set it until you press it again. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset. Original speed-up event by Decryptu.',
        payloads: {
            ...romPayloads(decodeBase64(`7wNlAAcAAAAAAM7Mw8rGvwDNyr+/vv/////////////////////////////////////MAOjp5uLn
AN3oAOPiANXi2ADj2tqr////////////////////////uwDn5NnX3dXgAOjm3dffAN3nAOvV3ejd
4tv//////////////////97p5+gA2uPmAO3j6av////////////////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IA6NzZ////////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADcwC9XgAACGZtaGwCvZgAAAhmbWhsAsrm2efnAMwA2uPmAOjm3eTg2QDn5NnZ2Kv+
yubZ5+cAzADV29Xd4gDo4wDk4NXtAOLj5uHV4ODtrf/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA
29Xh2a3/AHC1APAe+B6IACAYgBBNFKQQSAQ4AtQhWClQ+ucOSAFoKh9LG5sKAdERaADgEWAAKQPQ
CksZYGkcAWAA8AL4HoBwvYIjmwAEIhIGmxhwR8BGAPwDAgwCAADgJwADuP0DAjC1c0gEiHRIAWgB
MQFgekgAKAHQASEBcADwffgA8Ij4ZUsA8Hf4ASAEQifRbExkpQDwJ/hrTGSlAPAj+GxIAHgAKBvQ
ZUggIcJ+ACoH0IJ5A3qaQgPRwnlDeppCDtAkMAE58dFcSEFoATFBYFlMACwE0FJLAPBO+AE8+Ocw
vAG8AEcAtVpIAHgAKEHQACw/0FVIQWhqaJFCOtEBaCpokUI20VJJyYjJCzLRAPB++FNJACkQ0EhK
0mhTABsYi0IK2UVIAWkBMQFh0QgBMVIaANUAIsJgHOABtENIACHBhQGGK2gA8Bf4QEhBaGtomUIO
0QDwEPgA8Fn4ArxAGgDV5DA1StBgkWgBMZFgATy+5wGwAbwARxhHNko2SACIwEPABcAPUXhQcIhD
EXhBQBFwcEcQtSlIQWkAKQbQK0oRYIFpUWAAIUFhgWEuSQApKdAoSAJ4ACol0CBIwmkBMopCANMA
IsJhACoc0R9LHGgUSpRCA9FcaBNKlEIH0BxoEkqUQg/RXGgRSpRCC9EYStKI0gsH0RFIHGhEYVxo
hGEaTBxgXGAQvAG8AEdwRxVIAIigOADV5DBwR8BGNQcACGktAAjtnAUIbZ0FCIFbAQidSAEInCMA
AwIAAABg/wMCNAACAgIAAAACAAAAgCMAA7R6AwKA/wMCMAEABHHxAwLkAAAABgAABAAAAACp/QMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmACATAgBBodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIlAEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGACATAgBA4xBwAIZS0ACImdBQgJnjgEAuwiUAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWACATAgBA0xBwAIZS0ACHWdBQj1OAQC7CJQBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2ACATAgBBodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIlAEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZgAgEwIAQaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CJQBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURgAgEwIAQOMQcACGUtAAiJnQUICZ44BALsIlAEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlgAgEwIAQNMQcACGUtAAh1nQUI9TgEAuwiUAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNgAgEwIAQaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CJQBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-speed-4',
        label: 'Fast Forward 4× (Press R)',
        description: 'Press R and the game runs up to four times as fast, including walking, battles and text; press R again to play normally. R works anywhere, and the speed stays as you set it until you press it again. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset. Original speed-up event by Decryptu.',
        payloads: {
            ...romPayloads(decodeBase64(`8ANlAAgAAAAAAKW5AM3Kv7++///////////////////////////////////////////MAOjp5uLn
AN3oAOPiANXi2ADj2tqr////////////////////////uwDn5NnX3dXgAOjm3dffAN3nAOvV3ejd
4tv//////////////////97p5+gA2uPmAO3j6av////////////////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IA6NzZ////////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADdwC9XgAACGZtaGwCvZwAAAhmbWhsAsrm2efnAMwA6OMA5+TZ2dgA6NzZANvV4dkA
6eSr/srm2efnAMwA1dvV3eIA6OMA5ODV7QDi4+bh1eDg7a3/ztzd5wDb3droAN3n4rToANrj5v7o
3N3nANvV4dmt/wBwtQDwHvgeiAAgGIAQTRSkEEgEOALUIVgpUPrnDkgBaCofSxubCgHREWgA4BFg
ACkD0ApLGWBpHAFgAPAC+B6AcL2CI5sABCISBpsYcEfARgD8AwIMAgAA4CcAA7j9AwIwtXNIBIh0
SAFoATEBYHpIACgB0AEhAXAA8H34APCI+GVLAPB3+AEgBEIn0WxMZKUA8Cf4a0xkpQDwI/hsSAB4
ACgb0GVIICHCfgAqB9CCeQN6mkID0cJ5Q3qaQg7QJDABOfHRXEhBaAExQWBZTAAsBNBSSwDwTvgB
PPjnMLwBvABHALVaSAB4AChB0AAsP9BVSEFoamiRQjrRAWgqaJFCNtFSScmIyQsy0QDwfvhTSQAp
ENBIStJoUwAbGItCCtlFSAFpATEBYdEIATFSGgDVACLCYBzgAbRDSAAhwYUBhitoAPAX+EBIQWhr
aJlCDtEA8BD4APBZ+AK8QBoA1eQwNUrQYJFoATGRYAE8vucBsAG8AEcYRzZKNkgAiMBDwAXAD1F4
UHCIQxF4QUARcHBHELUpSEFpACkG0CtKEWCBaVFgACFBYYFhLkkAKSnQKEgCeAAqJdAgSMJpATKK
QgDTACLCYQAqHNEfSxxoFEqUQgPRXGgTSpRCB9AcaBJKlEIP0VxoEUqUQgvRGErSiNILB9ERSBxo
RGFcaIRhGkwcYFxgELwBvABHcEcVSACIoDgA1eQwcEfARjUHAAhpLQAI7ZwFCG2dBQiBWwEInUgB
CJwjAAMEAAAAYP8DAjQAAgIDAAAAAwAAAIAjAAO0egMCgP8DAjABAARx8QMC5AAAAAYAAAQAAAAA
qf0DAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmQCATAkBBodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIlQEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGQCATAkBA4xBwAIZS0ACImdBQgJnjwEAuwiVAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWQCATAkBA0xBwAIZS0ACHWdBQj1PAQC7CJUBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2QCATAkBBodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIlQEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZkAgEwJAQaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CJUBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURkAgEwJAQOMQcACGUtAAiJnQUICZ48BALsIlQEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlkAgEwJAQNMQcACGUtAAh1nQUI9TwEAuwiVAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNkAgEwJAQaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CJUBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-fast-text',
        label: 'Max Text Speed',
        description: 'All text prints at top speed, in the overworld and in battle, until the game is closed or reset. Talk to the deliveryman again after a reset.',
        payloads: {
            ...romPayloads(decodeBase64(`DwQ7AScAAAAYAMC7zc4Azr/Szv/////////////////////////////////////////I4wDh4+bZ
AOvV3ejd4tv/////////////////////////////////u+DgAOjZ7OgA5Obd4ujnANXoAOjj5ADn
5NnZ2P///////////////+ni6N3gAO3j6QDo6ebiAOPa2gDt4+nmANvV4dmt///////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADdwC9XgAACGZtaGwCvZ0AAAhmbWhsArvg4ADo2ezoAOTm3eLo5wDV6ADo4+QA5+TZ
2dj+4uPruADp4ujd4ADt4+kA6Onm4gDj2toA6NzZANvV4dmt/87c3ecA293a6ADd5+K06ADa4+b+
6Nzd5wDb1eHZrf9wtQDwHvgeiAAgGIAQTRSkEEgEOALUIVgpUPrnDkgBaCofSxubCgHREWgA4BFg
ACkD0ApLGWBpHAFgAPAC+B6AcL2CI5sABCISBpsYcEfARgD8AwLgAQAA4CcAA5D9AwIwtWlIBIhq
SAFoATEBYG9IACgB0AEhAXAA8Hf4XEsA8HP4ASAEQiPRY0xbpQDwI/hiTFulAPAf+F5IICHCfgAq
B9CCeQN6mkID0cJ5Q3qaQg7QJDABOfHRVUhBaAExQWBSTAAsBNBLSwDwTvgBPPjnMLwBvABHALVT
SAB4AChB0AAsP9BOSEFoamiRQjrRAWgqaJFCNtFLScmIyQsy0QDwcfhLSQApENBBStJoUwAbGItC
Ctk+SAFpATEBYdEIATFSGgDVACLCYBzgAbQ8SAAhwYUBhitoAPAX+DlIQWhraJlCDtEA8BD4APBM
+AK8QBoA1eQwLkrQYJFoATGRYAE8vucBsAG8AEcYRxC1KEhBaQApBtAqShFggWlRYAAhQWGBYS1J
ACkp0CdIAngAKiXQH0jCaQEyikIA0wAiwmEAKhzRH0scaBRKlEID0VxoE0qUQgfQHGgSSpRCD9Fc
aBFKlEIL0RdK0ojSCwfREEgcaERhXGiEYRlMHGBcYBC8AbwAR3BHE0gAiKA4ANXkMHBHNQcACGkt
AAjtnAUIbZ0FCIFbAQidSAEInCMAAwgAAABg/wMCNAACAgAAAAAAAAAAgCMAA7R6AwKA/wMCAAAA
AOQAAAAGAAAEAAAAAIP9AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRmQCATD8AxodBwAIUS0ACEmeBQjJngUIbVsBCIlIAQjsIiwEAtAi`),
                'BPRD 1.10': decodeBase64(`aAEBRGQCATD8Aw4xBwAIZS0ACImdBQgJnhQEAuwiLAQC0CI=`),
                'BPRI 1.10': decodeBase64(`aAEBSWQCATD8Aw0xBwAIZS0ACHWdBQj1FAQC7CIsBALQIg==`),
                'BPRS 1.10': decodeBase64(`aAEBU2QCATD8AxodBwAIUS0ACF2eBQjdngUIbVsBCIlIAQjsIiwEAtAi`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZkAgEw/AMaHQcACFEtAAhJngUIyZ4FCG1bAQiJSAEI7CIsBALQIg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURkAgEw/AMOMQcACGUtAAiJnQUICZ4UBALsIiwEAtAi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlkAgEw/AMNMQcACGUtAAh1nQUI9RQEAuwiLAQC0CI=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNkAgEw/AMaHQcACFEtAAhdngUI3Z4FCG1bAQiJSAEI7CIsBALQIg==`),
            }),
        },
    },
    {
        id: 'custom-travel-anywhere',
        label: 'Travel Anywhere (Fly with R, Run & Bike Indoors)',
        description: 'Press R outdoors to open the Fly map, with no Pokémon that knows Fly and no badge needed, and fly to any town you’ve visited with the usual animation. It works wherever Fly does: on foot, on a bike or surfing, but not indoors, in caves or during a cutscene. B closes the map. You can also run and ride your Bike everywhere, indoors and in caves included, once you have the Running Shoes or a Bike. In FireRed and LeafGreen, L still opens the Help menu. It lasts until the game is closed or reset; talk to the deliveryman again after a reset, or to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`FAQSACwAAAAEAM7Mu9C/xgC7yNPRwr/Mv//////////////////////////////////AxtMA693o
3ADMuAC8w8W/AN3i2OPj5uf/////////////////////yubZ5+cAzADj6ejY4+Pm5wDo4wDAxtO4
AOLj/////////////////8LHAOLZ2djZ2LgA1eLYAObp4gDV4tgAvMPFv//////////////////V
4u3r3Nnm2a0A0N3n3egA6NzZANjZ4N3q2ebt4dXi////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFowAACB+vAAAIRbsFowAACB+8AAAICrsFowAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2tAAAIZm4UCCENgAAAuwGZAAAIIxUQAAMnAb3jAAAIZm1obAK9HQEA
CGZuFAghDYABALsBmQAACCMVEAADOAG9QAEACGZtaGwCvVcBAAhmbWhsAr1rAQAIZm1obALN3NXg
4ADDAODZ6ADt4+kAwMbTAOvd6NwAzADV4tj+5uniANXi2AC8w8W/ANXi7evc2ebZrP++4+LZqwDJ
6ejY4+Pm57gA5ObZ5+cAzADo4wDAxtOt/sPoAODV5+jnAOni6N3gAO3j6QDm2efZ6K3/zubV6tng
ANXi7evc2ebZAN3nAOPirf7F2dnkAN3oAOPirP+81dffAOjjAOLj5uHV4ADo5tXq2eCr/73j4dkA
1tXX3wDV4u0A6N3h2av/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt/wAAcLUSSx6IACAY
gBFNFKQRSAQ4IVgpUPvRD0xgcAEgIHAOSAFoSxubCgLQYWBpHAFgBksegHC9CEgAIQFwAkgAKADQ
AXBwR8BGcfEDAggCAAQA/AMCzAEAAGD/AwLgJwAD8LVWTCB4AChC0FVPuIvABz7RVUgAKAHQASEB
cHhoU0mIQgvRYXgAKTLQACFhcFBKEWBMShFwT0l5YCngTkmIQibRUElIfgIiEENIdgEgCHYAIGBw
+I1AChrTSE0oeAAoFtFHSMB4ACgS0UZIwH1GSwDwc/gAKAvQAPBG+P8oB9CgcAEgKHBBSFAhQUsA
8GX4Y2gA8GL48LwBvABHELUoIUFDPExkGCCJACgQ0TpLAPBU+DpLAPBR+DlLAPBO+AEgACE4SwDw
SfgBICCBEL02SMCIAAQX1CFIgXg1SlFyASFBcDJLAPA5+DJLAPA2+DJLAPAz+DFLAPAw+BpIACEB
cBdIL0lBYBC98LX/JgAkZCBgQyxNLRgoAAshAPAc+AAoEtAoAC0hAPAW+AAoDNH/LgDRJgANJygA
OQAA8Az4EygH0AE3ES/20QE0Bizf0TAA8L0gAPC9ACIbSxhHYP8DAoAjAAO4JwADcfEDApGFEgjw
QgADoZ8FCG2dBQicEAADdHADAvhtAwK1mQUInfwDAo2rBwhgQwADUboJCBnBBggl/wUIYd8HCLR6
AwJN5wcInLADAqmbBQjJtQgILasHCG2GDAiAQgICkTMECA==`), {
                'BPGE 1.10': decodeBase64(`XAEBR6gEAWnUBAEl4AQBNegEASH0BAGd/AQBQQ==`),
                'BPRF 1.10': decodeBase64(`aAEBRjgDATCcBAXQIgADCKgEDjWIEghAQgAD/aAFCMmexAQCEZvMBBaVrAcIsEIAAw29CQh9wgYI
gQAGCGng6AQCVejwBA4FnQUI0bYICDWsBwjJiQQFAuUy`),
                'BPRD 1.10': decodeBase64(`aAEBRDgDATCcBAXQIgADCKgEDnmHEghAQgADPaAFCAmexAQCUZrMBBXVqwcIsEIAA028CQi9wQYI
wf8FCKnoBAGV8AQORZwFCBG2CAh1qwcICYkEBQEN`),
                'BPRI 1.10': decodeBase64(`aAEBSTgDATCcBAXQIgADCKgEDbmHEghAQgADKaAFCPXEBAI9mswEFcGrBwiwQgADObwJCKnBBgit
/wUIlegEAYHwBA4xnAUI/bUICGGrBwjpiAQFAvky`),
                'BPRS 1.10': decodeBase64(`aAEBUzgDATCcBAXQIgADCKgEDqWIEghAQgADEaEFCN2exAQCJZvMBBaprAcIsEIAAyG9CQiRwgYI
lQAGCH3g6AQCaejwBA4ZnQUI5bYICEmsBwjRiQQFAvky`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY4AwEwnAQF0CIAAwioBA4NiBIIQEIAA/2gBQjJnsQEAhGbzAQWlawHCLBCAAPhvAkI
fcIGCIEABgg94OgEAino8AQOBZ0FCKW2CAg1rAcInYkEBQLlMg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ4AwEwnAQF0CIAAwioBA5RhxIIQEIAAz2gBQgJnsQEAlGazAQV1asHCLBCAAMhvAkI
vcEGCMH/BQh96AQBafAEDkWcBQjltQgIdasHCN2IBAUBDQ==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk4AwEwnAQF0CIAAwipBAyHEghAQgADKaAFCPXEBAI9mswEFcGrBwiwQgADDbwJCKnB
Bgit/wUIaegEAVXwBA4xnAUI0bUICGGrBwi9iAQFAvky`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM4AwEwnAQF0CIAAwioBA59iBIIQEIAAxGhBQjdnsQEAiWbzAQWqawHCLBCAAP1vAkI
kcIGCJUABghR4OgEAj3o8AQOGZ0FCLm2CAhJrAcIpYkEBQL5Mg==`),
            }),
        },
    },
    {
        id: 'custom-pc-anywhere',
        label: 'PC Anywhere (Press R)',
        description: 'Press R in the field to open the Pokémon Storage System, the boxes you normally reach from a Pokémon Center’s PC: withdraw, deposit and move Pokémon and their items, then SEE YA! takes you back to where you were. It stays off in the Union Room. R no longer opens the Help menu while it’s on (L still does). It lasts until the game is closed or reset; talk to the deliveryman again to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`KgSJAEIAAAAMAMq9ALvI09HCv8y////////////////////////////////////////T4+nmANbj
7NnnuADj4tkA1uno6OPiANXr1e3/////////////////yubZ5+cAzADd4gDo3NkA2t3Z4NgA6OMA
6efZ/////////////////+3j6eYAyr205wDKycUbx8nIANbj7NnnrQDQ3efd6P/////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFowAACB+vAAAIRbsFowAACB+8AAAICrsFowAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2tAAAIZm4UCCENgAAAuwGZAAAIIxUQAAMvAb3jAAAIZm1obAK9HgEA
CGZuFAghDYABALsBmQAACCMVEAADQAG9PQEACGZtaGwCvWEBAAhmbWhsAr11AQAIZm1obALN3NXg
4ADDAODZ6ADt4+kA4+TZ4gDo3NkAyr3+693o3ADMuADr3Nnm2erZ5gDt4+kA1ebZrP++4+LZqwDK
5tnn5wDMAN3iAOjc2QDa3dng2ADo4/7p59kA6NzZAMq9uADp4ujd4ADt4+kA5tnn2eit/8q9ALvi
7evc2ebZAN3nAOPirf7F2dnkAN3oAOPirP+81dffAOjjAOjc2QDKvecA3eIAysnFG8fJyP69v8jO
v8zNq/+94+HZANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf9w
tRFLHogAIBiAEE0SpCwggAAEOCFYKVD70Q1MASAgcAxIAWhLG5sKAtBhYGkcAWAFSx6AcL0GSAAh
AXABSAFwcEfARnHxAwIIAgAEAPwDAmD/AwLgJwADkLUbTCB4ACgq0BpPuIvABybRGUgBIQFw+I1A
CiDTOGgXSYhCHNF4aBZJiEIY0RZIAHgAKBTRFUgAeAIoENEUSMB4ACgM0Q1IAHgCKAjSEUsA8Az4
ACgD0RBID0sA8Ab4Y2gA8AP4kLwBvABHGEfARmD/AwKAIwADcfEDAvatAwLtnAUIbZ0FCJwQAAOo
DwADdHADAj3sEQhx0gYIoPwDAiNRugkIaS8CACU8ACdrAgA=`), {
                'BPGE 1.10': decodeBase64(`XAEBR9ADARXdAwEl`),
                'BPRF 1.10': decodeBase64(`aAEBRjgDATCwAwLQIrwDBkmeBQjJntADBqnuEQjV090DAg29`),
                'BPRD 1.10': decodeBase64(`aAEBRDgDATCwAwLQIrwDBomdBQgJntADBuntEQgV090DAk28`),
                'BPRI 1.10': decodeBase64(`aAEBSTgDATCwAwLQIrwDBXWdBQj10AMGLe4RCAHT3QMCObw=`),
                'BPRS 1.10': decodeBase64(`aAEBUzgDATCwAwLQIrwDBl2eBQjdntADBhXvEQjp090DAiG9`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY4AwEwsAMC0CK8AwZJngUIyZ7QAwaB7hEI1dPdAwLhvA==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ4AwEwsAMC0CK8AwaJnQUICZ7QAwbB7REIFdPdAwIhvA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk4AwEwsAMC0CK8AwV1nQUI9dADBgXuEQgB090DAg28`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM4AwEwsAMC0CK8AwZdngUI3Z7QAwbt7hEI6dPdAwL1vA==`),
            }),
        },
    },
    {
        id: 'custom-walk-through-walls',
        label: 'Walk Through Walls (Press R)',
        description: 'Walk through walls, trees, rocks and counters. People still block the way, and ledges, water and height changes work as usual. It starts on; press R in the field to switch it off and on (a PC’s sounds tell which). Switching it off puts every wall back at once. It stays off in the Union Room. R no longer opens the Help menu while it’s on (L still does). It lasts until the game is closed or reset; talk to the deliveryman again to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`OQRcAFEAAAAcANG7xsUAzsLMyc/BwgDRu8bGzf/////////////////////////////I4+jc3eLb
AOfo1eLY5wDd4gDt4+nmAOvV7f//////////////////0dXg4OcA1eLYAOjm2dnnANfV4rToAOfo
4+QA7ePprf///////////8wA5+vd6Nfc2ecA3egA49raANXi2ADj4q0AzdnZ///////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFowAACB+vAAAIRbsFowAACB+8AAAICrsFowAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2tAAAIZm4UCCENgAAAuwGZAAAIIxUQAAMjAb3jAAAIZm1obAK9FwEA
CGZuFAghDYABALsBmQAACCMVEAADNgG9PQEACGZtaGwCvVQBAAhmbWhsAr1oAQAIZm1obALN3NXg
4ADDAODZ6ADt4+kA69Xg3wDo3Obj6dvc/uvV4ODnuADr3Nnm2erZ5gDt4+kA1ebZrP++4+LZqwDK
5tnn5wDMAN3iAOjc2QDa3dng2P7o4wDn693o19wA3egA49raANXi2ADj4q3/0dXg3wDO3Obj6dvc
ANHV4ODnAN3nAOPirf7F2dnkAN3oAOPirP/R1eDg5wDV5tkA69Xg4OcA1dvV3eKr/73j4dkA1tXX
3wDV4u0A6N3h2av/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt/wBwtRJLHogAIBiAEU0T
pEkggAAEOCFYKVD70Q5MASAgcGBwDUgBaEsbmwoC0GFgaRwBYAZLHoBwvQZIACEBcEFwAUgBcHBH
wEZx8QMCCAIABAD8AwJg/wMC4CcAA/C1M0wgeAAoPdAyT7iLwAc50TFIASEBcDhoMUmIQjLReGgw
SYhCLtEsSAB4Aigq0jFLAPAu+AAoJdEA8Cv4YHgAKCDQKkhAeSQhSEMpSUAYECFGXgIxRV4CPq8c
Aj00ACAAKQAmSwDwFPgCACAAKQAkSwDwDvgBNDAdhELw3QE1vULs3RJMY2gA8AP48LwBvABHGEcA
tfiNQAoX0xJIAHgAKBPREUgAeAIoD9EQSMB4ACgL0WB4ASFIQGBwEkkAKADREkkIAA1L//fj/wG8
AEdg/wMCgCMAA3HxAwL2rQMC7ZwFCG2dBQicEAADqA8AA3RwAwI0bgMCPewRCHHSBgjtxQUISccF
CBD9AwIU/QMCLwQAAiOZwQUIIyneBQgvAwACwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBRywEARU=`),
                'BPRF 1.10': decodeBase64(`aAEBRjADATAIBALQIhQEBkmeBQjJniwEDqnuEQjV0wYISccFCKXISQQH9cIFCCOF3w==`),
                'BPRD 1.10': decodeBase64(`aAEBRDADATAIBALQIhQEBomdBQgJniwEDentEQgV0wYIicYFCOVJBAY1wgUII8U=`),
                'BPRI 1.10': decodeBase64(`aAEBSTADATAIBALQIhQEBXWdBQj1LAQNLe4RCAHTBgh1xgUI0UkEBiHCBQgjsQ==`),
                'BPRS 1.10': decodeBase64(`aAEBUzADATAIBALQIhQEBl2eBQjdniwEDhXvEQjp0wYIXccFCLnISQQHCcMFCCOZ3w==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYwAwEwCAQC0CIUBAZJngUIyZ4sBA6B7hEI1dMGCEnHBQilyEkEB/XCBQgjhd8=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQwAwEwCAQC0CIUBAaJnQUICZ4sBA3B7REIFdMGCInGBQjlSQQGNcIFCCPF`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkwAwEwCAQC0CIUBAV1nQUI9SwEDQXuEQgB0wYIdcYFCNFJBAYhwgUII7E=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMwAwEwCAQC0CIUBAZdngUI3Z4sBA7t7hEI6dMGCF3HBQi5yEkEBwnDBQgjmd8=`),
            }),
        },
    },
    {
        id: 'custom-pokemon-follow',
        label: 'Pokémon Follow (Walks Behind You)',
        description: 'Your lead party Pokémon walks behind you in the overworld, as in later games, if the game has an overworld sprite of its species: Pikachu, Clefairy, Jigglypuff, Wigglytuff, Meowth, Psyduck, Slowpoke, Slowbro, Seel, Machop, Machoke, Poliwrath, Voltorb, Pidgey, Pidgeot, Spearow, Fearow, Doduo, Cubone, Chansey, Kangaskhan, Lapras, Kabuto and the Nidoran family (they turn to face where they go). It steps where you stepped, jumps ledges after you and comes along through doors; you and other people walk through it, and push past it where you can’t. Face it and press A to hear its cry. It hides while you bike or surf, and steps away while a menu or battle is on (back with your next step), so a save never keeps it. It lasts until the game is closed or reset.',
        payloads: {
            ...romPayloads(decodeBase64(`OAQZAFAAAAAUAMrJxRvHycgAwMnGxsnR///////////////////////////////////T4+nmAOTV
5uji2eYA69Xg3+cA693o3ADt4+n/////////////////0+Pp5gDg2dXYAMrJxRvHycgA69Xg3+cA
1tnc3eLY/////////////+3j6bgA3doA6NzZANvV4dkA3NXnAN3o5//////////////////////n
5Obd6NmtANDd593oAOjc2QDY2eDd6tnm7eHV4v//////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADTwC9XgAACGZtaGwCvXIAAAhmbWhsAsniAOni6N3gAO3j6QDm2efZ6Kv/ztzd5wDb
3droAN3n4rToANrj5v7o3N3nANvV4dmt/wAAAHC1DEseiAAgGIALTQ+kC0gEOCFYKVD70QlMCkgg
YApIAWhLG5sKAtAhYWkcAWABSx6AcL0IAgAEAPwDAuwCAABg/wMCAf8AAOAnAAPwtZJMIHgAKAvQ
kUiBi8kHB9FAaI9JiEID0QDwCfgA8Mz4I2kA8AP48LwBvABHGEcAtYpIQHkkIUhDiU0tGADw8viO
Tz94AC8C0I1PP3h/CINOACEyeNIHAtAyevAqBNAkNgExECn10QvgYXBxeYFCAdEALyfQAbQwAH9L
//fX/wG8/yFhcAAoAdAALwDQiOCCsOl6CQcJDwGRaYoAkSuKACHwInBPAPCH+AKwECh40mBwJCFI
Q2pONhgA8Hj4KGlgYAAg4HDwegAJAAEOMPByakgAeIAA8HBgSAB4HiEIQgvQAPBk+ClpYWAwaYhC
WNAAIOBwKYpqilTgKGlhaIhCFdBgYCh+AAkpfwExCRoUKQXRMHjCCQHRQAYI1BAhPSkA0R0hIXNo
aaBgASDgcOB4ACg20DB4wQkB0UAGMdQwAEtL//d0/wAg4HAhiWKJMIoIGnOK0xonewMhASgY0AIo
FdACIQEwE9ABMBDQAigL0QAhASsM0AIrCdABIQEzB9ABMwTQAisI0CGJYokK4BQneRgwADVL//dK
/3B4ICGIQ3BwAL0wADNL//dB/wC9cHggIQhDcHBwRzhHALUkTzhoJEmAOYhCLdEsSAB4ACgp0WF4
/ykm0Cp+EgfSDgQqA9gDIIAaAAQA4NAfKmmAGDJpkEIX0fiNQAgI0ih/ITgDKBDY//fU/ymKaorL
5x1IAXAwABdL//cL/xxI4YlBgBpL//cF/wC9ALULSEEhDUv/9/7+4IEYoQAii1oCMgArAdCDQvnR
Ugg3MYhcAL1g/wMCgCMAA22dBQiAQgICdHADAjRuAwKRMwQI1R8GCEl0BgjZdAYIyS4GCFkcBgic
EAADqA8AA0RDAANx0gYIkP4DAlqhAAAAAMUCGQAjACcAKAA0ADYATwBQAFYAQgBDAD4AZAAQABIA
FQAWAFQAaABxAHMAgwCMAB0AIAAhAAAAwEZ4cXODfXmAgX6ChnB/dHJuhYRvdXeHk3p7fAAA`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRiwCATCABAbQIgADyZ6UBBblMgQIMSEGCKV1Bgg1dgYIJTAGCLUdtAQGlEIAA9XT`),
                'BPRD 1.10': decodeBase64(`aAEBRCwCATCABAbQIgADCZ6UBBUNMwQIcSAGCOV0Bgh1dQYIZS8GCPW0BAaUQgADFdM=`),
                'BPRI 1.10': decodeBase64(`aAEBSSwCATCABAXQIgAD9ZQEFfkyBAhdIAYI0XQGCGF1BghRLwYI4bQEBpRCAAMB0w==`),
                'BPRS 1.10': decodeBase64(`aAEBUywCATCABAbQIgAD3Z6UBBb5MgQIRSEGCLl1BghJdgYIOTAGCMkdtAQGlEIAA+nT`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYsAgEwgAQG0CIAA8melAQW5TIECDEhBgildQYINXYGCCUwBgi1HbQEBpRCAAPV0w==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQsAgEwgAQG0CIAAwmelAQVDTMECHEgBgjldAYIdXUGCGUvBgj1tAQGlEIAAxXT`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUksAgEwgAQF0CIAA/WUBBX5MgQIXSAGCNF0BghhdQYIUS8GCOG0BAaUQgADAdM=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMsAgEwgAQG0CIAA92elAQW+TIECEUhBgi5dQYISXYGCDkwBgjJHbQEBpRCAAPp0w==`),
            }),
        },
    },
    {
        id: 'custom-hm-moves',
        label: 'HM Moves Without HMs',
        description: 'With the badge that allows each move, your party uses Cut, Rock Smash, Strength, Surf and Waterfall without any Pokémon knowing it: talk to the tree, rock or boulder, or press A facing the water, as usual. The first Pokémon in your party that isn’t an Egg does the work. Dark caves light up by themselves once you have the Boulder Badge, as Flash would light them. It lasts until the game is closed or reset; talk to the deliveryman again to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`LASDAEQAAAAAAMLHAMfJ0L/NuADIyQDCx+f////////////////////////////////T4+nmANbV
2NvZ5wDV5tkA2eLj6dvc////////////////////////vc/OuADNz8zAuADNzsy/yMHOwgDV4tgA
4ePm2bj//////////////+LjAMrJxRvHycgA4tnZ2OcA6OMA3+Lj6wDo3Nnhrf/////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFogAACB+vAAAIRbsFogAACB+8AAAICrsFogAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2sAAAIZm4UCCENgAAAuwGYAAAIIxUQAAMbAb3YAAAIZm1obAK9EQEA
CGZuFAghDYABALsBmAAACBEAYP8DAr02AQAIZm1obAK9TAEACGZtaGwCvWABAAhmbWhsAtHV4ugA
6OMA6efZAMLHAOHj6tnnAOvd6Nzj6ej+6NnV19zd4tsA6NzZ4az/vuPi2asA0+Pp5gDW1djb2ecA
1ebZANXg4ADt4+n+4tnZ2ADi4+u4AOni6N3gAO3j6QDm2efZ6K3/yOMAwsfnAOLZ2djZ2ADi4+ut
/sXZ2eQA3egA6NzV6ADr1e2s/7zV198A6OMA6NnV19zd4tsAwsfnq/+94+HZANbV198A1eLtAOjd
4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AMLUKTQykgyCAAAQ4IVgpUPvRB0wB
ICBwBkgBaEsbmwoC0GFgaRwBYDC9wEYA/AMCYP8DAuAnAAPwtVtMW0+4i8AHGtEAJiB4ACgV0Hho
WEmIQhHRV0gAeAIoDdIA8BL4APA9+AYAYXgIQAXQAPBG+AAoAdEA8HX4ZnBjaPC8AbyGRhhHYLVQ
TSh4ASgm0ehoYKYEIzFoiEID0Aw2ATv50RzgMIkA8H34ACgX0LB6ACgE0E9LAPB2+AAoD9AA8GT4
/ygL0EZJCIBwaE+hCIEADEiBqWAAIChwASBocGC9ACA7SQl4ACkI0ThJCXgCKQTROEnJeAApANEB
IHBHILX4jUAILNOBsGhGgRwrSwDwSvhqRhCIUYgBsChLAPBD+ChLAPBA+CdNACgQ0TRIAPA5+AAo
FNAtSwDwNfgAKA/QLk0A8CL4/ygK0CVJCIAuoAWBKQxBgSRLAPAl+AEgIL0AICC9ALUcSEB9ASgM
0SVIAPAY+AAoB9EiSADwE/gAKALQHEsA8A/4AL0USQAgynxSB1IPAioE0GQxATAGKPbR/yBwRxBL
GEfARmD/AwKAIwADbZ0FCPatAwKZ/AUIHccFCG3UBQgVohoIsA8AA6gPAAOcEAADdHADAvhtAwKA
QgICzHADAj0eBwhx0gYIVQAGCDEABggl0wwI7KEaCCQIAAAgCAAABggAACNRugkIaAAFAAAAAJIW
HAg8FhwIIQgAAJwXHAg1FxwIJQgAAJAYHAhPGBwIIwgAAAsaHAjXGRwIJggBAA==`), {
                'BPGE 1.10': decodeBase64(`XAEBR5QEAvGhxAQF+dIMCMjZBAEl5AQFbhYcCBjwBAV4FxwIEfwEBWwYHAgrCAUF5xkcCLM=`),
                'BPRF 1.10': decodeBase64(`aAEBRgQDATB8BAbQIgADyZ6IBA71/QUIecgFCMnVBQjbkrQEFkUfBwjV0wYIsQEGCI0BBgiB1gwI
spLZBAINveQEBqQFHAhOBfAEBqsGHAhEBvwEBpwHHAhbBwgFBgoJHAjWCA==`),
                'BPRD 1.10': decodeBase64(`aAEBRAQDATB8BAbQIgADCZ6IBA41/QUIuccFCAnVBQg+1rQEFoUeBwgV0wYI8QAGCM0ABgjB1QwI
FdbZBAJNvOQEBl9WHAgJVvAEBoNXHAgcV/wEBnxYHAg7WAgFBtBZHAicWQ==`),
                'BPRI 1.10': decodeBase64(`aAEBSQQDATB8BAXQIgAD9YgEDiH9BQilxwUI9dQFCNmAtAQWcR4HCAHTBgjdAAYIuQAGCKHVDAiw
gNkEAjm85AQHO/IbCOXxG/AEBzfzGwjQ8hv8BAcj9BsI4vMbCAUHbfUbCDn1Gw==`),
                'BPRS 1.10': decodeBase64(`aAEBUwQDATB8BAbQIgAD3Z6IBA4J/gUIjcgFCN3VBQhGo7QEFlkfBwjp0wYIxQEGCKEBBgiJ1gwI
HaPZBAIhveQEBgkZHAizGPAEBv0ZHAiWGfwEBuYaHAilGggFBjQcHAgAHA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYEAwEwfAQG0CIAA8meiAQO9f0FCHnIBQjJ1QUIt5K0BBZFHwcI1dMGCLEBBgiNAQYI
VdYMCI6S2QQC4bzkBAaABRwIKgXwBAaHBhwIIAb8BAZ4BxwINwcIBQbmCBwIsgg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQEAwEwfAQG0CIAAwmeiAQONf0FCLnHBQgJ1QUIGta0BBaFHgcIFdMGCPEABgjNAAYI
ldUMCPHV2QQCIbzkBAY7VhwI5VXwBAZfVxwI+Fb8BAZYWBwIF1gIBQasWRwIeFk=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkEAwEwfAQF0CIAA/WIBA4h/QUIpccFCPXUBQi1gLQEFnEeBwgB0wYI3QAGCLkABgh1
1QwIjIDZBAINvOQEBxfyGwjB8RvwBAcT8xsIrPIb/AQH//MbCL7zGwgFB0n1GwgV9Rs=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMEAwEwfAQG0CIAA92eiAQOCf4FCI3IBQjd1QUIIqO0BBZZHwcI6dMGCMUBBgihAQYI
XdYMCPmi2QQC9bzkBAblGBwIjxjwBAbZGRwIchn8BAbCGhwIgRoIBQYQHBwI3Bs=`),
            }),
        },
    },
    {
        id: 'custom-no-encounters',
        label: 'No Wild Encounters & Repel',
        description: 'Choose which wild Pokémon stay away: ALL OF THEM, until the game is closed or reset (fishing, Rock Smash and Sweet Scent still find Pokémon); the WEAKER ONES, with a Repel that lasts 65,535 steps and is saved with your game; or NONE, which brings them all back.',
        payloads: {
            ...romPayloads(decodeBase64(`/AMpABQAAAAcAMjJAL/IvcnPyM6/zM0ALQDMv8q/xv/////////////////////////R3eDYAMrJ
xRvHyci4AOfo1e0A1evV7av/////////////////////xdnZ5ADV4OAA693g2ADKycUbx8nIANXr
1e24/////////////////+PmAOPi4O0A6NzZAOvZ1d/Z5gDj4tnnrQDQ3efd6P/////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFtgAACB+vAAAIRbsFtgAACB+8AAAICrsFtgAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73AAAAIZiMVEAADRQEnIQ2AAQC7AYIAAAghDYACALsBlwAACCENgAAAuwWsAAAIEQHYhgMC
veUAAAhmbWhsAhEA2IYDAhYgQP//vRQBAAhmbWhsAhEA2IYDAhYgQAAAvToBAAhmbWhsAr1bAQAI
Zm1obAK9bwEACGZtaGwC0dzd19wA693g2ADKycUbx8nIAOfc4+ng2P7n6NXtANXr1e2s/8jj4tkA
693g4ADV5OTZ1eYA6eLo3eAA7ePp/ujp5uIA49raAO3j6eYA29Xh2a3/xt3f2QDVAMy/yr/GAOjc
1egA4NXn6Of+p6a4pqSmAOfo2eTnq//R3eDYAMrJxRvHycgA1ebZANbV198A6OP+4uPm4dXgq/+9
4+HZANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAAGgAyEK
Ilbgu8bGAMnAAM7Cv8f/0b+7xb/MAMnIv83/yMnIv/8AwEbwtYWwDQAXACtOACMCyAAiBsYBM6tC
+dEnThwgwBsAIToALaNsHhtdJUwA8C/4BwAAISRMAPAq+A4hAJEBlQKWACEDkQIhBJE4AAgiAiMe
TADwHPgOIQCRAZUAIQKROAACIQAiAiMZTADwEPgAICkAOgACIxZMAPAJ+AAgFUwA8AX4Dkj/IQGA
BbDwvSBHELWIsAAjnABsRCBgBHgBMP8s+9EDMIAIgAABM4tC8tFoRv/3pv8IsBC9sPsDAsxwAwK5
DQoIya4PCHk0EQhpMBEIfQMKCB2fDwgCBAYHCQsNDg==`), {
                'BPGE 1.10': decodeBase64(`XAEBR8gDFo0NCgihrg8IUTQRCEEwEQhRAwoI9Z4=`),
                'BPRF 1.10': decodeBase64(`aAEBRsgDFkEQCgi1sg8IwTYRCLEyEQgFBgoICaM=`),
                'BPRD 1.10': decodeBase64(`aAEBRMgDFoEPCgj1sQ8IATYRCPExEQhFBQoISaI=`),
                'BPRI 1.10': decodeBase64(`aAEBScgDFm0PCgj1sQ8IRTYRCDUyEQgxBQoISaI=`),
                'BPRS 1.10': decodeBase64(`aAEBU8gDFlUQCgjdsg8ILTcRCB0zEQgZBgoIMaM=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbIAxYVEAoIjbIPCJk2EQiJMhEI2QUKCOGi`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTIAxZVDwoIzbEPCNk1EQjJMREIGQUKCCGi`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnIAxZBDwoIzbEPCB02EQgNMhEIBQUKCCGi`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPIAxYpEAoItbIPCAU3EQj1MhEI7QUKCAmj`),
            }),
        },
    },
    {
        id: 'custom-national-dex',
        label: 'National Pokédex Unlock',
        description: 'Upgrades your Pokédex to the National Pokédex right away, instead of after beating the Elite Four and catching 60 Kanto Pokémon. It also lets Kanto Pokémon evolve into later species, and Celio needs it before he sends you on the errand that earns the Rainbow Pass, which the Aurora and Mystic Tickets need. It opens up trading too: a game without the National Pokédex can’t trade away or be sent Pokémon from outside its regional Pokédex, or Eggs. With this card on both games, Emerald and FireRed or LeafGreen trade any Pokémon both ways, right away with this page’s National Dex bypass on, otherwise once both players have entered the Hall of Fame and the Switch player has finished the Sevii Islands story. You need a Pokédex first.',
        payloads: {
            ...romPayloads(decodeBase64(`CASJACAAAAAIAMi7zsPJyLvGAMrJxRu+v9L///////////////////////////////+/6tnm7QDK
ycUbx8nIuADm3dvc6ADV69Xt////////////////////z+Tb5tXY2QDt4+nmAMrJxRu+v9IA6OMA
6NzZ/////////////////8i7zsPJyLvGAMrJxRu+v9IA4uPrrQDQ3efd6P/////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFfAAACB+vAAAIRbsFfAAACB+8AAAICrsFfAAACCspCLsAXgAACCtACLsBaAAACL2G
AAAIZm4UCCENgAAAuwFyAAAIJW8BvbwAAAhmbWhsAr3sAAAIZm1obAK9CgEACGZtaGwCvTgBAAhm
bWhsAr1MAQAIZm1obALN3NXg4ADDAOnk2+bV2NkA7ePp5gDKycUbvr/S/ujjAOjc2QDIu87Dyci7
xgDKycUbvr/SrP++4+LZqwDT4+nmAMrJxRu+v9IA3ecA4uPrAOjc2f7Iu87Dyci7xgDKycUbvr/S
rf/T4+kA2OPitOgA3NXq2QDVAMrJxRu+v9IA7dnoq//T4+nmAMrJxRu+v9IA3ecA1eDm2dXY7QDo
3Nn+yLvOw8nIu8YAysnFG76/0qv/vePh2QDW1dffANXi7QDo3eHZq//O3N3nANvd2ugA3efitOgA
2uPm/ujc3ecA29Xh2a3/`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRg==`),
                'BPRD 1.10': decodeBase64(`aAEBRA==`),
                'BPRI 1.10': decodeBase64(`aAEBSQ==`),
                'BPRS 1.10': decodeBase64(`aAEBUw==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM=`),
            }),
        },
    },
    {
        id: 'custom-shiny-hunting',
        label: 'Shiny Hunting (Boosted Odds)',
        description: 'Wild Pokémon are shiny 1 in 1024 times instead of 1 in 8192, and the odds climb with a chain of one species: catching one adds 2, defeating one adds 1, reaching 1 in 64 at a chain of 30. Catching or defeating another species starts a new chain; running away leaves it as it is. Wild Pokémon may flee (1 turn in 20, never while asleep or in a legendary’s battle), and one that flees ends the chain. Press R in the field to see your chain; R no longer opens the Help menu (L still does). A Pokémon made shiny gets its nature and IVs from one roll of the game’s random number generator, as a wild shiny does, so PKHeX finds it legal; it keeps its gender, ability, moves and held item. Roaming legendaries and Unown are never made shiny (PKHeX expects them exactly as the game makes them). It lasts until the game is closed or reset.',
        payloads: {
            ...romPayloads(decodeBase64(`/gOCABYAAAAAAM3Cw8jTAMLPyM7DyMH////////////////////////////////////N3N3i7QDK
ycUbx8nIuADh4+bZAOPa6Nni////////////////////vdXo19wA4+YA2Nna2dXoAOPi2QDKycUb
x8nI/////////////////9Xb1d3iANXi2ADV29Xd4gDo4wDh2dnoAN3o///////////////////n
3N3i7a0A0N3n3egA6NzZANjZ4N3q2ebt4dXi////////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFYAAACB+vAAAIRbsFYAAACB+8AAAICrsFYAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBVgAACCMVEAADYwC9agAACGZtaGwCvZIAAAhmbWhsAsniAOni6N3gAO3j6QDm
2efZ6Kv+zADn3OPr5wDt4+nmANfc1d3irf/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/
AAAAMLUKTQ2kCkgEOCFYKVD70QhMIGBgYAEgIHAHSAFoSxubCgLQIWFpHAFgML0A/AMCxAIAAGD/
AwLgJwAD8LWMTCB4ACgM0ItPuIvABwjRmEgBIQFwAPAL+ADwQPgA8Fz4I2kA8AP48LwBvABHGEcA
tWB4ACgx0IJJCngAKhrRAigr0YBJCWiASpFCJtF/SQloBCki0XhJUDEJeEkHHdF9S//35P/MIQkB
iEIW0nhJAyBIcBLgACFhcAYqDdABIAEqAtACIAcqCNEiieOImkIB0OKAAOChiAkYoYAAvQC1+I1A
ChjTOGh0SYhCFNF0SAB4ACgQ0XNIAHgCKAzRbUgAeAIoCNJxSKGIQYDhiIGAcEhtS//3q/8AvQC1
V00uaOBosEI+0OZgeGhkSYhCAtAMMIhCNNFRSABogYkJBECJCENpaIhCK9EoAAshU0v/947/IIFL
SQAiCnBaSQl4ASICKQDRAiJicMkoG9ABAPM5AikX2eGIACKIQgPRoogeKgDZHiICMlIBaGhwQAEM
SEAABAAMkEIF0gDwBfjmYAHgACBgcAC9kLWEsACWNgwA8Fj4AZariOmIS0AfAACYAAQ6STpKBABM
Q6QYAwx7QAEwZBgmDF5ACC750gYMfkCeQjnRJgwA8D34AZqWQu/RBgwnDD8EPkMtSiAASEOAGAQA
TEOkGEAAQAxkAGQM5AMgQwKQAJlxQCgAIDAsIoNYS0CDUAQ6+tUuYAAnAphBCQKRHyEIQACQJyHJ
GQCqKAAXTADwFvgBNwYv79EoABZMAPAP+ASwkL0XJjYEgBlOQ6QZs+cPSnJDUgy2GrYathpwRyBH
YP8DAoAjAAMoQAIC3EIAA4Y+AgJUQgADRYgBCEgrAgJ4PQICcYYECJEzBAglOwQIq6oAACUcBAht
TsZBc2AAAHHxAwL2rQMC7ZwFCG2dBQicEAADqA8AA3HSBgi8cAMCmP4DAiNRugkIaccDfQAGgIMB
BYBntP4DAmZtaGsCwEb9AgDX3NXd4vAA/QOr/8BG`), {
                'BPGE 1.10': decodeBase64(`XAEBR9kEASU=`),
                'BPRF 1.10': decodeBase64(`aAEBRjwCATB4BALQIoAEASyIBAWkQQADMZgECrGGBAjlMgQIeTqoBAJ5G7wEBkmeBQjJnswEAtXT
2QQCDb0=`),
                'BPRD 1.10': decodeBase64(`aAEBRDwCATB4BALQIoAEASyIBAKkQZgECvGFBAgNMwQIoTqoBAKhG7wEBomdBQgJnswEAhXT2QQC
Tbw=`),
                'BPRI 1.10': decodeBase64(`aAEBSTwCATB4BALQIoAEASyIBAKkQZgECt2FBAj5MgQIjTqoBAKNG7wEBXWdBQj1zAQCAdPZBAI5
vA==`),
                'BPRS 1.10': decodeBase64(`aAEBUzwCATB4BALQIoAEASyIBAWkQQADMZgECsWGBAj5MgQIjTqoBAKNG7wEBl2eBQjdnswEAunT
2QQCIb0=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY8AgEweAQC0CKABAEsiAQFpEEAAzGYBAqxhgQI5TIECHk6qAQCeRu8BAZJngUIyZ7M
BALV09kEAuG8`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ8AgEweAQC0CKABAEsiAQCpEGYBArxhQQIDTMECKE6qAQCoRu8BAaJnQUICZ7MBAIV
09kEAiG8`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk8AgEweAQC0CKABAEsiAQCpEGYBArdhQQI+TIECI06qAQCjRu8BAV1nQUI9cwEAgHT
2QQCDbw=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM8AgEweAQC0CKABAEsiAQFpEEAAzGYBArFhgQI+TIECI06qAQCjRu8BAZdngUI3Z7M
BALp09kEAvW8`),
            }),
        },
    },
    {
        id: 'custom-roamer',
        label: 'Roaming Pokémon Finder & Lure',
        description: 'Tells you which Pokémon is roaming and the route it’s on right now, then offers to lure it: while the lure is on, it follows you onto the routes it roams, and 1 in 4 wild Pokémon you meet there is it. Entei, Raikou or Suicune roams after you bring Celio the Sapphire. The lure lasts until the game is closed or reset; talk to the deliveryman again to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`EgT1ACoAAAAQAMzJu8fDyMEAysnFG8fJyP/////////////////////////////////A3eLYAN3o
uADo3NniAODp5tkA3ej/////////////////////////wN3i2ADj6egA69zZ5tkA6NzZAObj1eHd
4tv//////////////////8rJxRvHycgA3ecA1eLYAODp5tkA3egA6OMA7ePprf/////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFxQAACB+vAAAIRbsFxQAACB+8AAAICrsFxQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADrwEhDYAAALsBsQAACL3PAAAIZm0fYP8DAgG7AY0AAAi96wAACGZuFAghDYAAALsB
uwAACCMVEAADrgG9KAEACGZtaGwCvVoBAAhmbhQIIQ2AAQC7AbsAAAgRAGD/AwK9fgEACGZtaGwC
vZ0BAAhmbWhsAr2+AQAIZm1obAK90gEACGZtaGwC/QIA3ecA5uPV4d3i2/79AwDm3dvc6ADi4+ut
/83c1eDgAMMA4Onm2QDd6ADo4wDt4+msAMPoAOvd4OD+2uPg4OPrAO3j6QDV4OPi2wDd6OcA5uPp
6Nnnrf++4+LZqwDG4+PfANrj5gDd6ADd4gDo1eDgANvm1efn/tXi2ADj4gDo3NkA69Xo2eat/8Po
tOcA2uPg4OPr3eLbAO3j6a3+xdnZ5ADg6ebd4tsA3eis/8PoAOvd4OAA5uPV4QDj4gDd6OcA4+vi
ANXb1d3irf/I4wDKycUbx8nIAN3nAObj1eHd4tsA5t3b3Oj+4uPrrf+94+HZANbV198A1eLtAOjd
4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAAAQtRtMJGgbSCQY4HwaSQiAACgQ
0CGJGUgaSwDwJ/gaShB4UXgZSwDwIfgBfRRIACIXSwDwG/gQvXC1FkseiAAgGIAVTRikFUgEOCFY
KVD70RNMASAgcBNIAWhLG5sKAtBhYGkcAWALSx6AcL0YR8BG2EIAA9AwAADMcAMC0BwCAvAcAgJ5
RwQIqvMDAvGJBQjthAwICAIABAD8AwJYAAAAYP8DAuAnAAMQtRBMIHgAKBTQD0gAaA9JQVwAKQ7Q
AXkDKQvRQnkMSxh4kEID0Acz/yj50QLgCUgBcEJwY2gA8AP4ELwBvABHGEfARmD/AwLYQgAD4zAA
AGRGRgiq8wMC`), {
                'BPGE 1.10': decodeBase64(`XAEBR9QDAcE8BAJQPw==`),
                'BPRF 1.10': decodeBase64(`aAEBRrQDASjIAwLNRtADBk2LBQhJiOgDATA0BAEoPAQDANlF`),
                'BPRD 1.10': decodeBase64(`aAEBRLQDASjIAwL1RtADBo2KBQiJh+gDATA0BAEoPAQCuFM=`),
                'BPRI 1.10': decodeBase64(`aAEBSbQDASjIAwLhRtADBnmKBQhph+gDATA0BAEoPAQD4LJF`),
                'BPRS 1.10': decodeBase64(`aAEBU7QDASjIAwLhRtADBmGLBQhRiOgDATA0BAEoPAQDdOlF`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUa0AwEoyAMCzUbQAwZNiwUIHYjoAwEwNAQBKDwEAyjGRQ==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUS0AwEoyAMC9UbQAwaNigUIXYfoAwEwNAQBKDwEAiRF`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUm0AwEoyAMC4UbQAwZ5igUIPYfoAwEwNAQBKDwEA3iqRQ==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVO0AwEoyAMC4UbQAwZhiwUIJYjoAwEwNAQBKDwEA2zgRQ==`),
            }),
        },
    },
    {
        id: 'custom-legendary-respawn',
        label: 'Legendary Respawn',
        description: 'Brings back the legendary Pokémon you knocked out instead of catching, to where you met them: Articuno, Zapdos, Moltres, Mewtwo, Lugia, Ho-Oh and Deoxys. A roaming Pokémon you knocked out, or that left with Roar, roams again at full HP. Legendaries your Pokédex has as caught stay gone.',
        payloads: {
            ...romPayloads(decodeBase64(`EwSWACsAAAAcAMa/wb/IvrvM0wDMv83Ku9HI//////////////////////////////+7AOfZ1+Pi
2ADX3NXi19n/////////////////////////////////xtnb2eLY1ebtAMrJxRvHycgA7ePpANbZ
1ej//////////////////9bp6ADY3djitOgA19Xo19wA1+Ph2QDW1dffrf/////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFiwAACB+vAAAIRbsFiwAACB+8AAAICrsFiwAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR72VAAAIZm4UCCENgAAAuwGBAAAIIxUQAAP7ACENgAAAuwF3AAAIgwANgL3QAAAIZm1obAK9
9gAACGZtaGwCvR8BAAhmbWhsAr0zAQAIZm1obALN3NXg4ADDANbm3eLbANbV198A6NzZAODZ29ni
2NXm7f7KycUbx8nIAO3j6QDY3djitOgA19Xo19ys/77j4tmrAP0CAODZ29ni2NXm7QDKycUbx8nI
/tfV4dkA1tXX363/yOMA4Nnb2eLY1ebtAMrJxRvHycgA4tnZ2OcA6OP+1+Ph2QDW1dffrf+94+HZ
ANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAPC1gbAAJzGk
IIgAKBDQJUsA8Eb4ACgJ0GCIAPAz+AAoBNEgiCBLAPA7+AE3BDTr5x5NLWgfSC0YKIkAKB7Q6XwA
KRvRHEsA8Cv4APAb+AAoFNFoaACQGUgpiSp7K2gYTgDwH/gVSFgwAIhogQAgaHMBIOh0E0sA8BP4
ATcSSAeAAbDwvQAoCtABOMEICEoSaCgyUVwHIhBAwUABIAhAcEcYRzBHPR4HCBUeBwjYQgAD3EIA
A9AwAABBagQIKEACAhEXBAjRWBQIzHADAr4CkAC/ApEAvQKSALwClgD1AgAA9gIAAPcCAAAAAMBG`), {
                'BPGE 1.10': decodeBase64(`XAEBR2gDAak=`),
                'BPRF 1.10': decodeBase64(`aAEBRkgDDUUfBwgdHwcIKEIAAyxcAwKVaWQDBmUWBAjVWw==`),
                'BPRD 1.10': decodeBase64(`aAEBREgDDYUeBwhdHgcIKEIAAyxcAwK9aWQDBo0WBAgZWw==`),
                'BPRI 1.10': decodeBase64(`aAEBSUgDDXEeBwhJHgcIKEIAAyxcAwKpaWQDBnkWBAhZWw==`),
                'BPRS 1.10': decodeBase64(`aAEBU0gDDVkfBwgxHwcIKEIAAyxcAwKpaWQDBnkWBAhFXA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZIAw1FHwcIHR8HCChCAAMsXAMClWlkAwZlFgQIrVs=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURIAw2FHgcIXR4HCChCAAMsXAMCvWlkAwaNFgQI8Vo=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlIAw1xHgcISR4HCChCAAMsXAMCqWlkAwZ5FgQIMVs=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNIAw1ZHwcIMR8HCChCAAMsXAMCqWlkAwZ5FgQIHVw=`),
            }),
        },
    },
    {
        id: 'custom-instant-eggs',
        label: 'Instant Egg Hatch & Day Care Egg',
        description: 'Hatches every Egg in your party on the spot, each with the usual hatching scene and nickname prompt. With no Eggs to hatch, or if you say no, the deliveryman offers to have the Day Care’s Egg ready right away, when the two Pokémon there can have one.',
        payloads: {
            ...romPayloads(decodeBase64(`+AOcARAAAAAEAMPIzc67yM4Av8HBzf/////////////////////////////////////C1ejX3ADi
4+u4AOPmANvZ6ADj4tkA4uPr////////////////////wtXo19wA6NzZAL/Bwc0A7ePpANfV5ubt
uADj5v///////////////9vZ6ADo3NkAvrvTAL27zL+05wC/wcEA5t3b3Oj////////////////V
69XtrQDQ3efd6ADo3NkA2Nng3erZ5u3h1eL/////////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF9QAACB+vAAAIRbsF9QAACB+8AAAICrsF9QAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADCwIhDYAAALsBngAACL3/AAAIZm4UCCENgAAAuwGeAAAIaCMVEAADgAK4cQAACCMV
EAAD/gEhDYAAALsBlAAACCXCACcoEAC5dgAACL01AQAIZm1obAK9TQEACGZuFAghDYAAALsB6wAA
CCtmArsB1wAACCMVEAAD7QEhDYAAALsB4QAACL2HAQAIZm1obAK9sgEACGZtaGwCveMBAAhmbWhs
Ar0bAgAIZm1obAK9LwIACGZtaGwC0+PpANzV6tkAv8HBzQDr3ejcAO3j6av+zdzV4OAAwwDc1ejX
3ADo3NnhAObd29zoAOLj66z/ztXf2QDb4+PYANfV5tkA49oA6NzZ4av/zdzV4OAAwwDc1erZAOjc
2QC+u9MAvbvMv7Tn/r/BwQDm2dXY7QDa4+YA7ePpAObd29zoANXr1e2s/87c2QC+u9MAvbvMvwDc
1ecA1eIAv8HB/ubZ1djtANrj5gDt4+kA4uPrq//O3NkAvrvTAL27zL8A1eDm2dXY7QDc1ecA1eL+
v8HBAOvV3ejd4tsA2uPmAO3j6av/xtnV6tkA6OvjAMrJxRvHycgA6NzV6ADb2ej+1eDj4tsA1egA
6NzZAL670wC9u8y/ANrd5ufoq/+94+HZANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa
4+b+6Nzd5wDb1eHZrf8AACBIACEBOQGAHUgAIQYiw3xbB1sPBisA0QExZDABOvbRGUgBgHBHF0sa
iAEyEgQSDGQhUUMSSEAYBioK0sF8SQdJDwYpAtBkMAEy9ecagAEgAOAAIAxJCIBwRwC1C0gAaAtJ
QBgLSwDwCfgAKAPQCksA8AT4ASAESQiAAL0YR8BGgEICArxwAwLMcAMC2EIAA4AvAAD1nAQI8ZEE
CAdIAGgHSUAYB0maaBIaUhiaYPkikgAEOoNYi1D70XBH2EIAAyQ2AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRiwEASg0BAY1nQQIMZJcBAEo`),
                'BPRD 1.10': decodeBase64(`aAEBRCwEASg0BAV1nAQIcVwEASg=`),
                'BPRI 1.10': decodeBase64(`aAEBSSwEASg0BAVhnAQIXVwEASg=`),
                'BPRS 1.10': decodeBase64(`aAEBUywEASg0BAZJnQQIRZJcBAEo`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYsBAEoNAQGNZ0ECDGSXAQBKA==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQsBAEoNAQFdZwECHFcBAEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUksBAEoNAQFYZwECF1cBAEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMsBAEoNAQGSZ0ECEWSXAQBKA==`),
            }),
        },
    },
    {
        id: 'custom-trade-evolution',
        label: 'Trade Evolution Without Trading',
        description: 'Evolves a Pokémon from your party that normally evolves by trading, such as Kadabra, Machoke, Graveler or Haunter, with the usual evolution scene. One that needs to hold an item when traded (Onix, Scyther, Seadra, Slowpoke, Poliwhirl, Porygon) must be holding it, and uses it up as in a trade. Evolving into a Johto Pokémon needs the National Pokédex.',
        payloads: {
            ...romPayloads(decodeBase64(`EQRBACkAAAAIAM7Mu76/AL/QycbPzsPJyP/////////////////////////////////I4wDo5tXY
3eLbAOTV5uji2eYA4tnZ2NnY////////////////////v+rj4OrZANUAysnFG8fJyADo3NXoANnq
4+Dq2ef//////////////9btAOjm1djd4tu4AObd29zoANXr1e2tANDd593o///////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFrQAACB+vAAAIRbsFrQAACB+8AAAICrsFrQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR723AAAIZm0jFRAAA5ABuFEAAAglnwAnIQSABgC7BKoAAAgmDYBHASENgJwBuwGgAAAIfwAE
gCMVEAAD+QAhDYAAALsBlgAACCe96QAACGZtaGwCvf8AAAhmbWhsAr3UAAAIZm1obAJobAK9VAEA
CGZtaGwC0dzd19wAysnFG8fJyADn3OPp4NgA2erj4OrZrP+74gC/wcEA19XitOgA2erj4OrZq//O
1d/ZANvj49gA19Xm2QDj2gDd6Kv//QIA2OPZ5+K06ADZ6uPg6tkA1u3+6ObV2N3i2637zePh2QDK
ycUbx8nIAOLZ2dgA6OMA3OPg2ADV4v7d6NnhAOvc2eIA6NzZ7bTm2QDo5tXY2dit/87c3ecA293a
6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AMLUA8CX4BAABIQAiDEsA8BH4BQAL0AxIDEkBYAZLG3gA
IikAIAAHTADwBfgBJQNIBYAwvRhHIEe8cAMCzHADAm1mBAhRFQ0ITEYAA4mgBQgDSACIZCFIQwJJ
QBhwR8BGvHADAoBCAgIHSABoB0lAGAdJmmgSGlIYmmD5IpIABDqDWItQ+9FwR9hCAAMkNgAAAPwD
Ag==`), {
                'BPGE 1.10': decodeBase64(`XAEBRwgDASU=`),
                'BPRF 1.10': decodeBase64(`aAEBRgQDDsFlBAitGA0InEUAA+WhTAMBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRAQDDullBAjtFw0InEUAAyWhTAMBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSQQDDtVlBAjNFw0InEUAAxGhTAMBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBUwQDDtVlBAi1GA0InEUAA/mhTAMBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYEAw7BZQQIgRgNCJxFAAPloUwDASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQEAw7pZQQIwRcNCJxFAAMloUwDASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkEAw7VZQQIoRcNCJxFAAMRoUwDASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMEAw7VZQQIiRgNCJxFAAP5oUwDASg=`),
            }),
        },
    },
    {
        id: 'custom-espeon-umbreon',
        label: 'Espeon & Umbreon (Evolve Eevee)',
        description: 'Evolves a friendly Eevee from your party into Espeon or Umbreon, your choice, with the usual evolution scene. FireRed and LeafGreen have no clock, so this is the only way to get them. Eevee needs 220 friendship, as in a normal evolution, and the National Pokédex.',
        payloads: {
            ...romPayloads(decodeBase64(`JwTEAD8AAAAMAL/Nyr/JyAAtAM/HvMy/ycj///////////////////////////////++1e0A4+YA
4t3b3Oi4AOLjANfg49ffAOLZ2djZ2P//////////////uwDa5t3Z4tjg7QC/v9C/vwDZ6uPg6tnn
AN3i6OP//////////////7/Nyr/JyADj5gDPx7zMv8nIuADt4+nmAOTd19+t///////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFFAEACB+vAAAIRbsFFAEACB+8AAAICrsFFAEACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR70eAQAIZm0jFRAAA0gDuFEAAAglnwAnIQSABgC7BBEBAAgmDYBHASENgJwBuwEHAQAIfwAE
gCMVEAADmQEhDYABALsB2wAACCtACLsA5QAACCENgAIAuwHvAAAIvU4BAAhmIxUQAAObASchDYB/
ALsB/QAACGgjFRAAA48BJyYNgEcBIQ2AhQC7Af0AAAi9aQEACGZtaGwCvX8BAAhmbWhsAr2SAQAI
Zm1obAKDAQWAvbcBAAhmbWhsAr3hAQAIZm1obAK9OQEACGZtaGwCaGwCvfUBAAhmbWhsAtHc3dfc
AL+/0L+/AOfc4+ng2ADZ6uPg6tms/7viAL/BwQDX1eK06ADZ6uPg6tmr/9Hc3dfcANrj5uEA59zj
6eDYAN3oAOjV39ms/87V39kA2+Pj2ADX1ebZAOPaAN3oq//9AgDd5+K06ADV4gC/v9C/v6v/w+gA
4tnZ2OcA6NzZAMi7zsPJyLvGAMrJxRu+v9L+2t3m5+it//0CtOcA2ubd2eLY59zd5ADd5/79A60A
w+gA2erj4OrZ5wDV6ACjo6Gt/73j4dkA1tXX3wDV4u0A6N3h2av/ztzd5wDb3droAN3n4rToANrj
5v7o3N3nANvV4dmt/zC1APCx+AQAICEA8CT4FElIgAIl3CgA0wAlIAALIQDwGviFKADQASUOSAWA
ML0SoAIhCCJv4BC1DUgOSQFgAPCT+AhJCYjEMQEiBUsbeAdMAPAE+BC9ACIDSxhHIEfARrxwAwLM
cAMCkTMECFEVDQhMRgADiaAFCL/Nyr/JyP8Az8e8zL/JyP/wtYWwDQAXACtOACMCyAAiBsYBM6tC
+dEnThwgwBsAIToALaNsHhtdJUwA8C/4BwAAISRMAPAq+A4hAJEBlQKWACEDkQIhBJE4AAgiAiMe
TADwHPgOIQCRAZUAIQKROAACIQAiAiMZTADwEPgAICkAOgACIxZMAPAJ+AAgFUwA8AX4Dkj/IQGA
BbDwvSBHELWIsAAjnABsRCBgBHgBMP8s+9EDMIAIgAABM4tC8tFoRv/3pv8IsBC9sPsDAsxwAwK5
DQoIya4PCHk0EQhpMBEIfQMKCB2fDwgCBAYHCQsNDgNIAIhkIUhDAklAGHBHwEa8cAMCgEICAgdI
AGgHSUAYB0maaBIaUhiaYPkikgAEOoNYi1D70XBH2EIAAyQ2AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBR9ADASWsBBaNDQoIoa4PCFE0EQhBMBEIUQMKCPWe`),
                'BPRF 1.10': decodeBase64(`aAEBRswDDuUyBAitGA0InEUAA+WhrAQWQRAKCLWyDwjBNhEIsTIRCAUGCggJowQFASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRMwDDg0zBAjtFw0InEUAAyWhrAQWgQ8KCPWxDwgBNhEI8TERCEUFCghJogQFASg=`),
                'BPRI 1.10': decodeBase64(`aAEBScwDDvkyBAjNFw0InEUAAxGhrAQWbQ8KCPWxDwhFNhEINTIRCDEFCghJogQFASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU8wDDvkyBAi1GA0InEUAA/mhrAQWVRAKCN2yDwgtNxEIHTMRCBkGCggxowQFASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbMAw7lMgQIgRgNCJxFAAPloawEFhUQCgiNsg8ImTYRCIkyEQjZBQoI4aIEBQEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTMAw4NMwQIwRcNCJxFAAMloawEFlUPCgjNsQ8I2TURCMkxEQgZBQoIIaIEBQEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnMAw75MgQIoRcNCJxFAAMRoawEFkEPCgjNsQ8IHTYRCA0yEQgFBQoIIaIEBQEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPMAw75MgQIiRgNCJxFAAP5oawEFikQCgi1sg8IBTcRCPUyEQjtBQoICaMEBQEo`),
            }),
        },
    },
    {
        id: 'custom-move-tutor',
        label: 'Move Relearner, Deleter & Tutor Reset',
        description: 'Teaches a Pokémon from your party a move it could have learned by level up, like the Move Relearner but free, or makes it forget any move, HMs included. It can also let every one-time move tutor teach its move again.',
        payloads: {
            ...romPayloads(decodeBase64(`EATrACgAAAAUAMfJ0L8AzL/Gv7vMyL/MAC0Avr/Gv86/zP////////////////////+74tgA6NzZ
AOjp6OPm5wDo2dXX3ADV29Xd4v//////////////////zNng2dXm4gDVAOHj6tm4ANrj5tvZ6ADj
4tm4/////////////////+PmAODZ6ADo3NkA4ePq2QDo6ejj5ucA6NnV19z////////////////V
29Xd4q0A0N3n3egA6NzZANjZ4N3q2ebt4dXi////////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFnQEACB+vAAAIRbsFnQEACB+8AAAICrsFnQEACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAAD+wK4SgAACL2nAQAIZm4UCCENgAAAuwGZAAAIvfIBAAhmbSXbACchBIAGALsElgAA
CCVIASENgAEAuwEhAQAIIQWAAAC7ASsBAAgl4AAnaGwCvc8BAAhmbhQIIQ2AAAC7AT8BAAi98gEA
CGZtJZ8AJyEEgAYAuwSWAAAIJUgBIQ2AAQC7ASEBAAh/AASAJd8AIQ2AAQC7ATUBAAi9DgIACGZt
lwEl3AAnlwAhBYAEALsBlgAACCXeAL0rAgAIZm4UCCENgAAAuwGTAQAIJd0AvT4CAAhmbWhsAr1M
AgAIZm1obAK9bwIACGZtaGwCvZMCAAhmbWhsAr2rAgAIZm4UCCENgAAAuwGTAQAIKsACKsECKsIC
KsMCKsQCKsUCKsYCKscCKsgCKskCKsoCKssCKswCKs0CKs4CKt4CKt8CKuACveMCAAhmbWhsAr0M
AwAIZm1obAK9IAMACGZtaGwCzdzV4OAAwwDc2eDkANUAysnFG8fJyP7m2eHZ4dbZ5gDVAOHj6tms
/8nmAOfc1eDgAMMA4dXf2QDj4tkA2uPm29no/tUA4ePq2az/0dzd19wAysnFG8fJyADn3OPp4NgA
3egA1tms/9Hc3dfcAOHj6tkA59zj6eDYAN3oANrj5tvZ6Kz/x9Xf2QD9AgDa4+bb2ej+/QOs//0C
ANrj5tvj6AD9A6v/u+IAv8HBANjj2efitOgA3+Lj6wDV4u3+4ePq2ecA7dnoq//O3Nnm2bTnAOLj
AOHj6tkA2uPmAN3oAOjj/ubZ4dnh1tnmrf/9AgDf4uPr5wDj4uDtAOPi2f7h4+rZq//J5gDn3NXg
4ADDAODZ6ADo3NkA4ePq2QDo6ejj5uf+6NnV19wA6NzZ3eYA4ePq2ecA1dvV3eKs/77j4tmrAL/q
2ebtAOHj6tkA6Ono4+YA693g4P7o2dXX3ADV29Xd4q3/vePh2QDW1dffANXi7QDo3eHZq//O3N3n
ANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/AAdIAGgHSUAYB0maaBIaUhiaYPkikgAEOoNYi1D7
0XBH2EIAAyQ2AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRrAEASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLAEASg=`),
                'BPRI 1.10': decodeBase64(`aAEBSbAEASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU7AEASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUawBAEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSwBAEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmwBAEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOwBAEo`),
            }),
        },
    },
    {
        id: 'custom-reusable-tms',
        label: 'Reusable TMs',
        description: 'Teaching a move with a TM no longer uses the TM up, as in later games, so one TM can teach as many Pokémon as you like. HMs work as before, and selling a TM at a shop still sells it. It lasts until the game is closed or reset; talk to the deliveryman again after a reset, or to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`FQSJAC0AAAAYAMy/z827vMa/AM7H5//////////////////////////////////////O2dXX3ADV
AM7HANXb1d3iANXi2ADV29Xd4v//////////////////ztnV19zd4tsA1QDh4+rZAOvd6NwA1QDO
xwDi4////////////////+Dj4tvZ5gDp59nnAOjc2QDOxwDp5K0A0N3n3ej////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFowAACB+vAAAIRbsFowAACB+8AAAICrsFowAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2tAAAIZm4UCCENgAAAuwGZAAAIIxUQAAMbAb3RAAAIZm1obAK9DAEA
CGZuFAghDYABALsBmQAACCMVEAADLAG9NQEACGZtaGwCvUwBAAhmbWhsAr1gAQAIZm1obALN3NXg
4ADDAOHV39kA7ePp5gDOx+cA4NXn6P7a4+bZ6tnmrP++4+LZqwDO2dXX3N3i2wDVAOHj6tkA6+Pi
tOgA6efZ/unkAOjc2QDOxwDp4ujd4ADt4+kA5tnn2eit/9Pj6eYAzsfnAODV5+gA2uPm2erZ5q3+
xdnZ5ADd6ADo3NXoAOvV7az/zsfnANvZ6ADp59nYAOnkANXb1d3irf+94+HZANbV198A1eLtAOjd
4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AcLUPSx6IACAYgA5NEaQOSAQ4IVgp
UPvRDExgcAEgIHALSAFoSxubCgLQYWBpHAFgA0segHC9BUgAIQFwcEfARggCAAQA/AMCtAAAAGD/
AwLgJwAD8LUgTCB4ACg00B9PuIvABzDReGgdSYhCBtAjSYhCEtAiSYhCD9AL4BlNECYZSShoiEIC
0Sh5ACgK0Sg1AT720QAgYHAW4BpIAGgAKBLQA+ARSACKACgN0WB4ACgK0Q9IAIgPSUEaMikE0gEh
YXANSwDwBvhjaADwA/jwvAG8AEcYR8BGYP8DAoAjAAOBJhIIYEMAA9WXEgicsAMCLK0DAiEBAADp
1wkIKYkSCN2JEgiMsAMC`), {
                'BPGE 1.10': decodeBase64(`XAEBR7ADAVm4AwGtyAMJvdcJCAGJEgi1`),
                'BPRF 1.10': decodeBase64(`aAEBRiADATCsAw7QIgADDSkSCLBCAAN5msgDCnHaCQjNixIIgYw=`),
                'BPRD 1.10': decodeBase64(`aAEBRCADATCsAw7QIgADTSgSCLBCAAO9mcgDCrHZCQgRixIIxYs=`),
                'BPRI 1.10': decodeBase64(`aAEBSSADATCsAw7QIgADkSgSCLBCAAP9mcgDCp3ZCQhRixIIBYw=`),
                'BPRS 1.10': decodeBase64(`aAEBUyADATCsAw7QIgADeSkSCLBCAAPpmsgDCoXaCQg9jBII8Yw=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYgAwEwrAMO0CIAA+UoEgiwQgADUZrIAwpF2gkIpYsSCFmM`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQgAwEwrAMO0CIAAyUoEgiwQgADlZnIAwqF2QkI6YoSCJ2L`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkgAwEwrAMO0CIAA2koEgiwQgAD1ZnIAwpx2QkIKYsSCN2L`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMgAwEwrAMO0CIAA1EpEgiwQgADwZrIAwpZ2gkIFYwSCMmM`),
            }),
        },
    },
    {
        id: 'custom-physical-special-split',
        label: 'Gen 4 Physical/Special Split',
        description: 'Every damaging move is physical or special on its own, as from Diamond and Pearl on, instead of by its type: Fire Punch, Crunch and Dragon Claw hit with Attack, Shadow Ball, Hyper Beam and Sludge Bomb with Sp. Atk, and Hidden Power and Weather Ball are always special. Stat stages, Reflect and Light Screen, burns, Choice Band, Huge Power, Pure Power, Hustle’s boost, Guts, Counter and Mirror Coat follow the move’s kind too. In a link battle one of the two consoles runs the battle for both players, so use the card on both. It lasts until the game is closed or reset; talk to the deliveryman again after a reset, or to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`JgRlAT4AAAAcAMG/yAClAMrC083DvbvGus3Kv73Du8YAzcrGw87////////////////H4+rZ5wDc
3egA1ecA3eIA4NXo2eYA29Xh2ef/////////////////v9XX3ADh4+rZAN3nAOTc7efd19XgAOPm
/////////////////////+fk2dfd1eAA4+IA3ejnAOPr4rgA4uPoANbtAN3o5//////////////o
7eTZrQDQ3efd6ADo3NkA2Nng3erZ5u3h1eL/////////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFogAACB+vAAAIRbsFogAACB+8AAAICrsFogAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2sAAAIZm4UCCENgAAAuwGYAAAIIxUQAAPvAL3NAAAIZm1obAK97QAA
CGZuFAghDYABALsBmAAACBEAYP8DAr0KAQAIZm1obAK9HwEACGZtaGwCvTMBAAhmbWhsAtHV4ugA
6NzZAOTc7efd19Xguufk2dfd1eD+5+Tg3eis/77j4tmrAMPoAODV5+jnAOni6N3gAO3j6QDm2efZ
6K3/ztzZAOfk4N3oAN3nAOPirf7F2dnkAN3oAOPirP+81dffAOjjAOjc2QDj4NgA69Xtq/+94+HZ
ANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AADC1Ck0MpI8g
gAAEOCFYKVD70QdMASAgcAZIAWhLG5sKAtBhY2kcAWAwvcBGAPwDAmD/AwLgJwAD8LV0THROsIvA
B0fRYXgAKQHQAPCX+CB4ACg/0DBob0mIQjvRbkgAaAAoN9FtSAdoeA4EKDLRbEgAiHShwgiJXAci
AkDRQAElDUAMIUFDZ0hAGIZ4ZkgAaMF8OngAKQXQDCoB0UsGAdQ/Jg5ACT72D65CE9AFKhbQwyoU
0IYqEtAMKgvReXhdSAEpB9gA0VpIAHigcADwkfgCIWFwY2vwvAG8hkYYR1RIAPBn+AYAIQA4MQDw
aPhRSADwX/gHACEdAPBj+ADwZ/gALSPQcIjxjbopAdFBCEAYICFxXCUpAdBKKQDRQAA3KQHRQgiA
GPJsPikE0QAqBdBCCIAYAuDSBgDVQAgwgXB+MHe4iHiBuH54dxXgMIlwgDB/cHZ4ibiAeH+4dgAg
8Y26KQDR8IUgIrFcRCkA0LBU8WwQIpFD8WQBIarnALUAIGBwAikR0ADwIvgqSADwEfgBACAdAPAV
+CVIAPAK+AEAIAA4MADwC/gAvaB4APAh+AC9AHhYIUhDHElAGHBHUCIA4DAiBDqDWItQ+9FwRxlI
AHgZSQhcwAeADxhJQBgBiEoISkDSB9IPUwAaQ1FAAYBwRxAhQUMSSAkYSmiLaEtgimAKe0t7C3NK
c3BHYP8DAoAjAAOBWwEIxDsCAnA9AgJGPQIC0OckCOQ/AgLgOwICZz0CAmg9AgLSOwIC2j0CAog+
AgL/3/7///8FAH8sAh/Mv/CnnfvV7/2f2Y/u+////Vc0WoFP1o99/mawovnbUQAAwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR8gEAaw=`),
                'BPRF 1.10': decodeBase64(`aAEBRtgCATC0BAXQIgADbcgEAnyS`),
                'BPRD 1.10': decodeBase64(`aAEBRNgCATC0BALQIsgEAlDt`),
                'BPRI 1.10': decodeBase64(`aAEBSdgCATC0BALQIsgEAgx/`),
                'BPRS 1.10': decodeBase64(`aAEBU9gCATC0BAXQIgADbcgEAvSl`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbYAgEwtAQF0CIAA23IBAJYkg==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTYAgEwtAQC0CLIBAIs7Q==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnYAgEwtAQC0CLIBALofg==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPYAgEwtAQF0CIAA23JBAGl`),
            }),
        },
    },
    {
        id: 'custom-exp-share',
        label: 'Exp. Share for the Whole Party',
        description: 'Every Pokémon in your party gets experience from each battle, as with the Exp. Share of later games: the ones that battled get all of it instead of a share, and the rest get half. Eggs and fainted Pokémon get none. Trainer battles, traded Pokémon and the Lucky Egg still give their boosts, and the whole party gains EVs. It lasts until the game is closed or reset; talk to the deliveryman again to turn it off.',
        payloads: {
            ...romPayloads(decodeBase64(`KwTyAEMAAAAQAL/Syq0AzcK7zL8AwMnMALvGxv/////////////////////////////O3NkA69zj
4NkA5NXm6O0A2+bj6+f/////////////////////////v+rZ5u0AysnFG8fJyADd4gDt4+nmAOTV
5ujt/////////////////9vZ6OcAv9LKrQDa5uPhANnV19wA1tXo6ODZrf/////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFogAACB+vAAAIRbsFogAACB+8AAAICrsFogAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARx9g/wMCAbsBdAAACL2sAAAIZm4UCCENgAAAuwGYAAAIIxUQAANHAb3fAAAIZm1obAK9NgEA
CGZuFAghDYABALsBmAAACBEAYP8DAr1jAQAIZm1obAK9eAEACGZtaGwCvYwBAAhmbWhsAs3c1eDg
AO3j6eYA69zj4NkA5NXm6O0A29noAL/Syq3+2ubj4QDZ6tnm7QDW1ejo4Nms/77j4tmrAM7c4+fZ
AOjc1egA1tXo6ODZANvZ6ADV4OD+6NzZAL/Syq24AOjc2QDm2efoANvZ6ADc1eDarfvD6ADg1efo
5wDp4ujd4ADt4+kA5tnn2eit/9Pj6eYA69zj4NkA5NXm6O0A29no5wC/0sqt/sXZ2eQA3egA6NzV
6ADr1e2s/7zV198A6OMA6NzZAOPg2ADr1e2r/73j4dkA1tXX3wDV4u0A6N3h2av/ztzd5wDb3dro
AN3n4rToANrj5v7o3N3nANvV4dmt/wAwtQpNDKRBIIAABDghWClQ+9EHTAEgIHAGSAFoSxubCgLQ
YWBpHAFgML3ARgD8AwJg/wMC4CcAA/C1M0wzSIGLyQdV0SF4AClS0ABoMEmIQk7RMEgAaAAoStEv
SABoQQ4EKUXRAHgjKELRLE0taCxPOH8CKB3QASg60SpIAHhYIUhDKUlAGAGIKiKGXBwgSEMmSUAY
QHpwQwchBt9ggAAgI0kIgCh0APAo+FMhaFQCIDh3LnxhiADwIPjwQMAHEtRkIHBDHEqAGMJ8UgdS
DwIqDtFWIoJaACoK0FMj6lwBIAJD6lRJCAApANEBIVAiqVJjaPC8AbyGRhhHCkgAeIAHwA8MShBc
cEdg/wMCgCMAA4FbAQjEOwICcD0CAuQ/AgLAPwICaT0CAuA7AgJQIyUITj8CAko/AgKAQgIC`), {
                'BPGE 1.10': decodeBase64(`XAEBRygEASw=`),
                'BPRF 1.10': decodeBase64(`aAEBRjADATAIBAXQIgADbSgEA/zNJA==`),
                'BPRD 1.10': decodeBase64(`aAEBRDADATAIBALQIigEAtAo`),
                'BPRI 1.10': decodeBase64(`aAEBSTADATAIBALQIigEA4y6JA==`),
                'BPRS 1.10': decodeBase64(`aAEBUzADATAIBAXQIgADbSgEA3ThJA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYwAwEwCAQF0CIAA20oBAPYzSQ=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQwAwEwCAQC0CIoBAKsKA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkwAwEwCAQC0CIoBANouiQ=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMwAwEwCAQF0CIAA20pBALhJA==`),
            }),
        },
    },
    {
        id: 'custom-pp-max',
        label: 'Max PP for All Moves',
        description: 'Gives every move of every Pokémon in your party the most PP it can have, as if it had three PP Ups, and fills it up.',
        payloads: {
            ...romPayloads(decodeBase64(`AwQkABsAAAAcAMrKAMe70v/////////////////////////////////////////////H4+rZ5wDV
6ADa6eDgAOTj69nm////////////////////////////v+rZ5u0A4ePq2QDd4gDt4+nmAOTV5ujt
ANvZ6Of//////////////+jc2QDh4+foAMrKAN3oANfV4gDc1erZrQDQ3efd6P/////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFcgAACB+vAAAIRbsFcgAACB+8AAAICrsFcgAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR718AAAIZm4UCCENgAAAuwFoAAAIIxUQAAPDAL25AAAIZm1obAK95wAACGZtaGwCvfsAAAhm
bWhsAs3c1eDgAMMA5tXd59kA6NzZAMrKAOPaANnq2ebt/uHj6tkA3eIA7ePp5gDk1ebo7QDo4wDo
3NkA4dXsrP++4+LZqwC/6tnm7QDh4+rZANzV5wDo3NkA4ePn6P7KygDd6ADX1eIA3NXq2a3/vePh
2QDW1dffANXi7QDo3eHZq//O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/AADwtYGwEkwG
JeB8QAdADwIoGNEDJgAnvwAgAA0hiRkNSwDwFfgAKADQAzcBPvPVAJcgABUhakYISwDwCfggAAdL
APAF+GQ0AT3e0QGw8L0YR4BCAgKRMwQIJTsECIF6BAg=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRsACCuUyBAh5OgQI1Xk=`),
                'BPRD 1.10': decodeBase64(`aAEBRMACCg0zBAihOgQI/Xk=`),
                'BPRI 1.10': decodeBase64(`aAEBScACCvkyBAiNOgQI6Xk=`),
                'BPRS 1.10': decodeBase64(`aAEBU8ACCvkyBAiNOgQI6Xk=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbAAgrlMgQIeToECNV5`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTAAgoNMwQIoToECP15`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnAAgr5MgQIjToECOl5`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPAAgr5MgQIjToECOl5`),
            }),
        },
    },
    {
        id: 'custom-stat-judge',
        label: 'IV/EV Stat Judge',
        description: 'Choose a Pokémon from your party and the deliveryman tells you its nature, its six IVs (0 to 31) and its six EVs with their total (at most 510). It works on Eggs too.',
        payloads: {
            ...romPayloads(decodeBase64(`+wOyABMAAAAYAMPQur/QAM3Ou84AxM++wb/////////////////////////////////D0Oe4AL/Q
5wDV4tgA4tXo6ebZ////////////////////////////venm3ePp5wDV1uPp6ADt4+nmAMrJxRvH
ycis/////////////////87c2QDY2eDd6tnm7eHV4gDj4gDo3NkAo+LY///////////////////a
4OPj5gDj2gDVAMrJxRvHycgAvb/Izr/MANfV4v//////////////59zj6wDd6OcAw9DnuAC/0OcA
1eLYAOLV6Onm2a3//////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFdgAACB+vAAAIRbsFdgAACB+8AAAICrsFdgAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR72AAAAIZm0jFRAAAzQCuFEAAAglnwAnIQSABgC7BHQAAAgjFRAAA1kAZwDAAQJmbWhsAr2e
AAAIZm1obALR3N3X3ADKycUbx8nIAOfc4+ng2ADDAN7p2NvZrP/O3N3nANvd2ugA3efitOgA2uPm
/ujc3ecA29Xh2a3/AAAA8LWIsEJIAIhkIUhDP0wkGAAlACcnIQYtANMUIUkZIAA9SwDwcfhpAGpG
UFIGLQDTPxgBNQwt7dFqRheDACAHkDqlNE4AJyh4ATX9KBDQ/ygG0QeZACkD0A0AACcHl/LnMHAB
Nv8o7tEIsPC8AbwARyh4ATUwKCDSECgW0gAoCtEgAAIhMgAkSwDwPvgweP8o2dABNvrnIAAgSwDw
NfiAAB9JCVgA8A/4zecQOEAAwBlpRghaAPAP+MXnMDgMJ0dDB5Uvpb/nCHj/KAPQMHABMQE2+Odw
RzC1ACQSpSmIAjUBKQvQACKIQgLTQBoBMvrnFEPz0KEyMnABNu/noTAwcAE2MLwBvABHGEfARoBC
AgK8cAMCAMABApEzBAhFZgQIbBhGCOgDZAAKAAEA/QC05wDi1ejp5tkA3ecA/QGt/sLZ5tkA1ebZ
AN3o5wDD0Oe4AOPp6ADj2gCkovD7/TD7w+jnAL/Q5wDV2NgA6eQA6OMA/RwA49oApqKh8Pv9Mf8A
wsoA/RC4ALvOzru9xQD9EbgAvr/Av8jNvwD9Ev7Nyq0Au87FAP0UuADNyq0Avr/AAP0VuADNyr+/
vgD9E//ARgdIAGgHSUAYB0maaBIaUhiaYPkikgAEOoNYi1D70XBH2EIAAyQ2AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRzADApwS`),
                'BPRF 1.10': decodeBase64(`aAEBRigDC+UyBAiZZQQIlLZF8AMBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRCgDCg0zBAjBZQQITDHwAwEo`),
                'BPRI 1.10': decodeBase64(`aAEBSSgDC/kyBAitZQQIdJBF8AMBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBUygDC/kyBAitZQQICMdF8AMBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYoAwvlMgQImWUECFSkRfADASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQoAwoNMwQIwWUECFAj8AMBKA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkoAwv5MgQIrWUECKSIRfADASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMoAwv5MgQIrWUECJi+RfADASg=`),
            }),
        },
    },
    {
        id: 'custom-hidden-power',
        label: 'Hidden Power Checker & Max IVs',
        description: 'Choose a Pokémon from your party and the deliveryman tells you the type and power of its Hidden Power, then offers to raise all six of its IVs to 31, the maximum, which makes its Hidden Power Dark-type at power 70. Its stats update right away. Only two personalities go with six 31s in these games, so to stay legal in PKHeX it takes the one that keeps its ability: its nature becomes Modest and its gender may change. A shiny Pokémon keeps its personality and shininess instead (PKHeX won’t pass it).',
        payloads: {
            ...romPayloads(decodeBase64(`BQTJAB0AAAAIAMLDvr6/yADKydG/zAAtAMPQzf////////////////////////////+93NnX3wDd
6LgA6NzZ4gDh1ewA3ej/////////////////////////zdnZANUAysnFG8fJyLTnAMLDvr6/yADK
ydG/zLj//////////////+jc2eIA5tXd59kA1eDgAN3o5wDD0OcA6OMApKL////////////////d
2gDt4+kA4N3f2a0A0N3n3egA6NzZAKPA////////////////////2Nng3erZ5u3h1eIA49oA1QC9
v8jOv8yt/////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF0gAACB+vAAAIRbsF0gAACB+8AAAICrsF0gAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73cAAAIZm0jFRAAAwQDuFEAAAglnwAnIQSABgC7BM8AAAgmDYBHASENgJwBuwHFAAAIfwAE
gCMVEAADdQGDAgWAvR4BAAhmbb1GAQAIZm4UCCENgAAAuwG7AAAIIxUQAAOjASMVEAADSAGDAgWA
vX4BAAhmbWhsAr29AQAIZm1obAK9/wAACGZtaGwCaGwCvdEBAAhmbWhsAtHc4+fZAMLDvr6/yADK
ydG/zADn3OPp4NgAw/7X3NnX36z/u+IAv8HBAN/Z2eTnAN3o5wDk4+vZ5gDc3djY2eKr//0CtOcA
wsO+vr/IAMrJ0b/MAN3n/v0Drujt5Nm4AOTj69nmAP0Erf/N3NXg4ADDAObV3efZANXg4ADd6OcA
w9DnAOjj/qSirADD6OcA4tXo6ebZAOHV7QDX3NXi29mt/7vg4ADd6OcAw9DnANXm2QCkogDi4+ur
+8Po5wDCw76+v8gAysnRv8wA3ef+/QOu6O3k2bgA5OPr2eYA/QSt/73j4dkA1tXX3wDV4u0A6N3h
2av/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt//C1APCh+AQAACUAJgAnIAAnIckZMksA
8Fb4QQgBIhBAEUC4QLlABUMOQwE3Bi/u0SggcEM/IQbfHjAjSUiADyBoQz8hBt8BMAkoANMBMADw
NPjwvRC1APB3+AQAIWhgaEhAAgxQQAAEAAwIKAbTSQgcSQDSHEkgAADwOPj/IADwAfgQvXC1gbAG
AADwXfgEACclASAwQHYIHjAAkCAAKQBqRgpLAPAP+AE1LS3x0SAAB0sA8Aj4AbBwvQchQUMFSAkY
BUgFSxhHvHADAiU7BAglHAQIbM0kCPAcAgKtyAAIkTMECKkRUGhyb0L58LWUsAUADwAuaCgAIDBp
Riwig1iLUAQ6+9UDJGhGIDgxACIADEsA8BX4DJAoADkAIgAJSwDwDvgMmQgii1hzQHtAg1AEOvnV
ATzn1S9gASAUsPC9GEfARukwBAgDSACIZCFIQwJJQBhwR8BGvHADAoBCAgIHSABoB0lAGAdJmmgS
GlIYmmD5IpIABDqDWItQ+9FwR9hCAAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRxQEAUg=`),
                'BPRF 1.10': decodeBase64(`aAEBRgwECnk6BAh5GwQI1HccBAaVyAAI5TKEBAE9wAQBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRAwECqE6BAihGwQImNIcBAWpyAAIDYQEAWXABAEo`),
                'BPRI 1.10': decodeBase64(`aAEBSQwECo06BAiNGwQIbGQcBAapyAAI+TKEBAFRwAQBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBUwwECo06BAiNGwQINIscBAaVyAAI+TKEBAFRwAQBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYMBAp5OgQIeRsECLB3HAQGlcgACOUyhAQBPcAEASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQMBAqhOgQIoRsECHTSHAQFqcgACA2EBAFlwAQBKA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkMBAqNOgQIjRsECEhkHAQGqcgACPkyhAQBUcAEASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMMBAqNOgQIjRsECBCLHAQGlcgACPkyhAQBUcAEASg=`),
            }),
        },
    },
    {
        id: 'custom-hidden-power-type',
        label: 'Hidden Power Type Changer',
        description: 'Choose a Pokémon from your party, then physical or special and one of the eight types of that kind: its Hidden Power becomes that type at power 70, the most there is. Its IVs become 30 or 31 each, with HP and Attack at 31, and its stats update right away. PKHeX won’t pass the result: its IVs no longer go with its personality.',
        payloads: {
            ...romPayloads(decodeBase64(`IQTJADkAAAAYAMLDvr6/yADKydG/zADO08q///////////////////////////////+74u0A6O3k
2bgA5OPr2eYAqKH/////////////////////////////wd3q2QDVAMrJxRvHyci05wDCw76+v8gA
ysnRv8z//////////////+jc2QDo7eTZAO3j6QDX3OPj59mtANDd593o///////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF2AAACB+vAAAIRbsF2AAACB+8AAAICrsF2AAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73iAAAIZm0jFRAAAzQDuFEAAAglnwAnIQSABgC7BNUAAAgmDYBHASENgJwBuwHLAAAIfwAE
gL0lAQAIZiMVEAADLwEnIQ2AfwC7AcEAAAgZBoANgL1DAQAIZiMVEAADGQEnIQ2AfwC7AcEAAAgj
FRAAAz4BvU8BAAhmbWhsAr19AQAIZm1obAK9BgEACGZtaGwCaGwCvZEBAAhmbWhsAtHc4+fZAMLD
vr6/yADKydG/zADn3OPp4NgAw/7X3NXi29ms/7viAL/BwQDf2dnk5wDd6OcA5OPr2eYA3N3Y2Nni
q/+7AOTc7efd19XgAOPmANUA5+TZ193V4ADo7eTZrP/R3N3X3ADo7eTZrP++4+LZqwD9ArTnAMLD
vr6/yADKydG/zP7d5wD9A67o7eTZuADk4+vZ5gCooa3/vePh2QDW1dffANXi7QDo3eHZq//O3N3n
ANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/MKACIQgiruAAtYiwAPAS+AchSEMnSUAYACFqRlBQ
BzAEMSAp+dFoRgghByIA8FT4CLAAvRxIgIgJIUhDATBwRxC1//f3/x5JDIgAGQDwJPgVSICIwAAA
GYAAAzAA8AH4EL1wtYGwBgAA8Kb4BAAnJQEgMEB2CB4wAJAgACkAakYKSwDwD/gBNS0t8dEgAAhL
APAI+AGwcL0HIUFDBUgJGAVIBksYR8BGvHADAiU7BAglHAQIbM0kCPAcAgKtyAAIzHADAsrC083D
vbvG/wDARs3Kv73Du8b/8LWFsA0AFwArTgAjAsgAIgbGATOrQvnRJ04cIMAbACE6AC2jbB4bXSVM
APAv+AcAACEkTADwKvgOIQCRAZUClgAhA5ECIQSROAAIIgIjHkwA8Bz4DiEAkQGVACECkTgAAiEA
IgIjGUwA8BD4ACApADoAAiMWTADwCfgAIBVMAPAF+A5I/yEBgAWw8L0gRxC1iLAAI5wAbEQgYAR4
ATD/LPvRAzCACIAAATOLQvLRaEb/96b/CLAQvbD7AwLMcAMCuQ0KCMmuDwh5NBEIaTARCH0DCggd
nw8IAgQGBwkLDQ4DSACIZCFIQwJJQBhwR8BGvHADAoBCAgIHSABoB0lAGAdJmmgSGlIYmmD5IpIA
BDqDWItQ+9FwR9hCAAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBR7QDAUiYBBaNDQoIoa4PCFE0EQhBMBEIUQMKCPWe`),
                'BPRF 1.10': decodeBase64(`aAEBRqwDCnk6BAh5GwQI1He8AwGVmAQWQRAKCLWyDwjBNhEIsTIRCAUGCggJo/AEASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRKwDCqE6BAihGwQImNK8AwGpmAQWgQ8KCPWxDwgBNhEI8TERCEUFCghJovAEASg=`),
                'BPRI 1.10': decodeBase64(`aAEBSawDCo06BAiNGwQIbGS8AwGpmAQWbQ8KCPWxDwhFNhEINTIRCDEFCghJovAEASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU6wDCo06BAiNGwQINIu8AwGVmAQWVRAKCN2yDwgtNxEIHTMRCBkGCggxo/AEASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUasAwp5OgQIeRsECLB3vAMBlZgEFhUQCgiNsg8ImTYRCIkyEQjZBQoI4aLwBAEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSsAwqhOgQIoRsECHTSvAMBqZgEFlUPCgjNsQ8I2TURCMkxEQgZBQoIIaLwBAEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmsAwqNOgQIjRsECEhkvAMBqZgEFkEPCgjNsQ8IHTYRCA0yEQgFBQoIIaLwBAEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOsAwqNOgQIjRsECBCLvAMBlZgEFikQCgi1sg8IBTcRCPUyEQjtBQoICaPwBAEo`),
            }),
        },
    },
    {
        id: 'custom-ev-training',
        label: 'EV Reset & Training',
        description: 'Choose a Pokémon from your party, reset its EVs to 0 if you like, then pick the stats to max out, 252 each and 510 in total. Its stats update when you’re done.',
        payloads: {
            ...romPayloads(decodeBase64(`BgRqAB4AAAAAAL/QAM7Mu8PIw8jB///////////////////////////////////////O5tXd4gDr
3ejc4+noANbV6Ojg3eLb////////////////////////zNnn2egA1QDKycUbx8nItOcAv9DnAOPm
AOHV7P///////////////+jc2QDn6NXo5wDt4+kA19zj4+fZrQDQ3efd6P/////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF4gAACB+vAAAIRbsF4gAACB+8AAAICrsF4gAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73sAAAIZm0jFRAAAxQDuFEAAAglnwAnIQSABgC7BN8AAAgmDYBHASENgJwBuwHVAAAIfwAE
gL0iAQAIZm4UCCENgAAAuwGUAAAIIxUQAAMtAb1EAQAIZiMVEAADRgEnIQ2AfwC7AcQAAAgjFRAA
AzsBgwIFgL12AQAIZm25lAAACCMVEAADlAG9ggEACGZtaGwCvQoBAAhmbWhsAmhsAr2bAQAIZm1o
bALR3N3X3ADKycUbx8nIAOfc4+ng2ADDAOjm1d3irP+74gC/wcEA19XitOgA6ObV3eIA7dnoq//M
2efZ6ADV4OAA49oA/QK05/6/0OcA6OMAoQDa3ebn6Kz/0dzd19wAv9DnAOfc4+ng2ADDAOHV7Kz+
yubZ5+cAvADr3NniAO3j6bTm2QDY4+LZrf/9AwC/0OfwAP0Erf+74OAA2OPi2asAwePj2ADg6dff
uP79Aqv/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt/wAAMLWBsADwwvgEAAAgAJAaJSAA
KQBqRipLAPBI+AE1IC320QGwML0kSAYhCCJS4PC1gbAA8Kv4BAAeSAeIACUAJr5CBtAgABohiRkc
SwDwLvgtGAE2Bi7z0R1OdhsA1QAm/C4A2fwmIAAaIckZE0sA8B34hkIA0gYAAJYgABohyRlqRg9L
APAS+AlIRoC/AApJyVkNSA1LAPAJ+AGw8L0AtQDwdPgHSwDwAfgAvRhHwEa8cAMCzHADApSwPwiR
MwQIJTsECCUcBAjwHAICrcgACP4BAADwtYWwDQAXACFOACMCyAAiBsYBM6tC+dEdThwgwBsAIToA
I6NsHhtdG0wA8C/4BwAAIRpMAPAq+A4hAJEBlQKWACEDkQIhBJE4AAgiAiMUTADwHPgOIQCRAZUA
IQKROAACIQAiAiMPTADwEPgAICkAOgACIwxMAPAJ+AAgC0wA8AX4BEj/IQGABbDwvSBHwEaw+wMC
zHADArkNCgjJrg8IeTQRCGkwEQh9AwoIHZ8PCAIEBgcJCw0OA0gAiGQhSEMCSUAYcEfARrxwAwKA
QgICB0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6g1iLUPvRcEfYQgADJDYAAAD8AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBR8QDAtCueAQWjQ0KCKGuDwhRNBEIQTARCFEDCgj1ng==`),
                'BPRF 1.10': decodeBase64(`aAEBRsQDDrRAPwjlMgQIeToECHkb2AMBlXgEFkEQCgi1sg8IwTYRCLEyEQgFBgoICaPQBAEo`),
                'BPRD 1.10': decodeBase64(`aAEBRMQDDpi2PwgNMwQIoToECKEb2AMBqXgEFoEPCgj1sQ8IATYRCPExEQhFBQoISaLQBAEo`),
                'BPRI 1.10': decodeBase64(`aAEBScQDDiwuPwj5MgQIjToECI0b2AMBqXgEFm0PCgj1sQ8IRTYRCDUyEQgxBQoISaLQBAEo`),
                'BPRS 1.10': decodeBase64(`aAEBU8QDDlxgPwj5MgQIjToECI0b2AMBlXgEFlUQCgjdsg8ILTcRCB0zEQgZBgoIMaPQBAEo`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbEAw7wPj8I5TIECHk6BAh5G9gDAZV4BBYVEAoIjbIPCJk2EQiJMhEI2QUKCOGi0AQB
KA==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTEAw7UtD8IDTMECKE6BAihG9gDAal4BBZVDwoIzbEPCNk1EQjJMREIGQUKCCGi0AQB
KA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnEAw5oLD8I+TIECI06BAiNG9gDAal4BBZBDwoIzbEPCB02EQgNMhEIBQUKCCGi0AQB
KA==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPEAw6YXj8I+TIECI06BAiNG9gDAZV4BBYpEAoItbIPCAU3EQj1MhEI7QUKCAmj0AQB
KA==`),
            }),
        },
    },
    {
        id: 'custom-pokerus',
        label: 'Pokérus for Your Party',
        description: 'Gives every Pokémon in your party Pokérus, which doubles the EVs they earn in battle.',
        payloads: {
            ...romPayloads(decodeBase64(`9gNxAA4AAAAUAMrJxRvMz83///////////////////////////////////////////+719zj46v/
////////////////////////////////////////////uwDo3eLtAOrd5unnAOjc1egA3Nng5Of/
/////////////////////8rJxRvHycgA2+bj6wDn6Obj4tvZ5q0A0N3n3ej////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFcgAACB+vAAAIRbsFcgAACB+8AAAICrsFcgAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR718AAAIZm4UCCENgAAAuwFoAAAIIxUQAAPjAL3bAAAIZm1obAK9BQEACGZtaGwCvR0BAAhm
bWhsAsrJxRvMz80A3ecA1QDo3eLtAOrd5unnAOjc1ej+3Nng5OcAysnFG8fJyADb5uPrAOfo5uPi
29nmrfvR1eLoAO3j6eYA5NXm6O0AysnFG8fJyP7o4wDX1ejX3ADd6Kz/u9fc4+OrANPj6eYA5NXm
6O0AysnFG8fJyP7X1enb3OgAysnFG8zPzav/zejV7QDc2dXg6NztAOPp6ADo3Nnm2av/ztzd5wDb
3droAN3n4rToANrj5v7o3N3nANvV4dmt/zC1CUwGJeB8AyEIQAIoBdEgAAMhACISIwDwKfhkNAE9
8dEwvAG8AEeAQgICyQbKDohCANNAGpFCAdBJCPjncEcwtQQATQAgaBgh//fv/xyhCFzoQAMhCEAM
IUhDIDAgGDC8ArwIRwFoQGhIQHBH8LUHAAy0//fk/wQAOAD/9/P/BQAMvJEIiQBkGAMhEUDJACZo
bkAyAMpAEgYSDv8giECGQxgAiEAGQ25AJmCbGgggAUCLQLiLwBi4g/C8AbwAR+S02Jx4bOGx0pNy
Y8mNxodOSzktNiceGw==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRg==`),
                'BPRD 1.10': decodeBase64(`aAEBRA==`),
                'BPRI 1.10': decodeBase64(`aAEBSQ==`),
                'BPRS 1.10': decodeBase64(`aAEBUw==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM=`),
            }),
        },
    },
    {
        id: 'custom-friendship',
        label: 'Friendship Checker & Max Friendship',
        description: 'Choose a Pokémon from your party and the deliveryman tells you its friendship, out of 255, then offers to raise it to the maximum for that Pokémon or for your whole party. Pokémon that evolve through friendship, such as Pichu, Golbat and Chansey, then evolve at their next level up.',
        payloads: {
            ...romPayloads(decodeBase64(`+QOsABEAAAAQAMDMw7/Ivs3Cw8oAvcK/vcW/zP/////////////////////////////C4+sA1+Dj
59kA1ebZAO3j6az/////////////////////////////zdnZANzj6wDa5t3Z4tjg7QDVAMrJxRvH
ycgA3ee4/////////////+jc2eIA4dXf2QDd6ADj5gDt4+nmAOTV5ujtANXn///////////////a
5t3Z4tjg7QDV5wDX1eIA1tmtANDd593oAOjc2f//////////////2Nng3erZ5u3h1eIA4+IAo8AA
49oA1QC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF4QAACB+vAAAIRbsF4QAACB+8AAAICrsF4QAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73rAAAIZm0jFRAAAyADuFEAAAglnwAnIQSABgC7BN4AAAgmDYBHASENgJwBuwHUAAAIfwAE
gCMVEAADYQGDAQWAvSwBAAhmbb1OAQAIZiMVEAADXQEnIQ2AAgC7BMoAAAgjFRAAA1IBIQ2AAAC7
BcAAAAi9dQEACGZtaGwCvYgBAAhmbWhsAr2pAQAIZm1obAK9DAEACGZtaGwCaGwCvb0BAAhmbWhs
AtHc4+fZANrm3dni2Ofc3eQA59zj6eDYAMP+19zZ19+s/7viAL/BwQDc1efitOgA4dXY2QDa5t3Z
4tjnAO3Z6Kv//QK05wDa5t3Z4tjn3N3kAN3n/v0DAOPp6ADj2gCjpqat/83c1eDgAMMA4dXf2QDd
6ADV5wDa5t3Z4tjg7f7V5wDX1eIA1tms//0CANXY4+bZ5wDt4+kA4uPrq//T4+nmAOvc4+DZAOTV
5ujtANXY4+bZ5wDt4+n+4uPrq/+94+HZANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd5+K06ADa
4+b+6Nzd5wDb1eHZrf8AtQDwufggIRdLAPAm+BNJSIAAvRagAyELIoTgMLWBsP8gAJAOSACIACgE
0QDwpPgA8BD4DOALTAYl4HxAB0APAigC0SAAAPAF+GQ0AT300QGwML0gIWpGBEsYR7xwAwLMcAMC
gEICApEzBAglOwQIzsLDzQDKycUbx8nI/wDARtHCyca/AMq7zM7T/8jJAM7Cu8jFzf/ARvC1hbAN
ABcAK04AIwLIACIGxgEzq0L50SdOHCDAGwAhOgAto2weG10lTADwL/gHAAAhJEwA8Cr4DiEAkQGV
ApYAIQORAiEEkTgACCICIx5MAPAc+A4hAJEBlQAhApE4AAIhACICIxlMAPAQ+AAgKQA6AAIjFkwA
8An4ACAVTADwBfgOSP8hAYAFsPC9IEcQtYiwACOcAGxEIGAEeAEw/yz70QMwgAiAAAEzi0Ly0WhG
//em/wiwEL2w+wMCzHADArkNCgjJrg8IeTQRCGkwEQh9AwoIHZ8PCAIEBgcJCw0OA0gAiGQhSEMC
SUAYcEfARrxwAwKAQgICB0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6g1iLUPvRcEfYQgADJDYAAAD8
AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBR4QEFo0NCgihrg8IUTQRCEEwEQhRAwoI9Z4=`),
                'BPRF 1.10': decodeBase64(`aAEBRpQDBuUyBAh5OoQEFkEQCgi1sg8IwTYRCLEyEQgFBgoICaPcBAEo`),
                'BPRD 1.10': decodeBase64(`aAEBRJQDBg0zBAihOoQEFoEPCgj1sQ8IATYRCPExEQhFBQoISaLcBAEo`),
                'BPRI 1.10': decodeBase64(`aAEBSZQDBvkyBAiNOoQEFm0PCgj1sQ8IRTYRCDUyEQgxBQoISaLcBAEo`),
                'BPRS 1.10': decodeBase64(`aAEBU5QDBvkyBAiNOoQEFlUQCgjdsg8ILTcRCB0zEQgZBgoIMaPcBAEo`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUaUAwblMgQIeTqEBBYVEAoIjbIPCJk2EQiJMhEI2QUKCOGi3AQBKA==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSUAwYNMwQIoTqEBBZVDwoIzbEPCNk1EQjJMREIGQUKCCGi3AQBKA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmUAwb5MgQIjTqEBBZBDwoIzbEPCB02EQgNMhEIBQUKCCGi3AQBKA==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOUAwb5MgQIjTqEBBYpEAoItbIPCAU3EQj1MhEI7QUKCAmj3AQBKA==`),
            }),
        },
    },
    {
        id: 'custom-nature-mint',
        label: 'Nature Mint (Change Nature)',
        description: 'Choose a Pokémon from your party, then the stat its new nature raises and the one it lowers (the same stat twice gives a neutral nature). It keeps its gender, ability and shininess, and its stats update right away. Its IVs come with the new personality, as a wild Pokémon’s do, so PKHeX finds it legal.',
        payloads: {
            ...romPayloads(decodeBase64(`/wMrABcAAAAMAMi7zs/MvwDHw8jO//////////////////////////////////////+7ANrm2efc
AOLZ6wDi1ejp5tn/////////////////////////////yt3X3wDo3NkA5+jV6ADVAMrJxRvHyci0
5////////////////////+LV6Onm2QDm1d3n2ecA1eLYAOjc2QDj4tkA3ej////////////////g
4+vZ5uetANDd593oAOjc2QDY2eDd6tnm7eHV4v//////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFtAAACB+vAAAIRbsFtAAACB+8AAAICrsFtAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR72+AAAIZm0jFRAAA2gDuFEAAAglnwAnIQSABgC7BLEAAAh/AASAvdwAAAhmIxUQAAO/ACch
DYB/ALsBsQAACBkFgA2Ave4AAAhmIxUQAAOhACchDYB/ALsBsQAACCMVEAADmAC9AAEACGZtaGwC
aGwCvQ4BAAhmbWhsAtHc4+fZAOLV6Onm2QDn3OPp4NgAwwDX3NXi29ms/8zV3efZAOvc3dfcAOfo
1eis/8bj69nmAOvc3dfcAOfo1eis//0CAN3nAP0DAOLj66v/ztzd5wDb3droAN3n4rToANrj5v7o
3N3nANvV4dmt/wAAABBIBDAFIQgiKOAwtQ5IRYgFIU1DDUgAiC0YAPAd+QQAIXgqAADwdvggaBkh
APAg+QdJgAAJWAdIB0sA8AH4ML0YR8BGlLA/CLxwAwLMcAMCbBhGCPAcAgKtyAAI8LWFsA0AFwAh
TgAjAsgAIgbGATOrQvnRHU4cIMAbACE6ACOjbB4bXRtMAPAv+AcAACEaTADwKvgOIQCRAZUClgAh
A5ECIQSROAAIIgIjFEwA8Bz4DiEAkQGVACECkTgAAiEAIgIjD0wA8BD4ACApADoAAiMMTADwCfgA
IAtMAPAF+ARI/yEBgAWw8L0gR8BGsPsDAsxwAwK5DQoIya4PCHk0EQhpMBEIfQMKCB2fDwgCBAYH
CQsNDvC1lLAFAA2RDJIuaBGWqIjpiEhAMQxBQDIEEgxRQAgpAdMAIMBDDpAoAAshP0sA8Hr4DZ8A
IskoANFBShOSPgQ9STQATEM9SqQYATY6SWQYMAy4QifRIAwOmlMcA9BCQHpACCrx0gAEOEMRmkJA
E5saQurRD5AZIQDwcPgMmYhC49EtSS1KIABIQ4AYAwBLQ5sYQABADFsAWwzbAxhDEJAPnxGeBeD/
NwE3OAzH0AAgN+AoACAwaUYsIoNYi1AEOvvVAyRoRiA4MQAiABZLAPAq+AyQKAA5ACIAE0sA8CP4
DJkIIotYc0B7QINQBDr51QE859UvYAAkEJhBCRCRHyEIQA+QKAAnIQkZD6oJSwDwCvgBNAYs79Eo
AAVLAPAD+AEgFLDwvRhH6TAECJEzBAglHAQIJTsECG1OxkFzYAAAAAMDAwNIAIhkIUhDAklAGHBH
wEa8cAMCgEICAskGyg6IQgDTQBqRQgHQSQj453BHB0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6g1iL
UPvRcEfYQgADJDYAAAD8AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBR8QCAtCu0AICnBJ0AxaNDQoIoa4PCFE0EQhBMBEIUQMKCPWe`),
                'BPRF 1.10': decodeBase64(`aAEBRsQCArRA0AIDlLZF2AIBlXQDFkEQCgi1sg8IwTYRCLEyEQgFBgoICaO8BA49MAQI5TIECHkb
BAh5OiQFASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRMQCApi20AICTDHYAgGpdAMWgQ8KCPWxDwgBNhEI8TERCEUFCghJorwEDmUwBAgNMwQIoRsE
CKE6JAUBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBScQCAiwu0AIDdJBF2AIBqXQDFm0PCgj1sQ8IRTYRCDUyEQgxBQoISaK8BA5RMAQI+TIECI0b
BAiNOiQFASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU8QCAlxg0AIDCMdF2AIBlXQDFlUQCgjdsg8ILTcRCB0zEQgZBgoIMaO8BA5RMAQI+TIECI0b
BAiNOiQFASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbEAgLwPtACA1SkRdgCAZV0AxYVEAoIjbIPCJk2EQiJMhEI2QUKCOGivAQOPTAECOUy
BAh5GwQIeTokBQEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTEAgLUtNACAlAj2AIBqXQDFlUPCgjNsQ8I2TURCMkxEQgZBQoIIaK8BA5lMAQIDTME
CKEbBAihOiQFASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnEAgJoLNACA6SIRdgCAal0AxZBDwoIzbEPCB02EQgNMhEIBQUKCCGivAQOUTAECPky
BAiNGwQIjTokBQEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPEAgKYXtACA5i+RdgCAZV0AxYpEAoItbIPCAU3EQj1MhEI7QUKCAmjvAQOUTAECPky
BAiNGwQIjTokBQEo`),
            }),
        },
    },
    {
        id: 'custom-ability-capsule',
        label: 'Ability Capsule (Swap Ability)',
        description: 'Switches a Pokémon from your party to the other ability its species can have, keeping its nature, gender and shininess. Species with only one ability can’t switch. Its IVs come with the new personality, as a wild Pokémon’s do, so PKHeX finds it legal.',
        payloads: {
            ...romPayloads(decodeBase64(`AATpABgAAAAYALu8w8bDztMAvbvKzc/Gv//////////////////////////////////O5u0A3ejn
AOPo3NnmANXW3eDd6O3/////////////////////////zevd6NfcANUAysnFG8fJyADo4wDo3NkA
4+jc2eb//////////////9XW3eDd6O0A3ejnAOfk2dfd2ecA19XiANzV6tmt///////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFrAAACB+vAAAIRbsFrAAACB+8AAAICrsFrAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR722AAAIZm0jFRAAAxQDuFEAAAglnwAnIQSABgC7BKkAAAgmDYBHASENgJwBuwGfAAAIfwAE
gCMVEAADyQAhDYAAALsBlQAACL30AAAIZm1obAK9DAEACGZtaGwCvdUAAAhmbWhsAmhsAr0lAQAI
Zm1obALR3OPn2QDV1t3g3ejtAOfc4+ng2ADDAOfr3ejX3Kz/u+IAv8HBANfV4rToAOfr3ejX3ADV
1t3g3ejd2eer//0CtOcA1dbd4N3o7QDd5wDi4+v+/QOr//0CANzV5wDj4uDtAOPi2f7V1t3g3ejt
rf/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/8LWBsADw9PgEAAshIUsA8D34HCFIQyFJ
RhgAIPF9ACkw0CV4ASdvQDJ8ACoL0P4qCdKVQoBBl0KJQYhCA9BvHmgIANNvHCBoGSEA8N34AgA5
ACAAAPAq+AAoE9ABIAdAAJcgAC4hakYLSwDwD/gWNvFdDSBBQwpICRgKSApLAPAF+AEgAkkIgAGw
8L0YR8BGzHADApEzBAglOwQIUCMlCAzYJAjwHAICrcgACPC1lLAFAA2RDJIuaBGWqIjpiEhAMQxB
QDIEEgxRQAgpAdMAIMBDDpAoAAshP0sA8Hr4DZ8AIskoANFBShOSPgQ9STQATEM9SqQYATY6SWQY
MAy4QifRIAwOmlMcA9BCQHpACCrx0gAEOEMRmkJAE5saQurRD5AZIQDwcPgMmYhC49EtSS1KIABI
Q4AYAwBLQ5sYQABADFsAWwzbAxhDEJAPnxGeBeD/NwE3OAzH0AAgN+AoACAwaUYsIoNYi1AEOvvV
AyRoRiA4MQAiABZLAPAq+AyQKAA5ACIAE0sA8CP4DJkIIotYc0B7QINQBDr51QE859UvYAAkEJhB
CRCRHyEIQA+QKAAnIQkZD6oJSwDwCvgBNAYs79EoAAVLAPAD+AEgFLDwvRhH6TAECJEzBAglHAQI
JTsECG1OxkFzYAAAAAMDAwNIAIhkIUhDAklAGHBHwEa8cAMCgEICAskGyg6IQgDTQBqRQgHQSQj4
53BHB0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6g1iLUPvRcEfYQgADJDYAAAD8AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBRzADBiwjJQjo1w==`),
                'BPRF 1.10': decodeBase64(`aAEBRigDDuUyBAh5OgQI/M0kCLiCPAMBlWgEDj0wBAjlMgQIeRsECHk60AQBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRCgDDg0zBAihOgQI0CglCIzdPAMBqWgEDmUwBAgNMwQIoRsECKE60AQBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSSgDDvkyBAiNOgQIjLokCEhvPAMBqWgEDlEwBAj5MgQIjRsECI060AQBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBUygDDvkyBAiNOgQIdOEkCDCWPAMBlWgEDlEwBAj5MgQIjRsECI060AQBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYoAw7lMgQIeToECNjNJAiUgjwDAZVoBA49MAQI5TIECHkbBAh5OtAEASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQoAw4NMwQIoToECKwoJQho3TwDAaloBA5lMAQIDTMECKEbBAihOtAEASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkoAw75MgQIjToECGi6JAgkbzwDAaloBA5RMAQI+TIECI0bBAiNOtAEASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMoAw75MgQIjToECFDhJAgMljwDAZVoBA5RMAQI+TIECI0bBAiNOtAEASg=`),
            }),
        },
    },
    {
        id: 'custom-poke-ball-changer',
        label: 'Poké Ball Changer',
        description: 'Moves a Pokémon from your party into the Poké Ball of your choice: any ball but the Safari Ball. Its summary shows the new ball.',
        payloads: {
            ...romPayloads(decodeBase64(`KARkAEAAAAAUAMrJxRsAvLvGxgC9wrvIwb/M//////////////////////////////+7AOLZ6wDc
4+HZANrj5gDVAMrJxRvHycj/////////////////////x+Pq2QDVAMrJxRvHycgA3eLo4wDo3NkA
ysnFG////////////////7y7xsYA49oA7ePp5gDX3OPd19mtANDd593o///////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF3wAACB+vAAAIRbsF3wAACB+8AAAICrsF3wAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73pAAAIZm0jFRAAA/gCuFEAAAglnwAnIQSABgC7BNwAAAgmDYBHASENgJwBuwHSAAAIfwAE
gBYGgAAAvT0BAAhmIxUQAAMmASchDYB/ALsBswAACCMVEAADVwEhDYAAALsFfgAACL1cAQAIZm1o
bAIhBoAAALsByAAACBYGgAAAuX4AAAi9dgEACGZtaGwCvRMBAAhmbWhsAmhsAr2KAQAIZm1obALR
3N3X3ADKycUbx8nIAOfc4+ng2ADb2egA1f7i2esAysnFGwC8u8bGrP+74gC/wcEA3NXn4rToANbZ
2eIA19Xp29zoAN3iANX+ysnFGwC8u8bGq//R3N3X3ADKycUbALy7xsYA6+Pp4NgA3egA4N3f2az/
/QIA4uPrANfV4ODnAN3o5/79AwDc4+HZq/+94+HZANbV198A1eLtAOjd4dmr/87c3ecA293a6ADd
5+K06ADa4+b+6Nzd5wDb1eHZrf8AAAAwtYiwJUiAiCqkByUAKAHQBzQEJQAj4FwA8Dn4mQBpRAhg
ATOrQvbRBy0E0SSgmQBpRAhgATVoRikADSIA8ET4CLAwvRC1gbAUS5iIFEkJiAAoBNEHKQPRASCY
gBPgBzEUoERcAJQA8Iv4JiFqRg1LAPAS+CAAAPAK+AEAC0gMSwDwCvgAIAZJCIABsBC9LCFIQwVJ
QBhwRxhHwEa8cAMCzHADAiU7BAgkiz0I8BwCAq3IAAgEAwIBBgcICQoLDADHycy/sP/ARvC1hbAN
ABcAIU4AIwLIACIGxgEzq0L50R1OHCDAGwAhOgAjo2weG10bTADwL/gHAAAhGkwA8Cr4DiEAkQGV
ApYAIQORAiEEkTgACCICIxRMAPAc+A4hAJEBlQAhApE4AAIhACICIw9MAPAQ+AAgKQA6AAIjDEwA
8An4ACALTADwBfgESP8hAYAFsPC9IEfARrD7AwLMcAMCuQ0KCMmuDwh5NBEIaTARCH0DCggdnw8I
AgQGBwkLDQ4DSACIZCFIQwJJQBhwR8BGvHADAoBCAgIHSABoB0lAGAdJmmgSGlIYmmD5IpIABDqD
WItQ+9FwR9hCAAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBR6QDAmCJXAQWjQ0KCKGuDwhRNBEIQTARCFEDCgj1ng==`),
                'BPRF 1.10': decodeBase64(`aAEBRqADBnk6BAhMFawDAZVcBBZBEAoItbIPCME2EQixMhEIBQYKCAmjtAQBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRKADBqE6BAhAh6wDAalcBBaBDwoI9bEPCAE2EQjxMREIRQUKCEmitAQBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSaADBo06BAgQAawDAalcBBZtDwoI9bEPCEU2EQg1MhEIMQUKCEmitAQBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBU6ADBo06BAh4MawDAZVcBBZVEAoI3bIPCC03EQgdMxEIGQYKCDGjtAQBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUagAwZ5OgQIiBOsAwGVXAQWFRAKCI2yDwiZNhEIiTIRCNkFCgjhorQEASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSgAwahOgQIfIWsAwGpXAQWVQ8KCM2xDwjZNREIyTERCBkFCgghorQEASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmgAweNOgQITP88rAMBqVwEFkEPCgjNsQ8IHTYRCA0yEQgFBQoIIaK0BAEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOgAwaNOgQItC+sAwGVXAQWKRAKCLWyDwgFNxEI9TIRCO0FCggJo7QEASg=`),
            }),
        },
    },
    {
        id: 'custom-pokemon-gender',
        label: 'Pokémon Gender Change',
        description: 'Switches a Pokémon from your party between male and female, handy for breeding. It keeps its nature, ability and shininess. Species that are always one gender, or have none, can’t switch. Its IVs come with the new personality, as a wild Pokémon’s do, so PKHeX finds it legal.',
        payloads: {
            ...romPayloads(decodeBase64(`AQQgABkAAAAQAMrJxRvHycgAwb/Ivr/MAL3Cu8jBv//////////////////////////A4+YA6NzZ
AOTZ5trZ1+gA5NXd5v//////////////////////////zevd6NfcANUAysnFG8fJyADW2ejr2dni
AOHV4Nn//////////////9Xi2ADa2eHV4NmtANDd593oAOjc2f/////////////////////////Y
2eDd6tnm7eHV4gDj4gDo3NkAo+LYANrg4+Pm////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFwQAACB+vAAAIRbsFwQAACB+8AAAICrsFwQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73LAAAIZm0jFRAAAwQDuFEAAAglnwAnIQSABgC7BL4AAAgmDYBHASENgJwBuwG0AAAIfwAE
gCMVEAAD/QAhDYAAALsBqgAACCENgAIAuwGgAAAIvRYBAAhmbWhsAr0mAQAIZm1obAK9OAEACGZt
aGwCve8AAAhmbWhsAmhsAr1XAQAIZm1obALR3N3X3ADKycUbx8nIAOfc4+ng2ADn693o19z+29ni
2NnmrP/G2ei05wDr1d3oANrj5gDo3NkAv8HBAOjj/tzV6NfcANrd5ufoq//9AgDd5wDi4+sA4dXg
2av//QIA3ecA4uPrANrZ4dXg2av//QK05wDb2eLY2eYA19XitOj+1tkA5+vd6Nfc2dit/87c3ecA
293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAHC1APDT+AQACyEVSwDwJPgcIUhDE0lAGAJ8ACYA
KhjQ/ioW0iV4EQBpQAEgAUABJpVCAdMCOgImVRggaBkhAPDB+AIAKQAgAADwDvgAKADRACYCSQ6A
cL0YR8BGzHADApEzBAhQIyUI8LWUsAUADZEMki5oEZaoiOmISEAxDEFAMgQSDFFACCkB0wAgwEMO
kCgACyE/SwDwevgNnwAiySgA0UFKE5I+BD1JNABMQz1KpBgBNjpJZBgwDLhCJ9EgDA6aUxwD0EJA
ekAIKvHSAAQ4QxGaQkATmxpC6tEPkBkhAPBw+AyZiELj0S1JLUogAEhDgBgDAEtDmxhAAEAMWwBb
DNsDGEMQkA+fEZ4F4P83ATc4DMfQACA34CgAIDBpRiwig1iLUAQ6+9UDJGhGIDgxACIAFksA8Cr4
DJAoADkAIgATSwDwI/gMmQgii1hzQHtAg1AEOvnVATzn1S9gACQQmEEJEJEfIQhAD5AoACchCRkP
qglLAPAK+AE0Bizv0SgABUsA8AP4ASAUsPC9GEfpMAQIkTMECCUcBAglOwQIbU7GQXNgAAAAAwMD
A0gAiGQhSEMCSUAYcEfARrxwAwKAQgICyQbKDohCANNAGpFCAdBJCPjncEcHSABoB0lAGAdJmmgS
GlIYmmD5IpIABDqDWItQ+9FwR9hCAAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRywDASw=`),
                'BPRF 1.10': decodeBase64(`aAEBRigDB+UyBAj8zSRYBA49MAQI5TIECHkbBAh5OsAEASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRCgDBg0zBAjQKFgEDmUwBAgNMwQIoRsECKE6wAQBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSSgDB/kyBAiMuiRYBA5RMAQI+TIECI0bBAiNOsAEASg=`),
                'BPRS 1.10': decodeBase64(`aAEBUygDB/kyBAh04SRYBA5RMAQI+TIECI0bBAiNOsAEASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYoAwflMgQI2M0kWAQOPTAECOUyBAh5GwQIeTrABAEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQoAwYNMwQIrChYBA5lMAQIDTMECKEbBAihOsAEASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkoAwf5MgQIaLokWAQOUTAECPkyBAiNGwQIjTrABAEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMoAwf5MgQIUOEkWAQOUTAECPkyBAiNGwQIjTrABAEo`),
            }),
        },
    },
    {
        id: 'custom-unown-letters',
        label: 'Unown Letter Changer',
        description: 'Choose an Unown from your party and type the letter it should be, A to Z, ! or ?, on the naming screen; its nickname stays as it was. It keeps its nature and whether it is shiny. PKHeX won’t pass the result: each chamber of the Tanoby Ruins has its own letters.',
        payloads: {
            ...romPayloads(decodeBase64(`IATJADgAAAAYAM/IydHIAMa/zs6/zAC9wrvIwb/M///////////////////////////A5uPhALsA
6OMArP//////////////////////////////////////wd3q2QDV4gDPyMnRyADV4u0A49oA3ejn
AKOp/////////////////+DZ6OjZ5uetAMPoAN/Z2eTnAN3o5wDi1ejp5tmt///////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF2QAACB+vAAAIRbsF2QAACB+8AAAICrsF2QAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR73jAAAIZm0jFRAAAxwDuFEAAAglnwAnIQSABgC7BNYAAAh/AASAJg2ARwEhDYDJALsFwgAA
CL3wAAAIZm1olwEjFRAAAxMBJyMVEAADMQF/AASAIQ2AAAC7AcwAAAghDYACALsBtgAACL03AQAI
Zm1obAK9FAEACGZtuYAAAAi9UAEACGZtaGwCvWUBAAhmbWhsAmhsAr15AQAIZm1obALR3N3X3ADP
yMnRyKz/zu3k2QDd6OcA4tnrAODZ6OjZ5vD+uwDo4wDUuACrAOPmAKz/yeLZAODZ6OjZ5rgA5ODZ
1efZ8P67AOjjANS4AKsA4+YArP/9AgDd5wDo3NkA4Nno6Nnm/v0DAOLj66v/ztzV6LTnAOLj6ADV
4gDPyMnRyKv/vePh2QDW1dffANXi7QDo3eHZq//O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh
2a3/ELWCsADwzvgAaACQTUgBkElJ/yAIcAMgySL/I0hMAPCI+AKwEL3wtYSwQ0kAIgsAHHgBM/8s
BNAALPnQIAABMvbnASoD0AAgYtMCIGDgAgC7OhoqCtMaOhoqB9MaIqsoBNAbIqwoAdACIFHgAJK7
IBoqANORIIAYCHD/IEhwAPCR+AQAJmgwABkhAPCX+AUAYGgBDEhAAAQADAKQMQxxQAkECQxIQAgo
AdMAIclDAZEAIAOQIUsA8Dv4BwAfSwDwN/gBmUocBNBAB0APSEB4QAXgAQB5QAKaUUAIKQzTAAQH
QzgAGSEA8Gn4qEIE0QDwE/gAmYhCBtADmAEwA5AADNjQACAE4CAAOQAA8B34ASAISQiABLDwvQAg
GCGAADoAykADIxpAEEMIOffVHCFF4BhHIEfMcAMC8BwCAnGGBAi5EAoIpaAFCPC1lLAFAA8ALmgo
ACAwaUYsIoNYi1AEOvvVAyRoRiA4MQAiAAxLAPAV+AyQKAA5ACIACUsA8A74DJkIIotYc0B7QINQ
BDr51QE859UvYAEgFLDwvRhHwEbpMAQIA0gAiGQhSEMCSUAYcEfARrxwAwKAQgICyQbKDohCANNA
GpFCAdBJCPjncEcHSABoB0lAGAdJmmgSGlIYmmD5IpIABDqDWItQ+9FwR9hCAAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRygEAY0=`),
                'BPRF 1.10': decodeBase64(`aAEBRiQECrGGBAhBEwoIAaKIBAE92AQBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRCQECvGFBAiBEgoIQaGIBAFl2AQBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSSQECt2FBAhtEgoILaGIBAFR2AQBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBUyQECsWGBAhVEwoIFaKIBAFR2AQBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUYkBAqxhgQIFRMKCAGiiAQBPdgEASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQkBArxhQQIVRIKCEGhiAQBZdgEASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUkkBArdhQQIQRIKCC2hiAQBUdgEASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVMkBArFhgQIKRMKCBWiiAQBUdgEASg=`),
            }),
        },
    },
    {
        id: 'custom-max-conditions',
        label: 'Max Contest Conditions (Feebas to Milotic)',
        description: 'Raises a Pokémon’s Cool, Beauty, Cute, Smart and Tough conditions and its sheen to the maximum, as far as Pokéblocks can take them. A Feebas in this condition evolves into Milotic at its next level up, even in FireRed and LeafGreen, which have no way to raise Beauty.',
        payloads: {
            ...romPayloads(decodeBase64(`BARJARwAAAAEAMe70gC9yci+w87DycjN//////////////////////////////////+94+Lo2efo
AObZ1djtq///////////////////////////////////vcnJxrgAvL+7z87TuAC9z86/uADNx7vM
zgDV4tj//////////////87Jz8HCAOjjAOjc2QDh1ezwANUAwL+/vLvN///////////////////o
3NniANnq4+Dq2ecA1egA3ejnAOLZ7OgA4Nnq2eCt////////////0N3n3egAo8AA49oA1QDKycUb
x8nIAL2/yM6/zK3//////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFlwAACB+vAAAIRbsFlwAACB+8AAAICrsFlwAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR72hAAAIZm0jFRAAA2ABuFEAAAglnwAnIQSABgC7BJQAAAgmDYBHASENgJwBuwGKAAAIfwAE
gCMVEAAD5QC97QAACGZtaGwCvdAAAAhmbWhsAmhsAr0/AQAIZm1obALR3N3X3ADKycUbx8nIAOfc
4+ng2ADDANvZ6P7m2dXY7QDa4+YA1+Pi6Nnn6Oes/7viAL/BwQDX1eK06ADZ4ujZ5gDX4+Lo2efo
56v//QIA3ecA3eIA6OPkANfj4tjd6N3j4qv7uwDAv7+8u80A3eIA6Nzd5wDX4+LY3ejd4+IA693g
4P7Z6uPg6tkA1egA3ejnAOLZ7OgA4Nnq2eCt/87c3ecA293a6ADd5+K06ADa4+b+6Nzd5wDb1eHZ
rf8AACC1gbD/IACQACUA8BP4B6FJXWpGBEsA8AX4ATUGLfTRAbAgvRhHwEYlOwQIFhcYIS8wwEYD
SACIZCFIQwJJQBhwR8BGvHADAoBCAgIHSABoB0lAGAdJmmgSGlIYmmD5IpIABDqDWItQ+9FwR9hC
AAMkNgAAAPwDAg==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRtgCAnk6HAMBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBRNgCAqE6HAMBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSdgCAo06HAMBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBU9gCAo06HAMBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbYAgJ5OhwDASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTYAgKhOhwDASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnYAgKNOhwDASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPYAgKNOhwDASg=`),
            }),
        },
    },
    {
        id: 'custom-gender-swap',
        label: 'New Trainer Name/Gender',
        description: 'Offers you a new trainer name, then offers to switch your character between boy and girl on the spot, sprite included; say no to either to skip it. Your own Pokémon take the new name as their original trainer, so they still count as yours. Talk to the deliveryman again to switch back.',
        payloads: {
            ...romPayloads(decodeBase64(`9QOEAA0AAAAIAMi/0QDOzLvDyL/MAMi7x7+6wb/Ivr/M//////////////////////+7AOLZ6wDi
1eHZuADVAOLZ6wDg4+Pfq///////////////////////yNnrAOLV4dmsAMjZ6wDg4+PfrADQ3efd
6ADo3Nn//////////////9jZ4N3q2ebt4dXiAOPiAOjc2QCj4tgA2uDj4+b////////////////j
2gDVAMrJxRvHycgAvb/Izr/MAOjjAObZ4tXh2f//////////////4+YA5+vV5ADW2ejr2dniALzJ
0wDV4tgAwcPMxq3//////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFsAAACB+vAAAIRbsFsAAACB+8AAAICrsFsAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR726AAAIZm4UCCENgAAAuwF8AAAIIxUQAAMbArheAAAIaJcBIxUQAANOAScjFRAAA2YBvdUA
AAhmbb3rAAAIZm4UCCENgAAAuwGmAAAIaJcDIxUQAAPfAJcCvRIBAAhmbWhsAr1BAQAIZm1obAK9
VQEACGZtaGwC0ePp4NgA7ePpAODd39kA1QDi2esA4tXh2az/yN3X2QDo4wDh2dnoAO3j6bgA/QGr
/83c1eDgAMMA5+vV5ADt4+kA1tno69nZ4v68ydMA1eLYAMHDzMas/87VrtjVqwDO1eDfAOjjAOHZ
ANXb1d3iANXi7f7o3eHZAOjjAOfr1eQA1tXX363/vePh2QDW1dffANXi7QDo3eHZq//O3N3nANvd
2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/MLUzSABoAXoBIlFAAXIyTOFxZXkkIEVDMEgtGAAgM0sA
8FP4AQAoADFLAPBO+Cl+CQcJDygAL0sA8Ef4MLwBvABHELWCsC1LAZMAIACQIEkJaAp6ACMAICdM
APA3+AKwEL3wtRpPP2gNIC0COVwNQwE4Cij52hpMBiZkIgDwE/gYTCRoBDRpJrYAUCIA8Av4EEwk
aBRIJBgCJowiAPAD+PC8AbwARwE+DtThfIkHCdVhaKlCBtEhABQxByABODtcC1T70aQY7udwRxhH
IEfARtxCAAPYQgADdHADAjRuAwKAQgIC4EIAA4AvAACF/wUIBSgGCL0pBgi5EAoIiaAFCAdIAGgH
SUAYB0maaBIaUhiaYPkikgAEOoNYi1D70XBH2EIAAyQ2AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBR7wDAY0=`),
                'BPRF 1.10': decodeBase64(`aAEBRpQDBSxCAAMoqAMBMLADEuEABghhKQYIGSsGCEETCgjloeQDASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRJQDBSxCAAMoqAMBMLADEiEABgihKAYIWSoGCIESCggloeQDASg=`),
                'BPRI 1.10': decodeBase64(`aAEBSZQDBSxCAAMoqAMBMLADEg0ABgiNKAYIRSoGCG0SCggRoeQDASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU5QDBSxCAAMoqAMBMLADEvUABgh1KQYILSsGCFUTCgj5oeQDASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUaUAwUsQgADKKgDATCwAxLhAAYIYSkGCBkrBggVEwoI5aHkAwEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSUAwUsQgADKKgDATCwAxIhAAYIoSgGCFkqBghVEgoIJaHkAwEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmUAwUsQgADKKgDATCwAxINAAYIjSgGCEUqBghBEgoIEaHkAwEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOUAwUsQgADKKgDATCwAxL1AAYIdSkGCC0rBggpEwoI+aHkAwEo`),
            }),
        },
    },
    {
        id: 'custom-rival-name',
        label: 'Rename Your Rival',
        description: 'Gives your rival a new name with the game’s own naming screen.',
        payloads: {
            ...romPayloads(decodeBase64(`DQSFACUAAAAQAMy/yLvHvwDTyc/MAMzD0LvG///////////////////////////////N4dng4ADt
1QDg1ejZ5qv/////////////////////////////////wd3q2QDt4+nmAObd6tXgANUA4tnrAOLV
4dmt/////////////////9Dd593oAOjc2QDY2eDd6tnm7eHV4gDj4gDo3Nn///////////////+j
4tgA2uDj4+YA49oA1QDKycUbx8nI////////////////////////vb/Izr/Mrf//////////////
/////////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFggAACB+vAAAIRbsFggAACB+8AAAICrsFggAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR72MAAAIZm4UCCENgAAAuwF4AAAIIxUQAAPrALheAAAIaJcBIxUQAAOkACe9ugAACGZtaGwC
vdkAAAhmbWhsAr3tAAAIZm1obALR4+ng2ADt4+kA4N3f2QDo4wDb3erZAO3j6eb+5t3q1eAA1QDi
2esA4tXh2az/wObj4QDi4+sA4+K4AO3j6eYA5t3q1eAA3ef+/Qar/73j4dkA1tXX3wDV4u0A6N3h
2av/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt/xC1grALSwGTACAAkAZJCWgGSokYACIA
IwQgBUwA8AL4ArAQvSBHwEbYQgADTDoAALkQCgiJoAUIB0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6
g1iLUPvRcEfYQgADJDYAAAD8AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBR4wCAY0=`),
                'BPRF 1.10': decodeBase64(`aAEBRoQCASiMAgZBEwoI5aG0AgEo`),
                'BPRD 1.10': decodeBase64(`aAEBRIQCASiMAgaBEgoIJaG0AgEo`),
                'BPRI 1.10': decodeBase64(`aAEBSYQCASiMAgZtEgoIEaG0AgEo`),
                'BPRS 1.10': decodeBase64(`aAEBU4QCASiMAgZVEwoI+aG0AgEo`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUaEAgEojAIGFRMKCOWhtAIBKA==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSEAgEojAIGVRIKCCWhtAIBKA==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmEAgEojAIGQRIKCBGhtAIBKA==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOEAgEojAIGKRMKCPmhtAIBKA==`),
            }),
        },
    },
    {
        id: 'custom-nickname',
        label: 'Nickname Change & Removal',
        description: 'Choose a Pokémon from your party to give it a new nickname, or to take its nickname away so it goes by its species name again. Works on Pokémon from trades too; one from another language’s game can get a new nickname, but your game doesn’t know its species name in that language to give back.',
        payloads: {
            ...romPayloads(decodeBase64(`/QPJABUAAAAMAMjDvcXIu8e/AL3Cu8jBv/////////////////////////////////+7AOLZ6wDi
1eHZuADj5gDi4+LZANXoANXg4P//////////////////wd3q2QDVAMrJxRvHycgA1QDi2esA4t3X
3+LV4dn//////////////+PmAN3o5wDn5NnX3dnnAOLV4dkA1tXX360A0N3n3ej////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF8wAACB+vAAAIRbsF8wAACB+8AAAICrsF8wAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR739AAAIZm0jFRAAA0ACuFEAAAglnwAnIQSABgC7BPEAAAgmDYBHASENgJwBuwHdAAAIfwAE
gL0cAQAIZm4UCCENgAEAuwHIAAAIIxUQAANVASENgAAAuwHnAAAIvTQBAAhmbhQIIQ2AAAC7AecA
AAgjFRAAA3sBfwAEgL1jAQAIZm1obAJolwElngAnfwAEgL14AQAIZm1obAK9jgEACGZtaGwCva4B
AAhmbWhsAmwCvcIBAAhmbWhsAtHc4+fZAOLd19/i1eHZAOfc1eDgAMMA19zV4tvZrP/B3erZAP0C
ANUA4tnrAOLd19/i1eHZrP/N3OPp4NgA/QIA2+MA1tXX3wDo4/7d6OcA5+TZ193Z5wDi1eHZAN3i
5+jZ1dis/77j4tmrAMPotOcA/QIA1dvV3eKt/8Dm4+EA4uPrAOPiuADd6LTnAP0Cq/+74gC/wcEA
2OPZ5+K06ADc1erZANUA4tXh2QDt2eir/73j4dkA1tXX3wDV4u0A6N3h2av/ztzd5wDb3droAN3n
4rToANrj5v7o3N3nANvV4dmt/wAAABC1APAv+CAAAyEkSwDwPfgCKAHQACAU4CAAAiEeSiAyHksA
8DL4HEgBACAxAngLeJpCBdEBMAEx/yr30QAgAOABIBRJCIAQvAG8AEcQtQDwCfggAAIhEEoRSwDw
FvgQvAG8AEcAtQpIAIhkIUhDB0wkGCAACyEJSwDwB/gBAAZICEsA8AL4AbwARxhHgEICArxwAwLM
cAMCAMABApEzBAglOwQIeUcECAdIAGgHSUAYB0maaBIaUhiaYPkikgAEOoNYi1D70XBH2EIAAyQ2
AAAA/AMC`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRkQDAQPQAwrlMgQIeToECM1G/AMBKA==`),
                'BPRD 1.10': decodeBase64(`aAEBREQDAQXQAwoNMwQIoToECPVG/AMBKA==`),
                'BPRI 1.10': decodeBase64(`aAEBSUQDAQTQAwr5MgQIjToECOFG/AMBKA==`),
                'BPRS 1.10': decodeBase64(`aAEBU0QDAQfQAwr5MgQIjToECOFG/AMBKA==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZEAwED0AMK5TIECHk6BAjNRvwDASg=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUREAwEF0AMKDTMECKE6BAj1RvwDASg=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlEAwEE0AMK+TIECI06BAjhRvwDASg=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNEAwEH0AMK+TIECI06BAjhRvwDASg=`),
            }),
        },
    },
    {
        id: 'custom-trainer-ids',
        label: 'Trainer & Secret ID Reveal',
        description: 'Shows your Trainer ID and the Secret ID the game never shows you. Together they decide which Pokémon you meet are shiny.',
        payloads: {
            ...romPayloads(decodeBase64(`BwQ/AB8AAAAYAM7Mu8PIv8wAw74AzL/Qv7vG//////////////////////////////+84+jcAOPa
AO3j6eYAw77n////////////////////////////////xtnV5uIA7ePp5gDOzLvDyL/MAMO+ANXi
2ADo3Nn//////////////82/vcy/zgDDvgDo3NkA29Xh2QDc3djZ563////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFVAAACB+vAAAIRbsFVAAACB+8AAAICrsFVAAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyMVEAADnwC9XgAACGZtaGwCvcUAAAhmbWhsAtPj6eYAzsy7w8i/zADDvgDd5wD9Av7V4tgA
7ePp5gDNv73Mv84Aw74A3ecA/QOt+87j29no3NnmAOjc2e0A2NnX3djZAOvc3dfc/srJxRvHycgA
7ePpAOHZ2egA1ebZAOfc3eLtrf/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/MLUITCRo
Ck1hiQdIAiIFIwDwB/ihiQVIAiIFIwDwAfgwvShH3EIAA9AcAgLwHAICockACA==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRlgCASxkAgGJ`),
                'BPRD 1.10': decodeBase64(`aAEBRFgCASxkAgGd`),
                'BPRI 1.10': decodeBase64(`aAEBSVgCASxkAgGd`),
                'BPRS 1.10': decodeBase64(`aAEBU1gCASxkAgGJ`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZYAgEsZAIBiQ==`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURYAgEsZAIBnQ==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlYAgEsZAIBnQ==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNYAgEsZAIBiQ==`),
            }),
        },
    },
    {
        id: 'custom-wishmkr-jirachi',
        label: 'Jirachi (WISHMKR Event)',
        description: 'A Jirachi like the one the Pokémon Colosseum Bonus Disc gave in North America: level 5, OT WISHMKR, ID 20043, from Ruby, knowing Wish, Confusion and Rest and holding a Salac or Ganlon Berry. Its nature, IVs and berry are rolled the way the disc rolled them, so it can even be shiny, as a few from the disc were. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`FgSZAS4AAAAMANHDzcLHxcwAxMPMu73Cw//////////////////////////////////O3NkAvMnI
z80AvsPNvQDb3dro////////////////////////////ztzZAOvd59yu2+bV4ujd4tsAxMPMu73C
wwDj2v///////////////+jc2QC9ycbJzc2/z8cAvMnIz80AvsPNva3////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+0AIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbgAAAhmbWhsAr3PAAAIZm1obAK9+wAACGZtaGwCvRsBAAhmbWhsAv0BAObZ19nd6tnYAMTD
zLu9wsOr/8PoAOvV5wDn2eLoAOjjAOjc2QDKva3/zNnX2d3q2QDo3NkA19Xm2ADV29Xd4gDa4+b+
1eLj6NzZ5gDEw8y7vcLDq//T4+nmAOTV5ujtANXi2ADo3NkAyr0A1ebZANrp4OCr/87c3ecA293a
6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAPC1hrBapEpLAPCO+AYENgwA8ID4BwQA8H34B0MBlwEg
AJABIAKQRUgDkADwc/hHBH8MAPBv+EAEgAg4QwSQAPBp+AYAPkghiAUiACM9TwDwbPhFogchAPBl
+AAgMSEA8F/4/yAjIQDwW/gCICUhAPBX+DAAAyEG3wEhAUCqIEAaDCEA8E34AiADIQDwSfgEnicn
HyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACRIJk8A8Dr4ATYELvTRIEgkTwDwM/gkTjd4HUgG
LwPTI0sA8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdwACAPSQiAAigM0CCIGUsA8BX4BgACIRhP
APAR+DAAAyEA8A34BrDwvQdIcEMHSUYYMAxwRwWQBaoGSAhLGEc4R8xwAwJxhgQIbU7GQXNgAABL
TgAAKEACAv0RBAglOwQIDSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICNHDzcLHxcz/mQERAV0A
nAAAAMBG`), {
                'BPGE 1.10': decodeBase64(`XAEBR/ADAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRrwDAbHQAw5REQQIeToECGEgBAh5G+gDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLwDAvGF0AMOeREECKE6BAiJIAQIoRvoAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSbwDAt2F0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU7wDAcXQAw5lEQQIjToECHUgBAiNG+gDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUa8AwGx0AMOUREECHk6BAhhIAQIeRvoAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUS8AwLxhdADDnkRBAihOgQIiSAECKEb6AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUm8AwLdhdADDmURBAiNOgQIdSAECI0b6AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVO8AwHF0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-10-aniv-celebi',
        label: 'Celebi (10 ANIV Event)',
        description: 'A Celebi like the one the 2006 Pokémon 10th Anniversary “Journey Across America” tour gave: level 70, OT 10 ANIV, ID 00010, from Ruby, knowing Ancient Power, Future Sight, Baton Pass and Perish Song. Its nature, IVs and OT gender are rolled the way the tour rolled them, never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`FwT7AC8AAAAEAKKhALvIw9AAvb/Gv7zD///////////////////////////////////O3NkAoqHo
3AC74uLd6tnm59Xm7QDb3dro////////////////////ztzZAL2/xr+8wwDj2gDo3NkAo6GhpwCi
oejc/////////////////7vi4t3q2ebn1ebtAOjj6eYA49oAu+HZ5t3X1a3////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbcAAAhmbWhsAr3OAAAIZm1obAK9+QAACGZtaGwCvRkBAAhmbWhsAv0BAObZ19nd6tnYAL2/
xr+8w6v/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V
4uPo3NnmAL2/xr+8w6v/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA
3efitOgA2uPm/ujc3ecA29Xh2a3/8LWGsFukS0sA8I/4BgQ2DADwgfgHBADwfvgHQzgMeEBHSUhA
AATADALRCDf/CP8AAZcBIACQASACkEFIA5AA8Gr4RwR/DADwZvhABIAIOEMEkADwYPgGADpIIYhG
IgAjOU8A8GP4QaIHIQDwXPjwQ8AJMSEA8FX4/yAjIQDwUfgCICUhAPBN+AIgAyEA8En4BJ4nJx8g
MEB2CTkAAPBB+AE3LS/20QAmcQBhGEmIMgAkSCdPAPA6+AE2BC700SFIJE8A8DP4JU43eB5IBi8D
0yNLAPAq+AvgZCF5Qx5KiRhgIoNYi1AEOvvVATc3cAAgD0kIgAIoDNAgiBpLAPAV+AYAAiEYTwDw
EfgwAAMhAPAN+Aaw8L0ISHBDCElGGDAMcEcFkAWqB0gISxhHOEfARsxwAwJxhgQIbU7GQXNgAAAK
AAAAKEACAv0RBAglOwQIDSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICKKhALvIw9D/+wD2APgA
4gDDAMBG`), {
                'BPGE 1.10': decodeBase64(`XAEBR/ADAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRrwDAbHQAw5REQQIeToECGEgBAh5G+gDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLwDAvGF0AMOeREECKE6BAiJIAQIoRvoAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSbwDAt2F0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU7wDAcXQAw5lEQQIjToECHUgBAiNG+gDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUa8AwGx0AMOUREECHk6BAhhIAQIeRvoAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUS8AwLxhdADDnkRBAihOgQIiSAECKEb6AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUm8AwLdhdADDmURBAiNOgQIdSAECI0b6AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVO8AwHF0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-10-aniv-kanto',
        label: 'Party of the Decade: Kanto (10 ANIV Event)',
        description: 'Choose one of the Kanto favorites of the 2006 Pokémon 10th Anniversary “Party of the Decade”: Bulbasaur, Charizard, Blastoise, the Pikachu that knows Fly, Alakazam or Dragonite. Each is level 70, OT 10 ANIV, ID 06808, from Ruby, with the event’s own moves, its nature, IVs and OT gender rolled the way the event rolled them, never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`GAQZADAAAAAQAMq7zM7TAMnAAM7CvwC+v727vr/////////////////////////////Fu8jOyQDa
1erj5t3o2ef/////////////////////////////////vM/GvLvNu8/MuAC9wrvMw9S7zL64////
/////////////////////7zGu83OycPNv7gAysPFu73Cz7gAu8a7xbvUu8f////////////////j
5gC+zLvBycjDzr/wANfc4+Pn2QDj4tkA4+L/////////////////o8AA49oA1QDKycUbx8nIAL2/
yM6/zK3//////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF3AAACB+vAAAIRbsF3AAACB+8AAAICrsF3AAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBvgAACBYEgAAAIxUQAAOTAiENgAAAuwHSAAAIfQAGgL1pAQAIZm4UCCENgAEAuwGF
AAAIFwSAAQC5UQAACEMjFRAAAxQBIQ2AAgC7AcgAAAgp2AMxAQG95gAACGYybSENgAEAuwG0AAAI
aGwCvfYAAAhmbWhsAr0NAQAIZm1obAK9NQEACGZtaGwCvVUBAAhmbWhsAr18AQAIZm1obAL9AQDm
2dfZ3erZ2AD9Aqv/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3i
ANrj5v7V4uPo3NnmAOPi2av/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq/+94+HZANbV
198A1eLtAOjd4dmr/9Hj6eDYAO3j6QDg3d/ZAP0CrP/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA
29Xh2a3/APC1hrBqpGdIAIgKIUhDJBhWSwDwmPgGBDYMAPCK+AcEAPCH+AdDOAx4QFNJSEAABMAM
AtEIN/8I/wABlwEgAJABIAKQTEgDkADwc/hHBH8MAPBv+EAEgAg4QwSQAPBp+AYARkghiEYiACNF
TwDwbPhNogchAPBl+PBDwAkxIQDwXvj/ICMhAPBa+AIgJSEA8Fb4AiADIQDwUvhFoCIaPCoE0lKg
ghgCIQDwS/gEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACtILk8A8Dr4ATYELvTRKEgr
TwDwM/gsTjd4JUgGLwPTKksA8Cr4C+BkIXlDJUqJGGAig1iLUAQ6+9UBNzdwACAWSQiAAigM0CCI
IUsA8BX4BgACIR9PAPAR+DAAAyEA8A34BrDwvQ9IcEMPSUYYMAxwRwWQBaoOSA9LGEc4RxZIAYgA
IgYpBdIKIlFDFaJRWoGAASIBSAKAcEfARsxwAwJxhgQIbU7GQXNgAACYGgAAKEACAv0RBAglOwQI
DSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICLxwAwKioQC7yMPQ/wEA5gBKAEwA6wAGABEAowBS
AFMACQC2APAAggA4ABkAVQBXAHEAEwBBAPgAWwFeAA8BlQBhANsAEQDIALzPxry7zbvPzP+9wrvM
w9S7zL7/vMa7zc7Jw82//8rDxbu9ws////+7xrvFu9S7x///vsy7wcnIw86//w==`), {
                'BPGE 1.10': decodeBase64(`XAEBR4wEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRlgEAbFsBA5REQQIeToECGEgBAh5G4QECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRFgEAvGFbAQOeREECKE6BAiJIAQIoRuEBAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSVgEAt2FbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU1gEAcVsBA5lEQQIjToECHUgBAiNG4QECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZYBAGxbAQOUREECHk6BAhhIAQIeRuEBAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURYBALxhWwEDnkRBAihOgQIiSAECKEbhAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlYBALdhWwEDmURBAiNOgQIdSAECI0bhAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNYBAHFbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-10-aniv-legends',
        label: 'Party of the Decade: Legends (10 ANIV Event)',
        description: 'Choose one of the Legendary Pokémon of the 2006 Pokémon 10th Anniversary “Party of the Decade”: Articuno, Zapdos, Moltres, Raikou, Entei, Suicune, Latias or Latios. Each is level 70, OT 10 ANIV, ID 06808, from Ruby, with the event’s own moves, its nature, IVs and OT gender rolled the way the event rolled them, never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`GQSQADEAAAAYAMq7zM7TAMnAAM7CvwC+v727vr/////////////////////////////G2dvZ4tjV
5u0AysnFG8fJyP//////////////////////////////u8zOw73PyMm4ANS7yr7JzbgAx8nGzsy/
zbj//////////////////8y7w8XJz7gAv8jOv8O4AM3Pw73PyL+4AMa7zsO7zf/////////////j
5gDGu87Dyc3wANfc4+Pn2QDj4tkA4+L/////////////////////o8AA49oA1QDKycUbx8nIAL2/
yM6/zK3//////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF3AAACB+vAAAIRbsF3AAACB+8AAAICrsF3AAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBvgAACBYEgAAAIxUQAAOTAiENgAAAuwHSAAAIfQAGgL1pAQAIZm4UCCENgAEAuwGF
AAAIFwSAAQC5UQAACEMjFRAAAxQBIQ2AAgC7AcgAAAgp2AMxAQG95gAACGYybSENgAEAuwG0AAAI
aGwCvfYAAAhmbWhsAr0NAQAIZm1obAK9NQEACGZtaGwCvVUBAAhmbWhsAr18AQAIZm1obAL9AQDm
2dfZ3erZ2AD9Aqv/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3i
ANrj5v7V4uPo3NnmAOPi2av/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq/+94+HZANbV
198A1eLtAOjd4dmr/9Hj6eDYAO3j6QDg3d/ZAP0CrP/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA
29Xh2a3/APC1hrBqpGdIAIgKIUhDJBhWSwDwmPgGBDYMAPCK+AcEAPCH+AdDOAx4QFNJSEAABMAM
AtEIN/8I/wABlwEgAJABIAKQTEgDkADwc/hHBH8MAPBv+EAEgAg4QwSQAPBp+AYARkghiEYiACNF
TwDwbPhNogchAPBl+PBDwAkxIQDwXvj/ICMhAPBa+AIgJSEA8Fb4AiADIQDwUvhFoCIaHioE0leg
ghgCIQDwS/gEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACtILk8A8Dr4ATYELvTRKEgr
TwDwM/gsTjd4JUgGLwPTKksA8Cr4C+BkIXlDJUqJGGAig1iLUAQ6+9UBNzdwACAWSQiAAigM0CCI
IUsA8BX4BgACIR9PAPAR+DAAAyEA8A34BrDwvQ9IcEMPSUYYMAxwRwWQBaoOSA9LGEc4RxZIAYgA
IggpBdIKIlFDFaJRWoGAASIBSAKAcEfARsxwAwJxhgQIbU7GQXNgAACYGgAAKEACAv0RBAglOwQI
DSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICLxwAwKioQC7yMPQ/5AAYQCqADoAcwCRAGEAxQBB
AAwBkgBhAMsANQDbAPMAYgDRAHMA8gD0AFMAFwA1AM8A9QAQAD4ANgDzAJcBKAFeAGkAzACYAScB
XgBpAF0Bu8zOw73PyMn//9S7yr7Jzf/////HycbOzL/N////wEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR4wEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRlgEAbFsBA5REQQIeToECGEgBAh5G4QECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRFgEAvGFbAQOeREECKE6BAiJIAQIoRuEBAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSVgEAt2FbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU1gEAcVsBA5lEQQIjToECHUgBAiNG4QECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZYBAGxbAQOUREECHk6BAhhIAQIeRuEBAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURYBALxhWwEDnkRBAihOgQIiSAECKEbhAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlYBALdhWwEDmURBAiNOgQIdSAECI0bhAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNYBAHFbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-10-aniv-johto-hoenn',
        label: 'Party of the Decade: Johto & Hoenn (10 ANIV Event)',
        description: 'Choose one of the Johto and Hoenn favorites of the 2006 Pokémon 10th Anniversary “Party of the Decade”: Typhlosion, Espeon, Umbreon, Tyranitar, Blaziken or Absol. Each is level 70, OT 10 ANIV, ID 06808, from Ruby, with the event’s own moves, its nature, IVs and OT gender rolled the way the event rolled them, never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`GgTEADIAAAAIAMq7zM7TAMnAAM7CvwC+v727vr/////////////////////////////EycLOyQAt
AMLJv8jIANrV6uPm3ejZ5///////////////////////ztPKwsbJzcPJyLgAv83Kv8nIuADPx7zM
v8nIuP///////////////87TzLvIw867zLgAvMa71MPFv8gA4+b///////////////////////+7
vM3JxvAA19zj4+fZAOPi2QDj4v//////////////////////////o8AA49oA1QDKycUbx8nIAL2/
yM6/zK3//////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF3AAACB+vAAAIRbsF3AAACB+8AAAICrsF3AAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBvgAACBYEgAAAIxUQAAOTAiENgAAAuwHSAAAIfQAGgL1pAQAIZm4UCCENgAEAuwGF
AAAIFwSAAQC5UQAACEMjFRAAAxQBIQ2AAgC7AcgAAAgp2AMxAQG95gAACGYybSENgAEAuwG0AAAI
aGwCvfYAAAhmbWhsAr0NAQAIZm1obAK9NQEACGZtaGwCvVUBAAhmbWhsAr18AQAIZm1obAL9AQDm
2dfZ3erZ2AD9Aqv/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3i
ANrj5v7V4uPo3NnmAOPi2av/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq/+94+HZANbV
198A1eLtAOjd4dmr/9Hj6eDYAO3j6QDg3d/ZAP0CrP/O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA
29Xh2a3/APC1hrBqpGdIAIgKIUhDJBhWSwDwmPgGBDYMAPCK+AcEAPCH+AdDOAx4QFNJSEAABMAM
AtEIN/8I/wABlwEgAJABIAKQTEgDkADwc/hHBH8MAPBv+EAEgAg4QwSQAPBp+AYARkghiEYiACNF
TwDwbPhNogchAPBl+PBDwAkxIQDwXvj/ICMhAPBa+AIgJSEA8Fb4AiADIQDwUvhFoCIaMioE0lKg
ghgCIQDwS/gEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACtILk8A8Dr4ATYELvTRKEgr
TwDwM/gsTjd4JUgGLwPTKksA8Cr4C+BkIXlDJUqJGGAig1iLUAQ6+9UBNzdwACAWSQiAAigM0CCI
IUsA8BX4BgACIR9PAPAR+DAAAyEA8A34BrDwvQ9IcEMPSUYYMAxwRwWQBaoOSA9LGEc4RxZIAYgA
IgYpBdIKIlFDFaJRWoGAASIBSAKAcEfARsxwAwJxhgQIbU7GQXNgAACYGgAAKEACAv0RBAglOwQI
DSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICLxwAwKioQC7yMPQ/50AYgCsAIEANQDEADwA9ABe
AOoAxQC5ANQAZwDsAPgAJQC4APIAWQAaASsBowB3AEcBeAFoAKMA+ADDAM7TysLGyc3Dyci/zcq/
ycj/////z8e8zL/JyP///87TzLvIw867zP+8xrvUw8W/yP//wEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR4wEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRlgEAbFsBA5REQQIeToECGEgBAh5G4QECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRFgEAvGFbAQOeREECKE6BAiJIAQIoRuEBAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSVgEAt2FbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU1gEAcVsBA5lEQQIjToECHUgBAiNG4QECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZYBAGxbAQOUREECHk6BAhhIAQIeRuEBAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURYBALxhWwEDnkRBAihOgQIiSAECKEbhAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlYBALdhWwEDmURBAiNOgQIdSAECI0bhAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNYBAHFbAQOZREECI06BAh1IAQIjRuEBAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-doel-deoxys',
        label: 'Deoxys (DOEL Event)',
        description: 'A Deoxys like the one the DOEL distribution gave: level 70, OT DOEL, ID 28606, from Ruby, knowing Cosmic Power, Recover, Psycho Boost and Hyper Beam, marked as met in a fateful encounter so it obeys. Like any Deoxys it takes your game’s form: Attack Forme in FireRed, Defense Forme in LeafGreen. Never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`GwSaATMAAAAYAL7Jv8YAvr/J0tPN///////////////////////////////////////A5uPhAOPp
6NnmAOfk1dfZ////////////////////////////////ztzZAL6/ydLTzQDj2gDo3NkAvsm/xv//
/////////////////////9jd5+jm3dbp6N3j4rgA5tnV2O0A6OMA49bZ7a3////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbcAAAhmbWhsAr3OAAAIZm1obAK9+QAACGZtaGwCvRkBAAhmbWhsAv0BAObZ19nd6tnYAL6/
ydLTzav/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V
4uPo3NnmAL6/ydLTzav/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA
3efitOgA2uPm/ujc3ecA29Xh2a3/8LWGsF2kTUsA8JP4BgQ2DADwhfgHBADwgvgHQzgMeEBJSUhA
AATADALRCDf/CP8AAZcBIACQASACkENIA5AA8G74RwR/DADwavhABIAIOEMEkADwZPgGADxIIYhG
IgAjO08A8Gf4Q6IHIQDwYPjwQ8AJMSEA8Fn4/yAjIQDwVfgCICUhAPBR+AIgAyEA8E34ASBQIQDw
SfgEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACRIJ08A8Dr4ATYELvTRIUgkTwDwM/gl
Tjd4HkgGLwPTI0sA8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdwACAPSQiAAigM0CCIGksA8BX4
BgACIRhPAPAR+DAAAyEA8A34BrDwvQhIcEMISUYYMAxwRwWQBaoHSAhLGEc4R8BGzHADAnGGBAht
TsZBc2AAAL5vAAAoQAIC/REECCU7BAgNIQQIJRwECIBCAgIlQAICOUMECEFqBAjZxQgIvsm/xv//
//+aAUIBaQBiAT8AwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR/gDAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRsQDAbHYAw5REQQIeToECGEgBAh5G/ADCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRMQDAvGF2AMOeREECKE6BAiJIAQIoRvwAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBScQDAt2F2AMOZREECI06BAh1IAQIjRvwAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU8QDAcXYAw5lEQQIjToECHUgBAiNG/ADCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbEAwGx2AMOUREECHk6BAhhIAQIeRvwAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTEAwLxhdgDDnkRBAihOgQIiSAECKEb8AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnEAwLdhdgDDmURBAiNOgQIdSAECI0b8AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPEAwHF2AMOZREECI06BAh1IAQIjRvwAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-space-c-deoxys',
        label: 'Deoxys (SPACE C Event)',
        description: 'A Deoxys like the one the SPACE C distribution gave: level 70, OT SPACE C, ID 00010, from Ruby, knowing Cosmic Power, Recover, Psycho Boost and Hyper Beam, marked as met in a fateful encounter so it obeys. Like any Deoxys it takes your game’s form: Attack Forme in FireRed, Defense Forme in LeafGreen. Never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`HASaATQAAAAUAM3Ku72/AL0Avr/J0tPN///////////////////////////////////A5uPhAOPp
6NnmAOfk1dfZ////////////////////////////////ztzZAL6/ydLTzQDj2gDo3NkAzcq7vb8A
vf///////////////////9jd5+jm3dbp6N3j4rgA5tnV2O0A6OMA49bZ7a3////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbcAAAhmbWhsAr3OAAAIZm1obAK9+QAACGZtaGwCvRkBAAhmbWhsAv0BAObZ19nd6tnYAL6/
ydLTzav/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V
4uPo3NnmAL6/ydLTzav/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA
3efitOgA2uPm/ujc3ecA29Xh2a3/8LWGsF2kTUsA8JP4BgQ2DADwhfgHBADwgvgHQzgMeEBJSUhA
AATADALRCDf/CP8AAZcBIACQASACkENIA5AA8G74RwR/DADwavhABIAIOEMEkADwZPgGADxIIYhG
IgAjO08A8Gf4Q6IHIQDwYPjwQ8AJMSEA8Fn4/yAjIQDwVfgCICUhAPBR+AIgAyEA8E34ASBQIQDw
SfgEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACRIJ08A8Dr4ATYELvTRIUgkTwDwM/gl
Tjd4HkgGLwPTI0sA8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdwACAPSQiAAigM0CCIGksA8BX4
BgACIRhPAPAR+DAAAyEA8A34BrDwvQhIcEMISUYYMAxwRwWQBaoHSAhLGEc4R8BGzHADAnGGBAht
TsZBc2AAAAoAAAAoQAIC/REECCU7BAgNIQQIJRwECIBCAgIlQAICOUMECEFqBAjZxQgIzcq7vb8A
vf+aAUIBaQBiAT8AwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR/gDAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRsQDAbHYAw5REQQIeToECGEgBAh5G/ADCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRMQDAvGF2AMOeREECKE6BAiJIAQIoRvwAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBScQDAt2F2AMOZREECI06BAh1IAQIjRvwAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU8QDAcXYAw5lEQQIjToECHUgBAiNG/ADCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbEAwGx2AMOUREECHk6BAhhIAQIeRvwAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTEAwLxhdgDDnkRBAihOgQIiSAECKEb8AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnEAwLdhdgDDmURBAiNOgQIdSAECI0b8AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPEAwHF2AMOZREECI06BAh1IAQIjRvwAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-aura-mew',
        label: 'Mew (Aura Event)',
        description: 'A Mew like the one the Aura distribution gave: level 10, OT Aura, ID 20078, from Ruby, knowing Pound and Transform, marked as met in a fateful encounter so it obeys. Never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`HQSXADUAAAAMALvPzLsAx7/R///////////////////////////////////////////O3NkAu+nm
1QDb3dro////////////////////////////////////ztzZAMe/0QDj2gDo3NkAu+nm1f//////
/////////////////////9jd5+jm3dbp6N3j4rgA5tnV2O0A6OMA49bZ7a3////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+UAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbQAAAhmbWhsAr3LAAAIZm1obAK98wAACGZtaGwCvRMBAAhmbWhsAv0BAObZ19nd6tnYAMe/
0av/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V4uPo
3NnmAMe/0av/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA3efitOgA
2uPm/ujc3ecA29Xh2a3/AADwtYawW6RLSwDwj/gGBDYMAPCB+AcEAPB++AdDOAx4QEdJSEAABMAM
AtEIN/8I/wABlwEgAJABIAKQQUgDkADwavhHBH8MAPBm+EAEgAg4QwSQAPBg+AYAOkghiAoiACM5
TwDwY/hBogchAPBc+PBDwAkxIQDwVfj/ICMhAPBR+AIgJSEA8E34ASBQIQDwSfgEnicnHyAwQHYJ
OQAA8EH4ATctL/bRACZxAGEYSYgyACRIJ08A8Dr4ATYELvTRIUgkTwDwM/glTjd4HkgGLwPTI0sA
8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdwACAPSQiAAigM0CCIGksA8BX4BgACIRhPAPAR+DAA
AyEA8A34BrDwvQhIcEMISUYYMAxwRwWQBaoHSAhLGEc4R8BGzHADAnGGBAhtTsZBc2AAAG5OAAAo
QAIC/REECCU7BAgNIQQIJRwECIBCAgIlQAICOUMECEFqBAjZxQgIu+nm1f////+XAAEAkAAAAAAA
wEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR+wDAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRrgDAbHMAw5REQQIeToECGEgBAh5G+QDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLgDAvGFzAMOeREECKE6BAiJIAQIoRvkAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSbgDAt2FzAMOZREECI06BAh1IAQIjRvkAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU7gDAcXMAw5lEQQIjToECHUgBAiNG+QDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUa4AwGxzAMOUREECHk6BAhhIAQIeRvkAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUS4AwLxhcwDDnkRBAihOgQIiSAECKEb5AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUm4AwLdhcwDDmURBAiNOgQIdSAECI0b5AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVO4AwHFzAMOZREECI06BAh1IAQIjRvkAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-mystry-mew',
        label: 'Mew (MYSTRY Event)',
        description: 'A Mew like the one the MYSTRY distribution gave: level 10, OT MYSTRY, ID 06930, from Ruby, knowing Pound and Transform, marked as met in a fateful encounter so it obeys. It comes from one of the 86 seeds the real ones came from, as PKHeX expects. Never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`HgSXADYAAAAEAMfTzc7M0wDHv9H////////////////////////////////////////O3NkAx9PN
zszTANvd2uj/////////////////////////////////ztzZAMe/0QDj2gDo3NkAx9PNzszT////
/////////////////////9jd5+jm3dbp6N3j4rgA5tnV2O0A6OMA49bZ7a3////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+UAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbQAAAhmbWhsAr3LAAAIZm1obAK98wAACGZtaGwCvRMBAAhmbWhsAv0BAObZ19nd6tnYAMe/
0av/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V4uPo
3NnmAMe/0av/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA3efitOgA
2uPm/ujc3ecA29Xh2a3/AADwtYawaKRXSwDwqPgGBDYMMABWIQbfSQBmoEBasgeSDwEyXkmIQgDR
AiIFIUpDBgAA8Ir4ATr70QDwhvgHBADwg/gHQzgMeEBJSUhAAATADALRCDf/CP8AAZcBIACQASAC
kENIA5AA8G/4RwR/DADwa/hABIAIOEMEkADwZfgGADxIIYgKIgAjO08A8Gj4RKIHIQDwYfgwAAMh
Bt8xIQDwWfj/ICMhAPBV+AIgJSEA8FH4AiADIQDwTfgBIFAhAPBJ+ASeJycfIDBAdgk5AADwQfgB
Ny0v9tEAJnEAYRhJiDIAJEgmTwDwOvgBNgQu9NEgSCRPAPAz+CRON3gdSAYvA9MjSwDwKvgL4GQh
eUMeSokYYCKDWItQBDr71QE3N3AAIA9JCIACKAzQIIgZSwDwFfgGAAIhGE8A8BH4MAADIQDwDfgG
sPC9B0hwQwdJRhgwDHBHBZAFqgZICEsYRzhHzHADAnGGBAhtTsZBc2AAABIbAAAoQAIC/REECCU7
BAgNIQQIJRwECIBCAgIlQAICOUMECEFqBAjZxQgIZWAAAMfTzc7M0///lwABAJAAAAAAAMBGUgYy
CRMMQw3uDmMSyRMUFgkcpR6/IIkjOSktMG4w8zTzRc5GDUpjS3lMjlCrUEBSJ1O6VsxWQVhgWsFb
K17zXmVgP2RXZKNnRGkGbmJuZ3bvd9J4VYaSikiL0JMdlKCVfZaQljecQJycneSdhp5ToUOkrKgI
rPuv8rExuJa+1MKFw87GLMlTyWLJQ8xHzZbN5NHt3yzmzOYK6V3pkemy63/un+7I7+TwTv6d/g==`), {
                'BPGE 1.10': decodeBase64(`XAEBRxwEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRugDAbH8Aw5REQQIeToECGEgBAh5GxQECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBROgDAvGF/AMOeREECKE6BAiJIAQIoRsUBAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSegDAt2F/AMOZREECI06BAh1IAQIjRsUBAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU+gDAcX8Aw5lEQQIjToECHUgBAiNGxQECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUboAwGx/AMOUREECHk6BAhhIAQIeRsUBAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUToAwLxhfwDDnkRBAihOgQIiSAECKEbFAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnoAwLdhfwDDmURBAiNOgQIdSAECI0bFAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPoAwHF/AMOZREECI06BAh1IAQIjRsUBAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-rocks-metang',
        label: 'Metang (ROCKS Event)',
        description: 'A Metang like the one the ROCKS distribution gave: level 30, OT ROCKS, ID 02005, from Ruby, knowing Take Down, Confusion, Metal Claw and Refresh, with the National Ribbon. Never shiny. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`HwSPATcAAAAUAMzJvcXNAMe/zrvIwf/////////////////////////////////////R3ejcAOjc
2QDI1ejd4+LV4ADM3dbW4+L/////////////////////ztzZAMe/zrvIwQDj2gDo3NkAzMm9xc3/
/////////////////////9jd5+jm3dbp6N3j4rgA693o3ADd6OcA5t3W1uPirf/////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbcAAAhmbWhsAr3OAAAIZm1obAK9+QAACGZtaGwCvRkBAAhmbWhsAv0BAObZ19nd6tnYAMe/
zrvIwav/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V
4uPo3NnmAMe/zrvIwav/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA
3efitOgA2uPm/ujc3ecA29Xh2a3/8LWGsFukS0sA8I/4BgQ2DADwgfgHBADwfvgHQzgMeEBHSUhA
AATADALRCDf/CP8AAZcBIACQASACkEFIA5AA8Gr4RwR/DADwZvhABIAIOEMEkDxIIYgeIgAjO08A
8Gb4QqIHIQDwX/gAIDEhAPBZ+P8gIyEA8FX4AiAlIQDwUfgCIAMhAPBN+AEgTCEA8En4BJ4nJx8g
MEB2CTkAAPBB+AE3LS/20QAmcQBhGEmIMgAkSCdPAPA6+AE2BC700SFIJE8A8DP4JU43eB5IBi8D
0yNLAPAq+AvgZCF5Qx5KiRhgIoNYi1AEOvvVATc3cAAgD0kIgAIoDNAgiBpLAPAV+AYAAiEYTwDw
EfgwAAMhAPAN+Aaw8L0ISHBDCElGGDAMcEcFkAWqB0gISxhHOEfARsxwAwJxhgQIbU7GQXNgAADV
BwAAKEACAv0RBAglOwQIDSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICMzJvcXN////jwEkAF0A
6AAfAcBG`), {
                'BPGE 1.10': decodeBase64(`XAEBR/ADAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRrwDAbHQAw5REQQIeToECGEgBAh5G+gDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLwDAvGF0AMOeREECKE6BAiJIAQIoRvoAwq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSbwDAt2F0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU7wDAcXQAw5lEQQIjToECHUgBAiNG+gDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUa8AwGx0AMOUREECHk6BAhhIAQIeRvoAwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUS8AwLxhdADDnkRBAihOgQIiSAECKEb6AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUm8AwLdhdADDmURBAiNOgQIdSAECI0b6AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVO8AwHF0AMOZREECI06BAh1IAQIjRvoAwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-channel-jirachi',
        label: 'Jirachi (Pokémon Channel Event)',
        description: 'The Jirachi that Pokémon Channel gave in Europe, as it came: OT CHANNEL, ID 40122, level 5, holding a Ganlon or Salac Berry, with Wish, Confusion and Rest, and its trainer’s secret ID, game and gender from the same random numbers, the way PKHeX checks them. It can be shiny. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`LQSZAUUAAAAIAL3Cu8jIv8YAxMPMu73Cw//////////////////////////////////O3NkAysnF
G8fJyAC9wrvIyL/GANvd2uj/////////////////////ztzZAMTDzLu9wsMA6NzV6ADKycUbx8nI
/////////////////////73Cu8jIv8YA29Xq2QDd4gC/6ebj5Nmt///////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+0AIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbgAAAhmbWhsAr3PAAAIZm1obAK9+wAACGZtaGwCvRsBAAhmbWhsAv0BAObZ19nd6tnYAMTD
zLu9wsOr/8PoAOvV5wDn2eLoAOjjAOjc2QDKva3/zNnX2d3q2QDo3NkA19Xm2ADV29Xd4gDa4+b+
1eLj6NzZ5gDEw8y7vcLDq//T4+nmAOTV5ujtANXi2ADo3NkAyr0A1ebZANrp4OCr/87c3ecA293a
6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAPC1hrB9pGxLAPDS+AYENgxpSwDwzfgABAZDACUA8L74
sA8BIYFADUMOLffTAPC2+ADwtPgA8LL4APCw+ADwrvgBIYkDiEIG2QDwqPhdSYhCAdkA8KP4APCh
+ADwn/gFAADwnPgHBADwmfgHQwEhCCgA0wAhOAxoQFNKUECIQgLQASDAB0dALQQA8Ij4wAsFQwDw
hPjAC0AABUMA8H/4wAuAAAVDACIAIwDwePjACphAAkMFMx4r99EEkgGXASAAkAEgApAoDAAEP0kI
QwOQP0ghiAUiACM+TwDwa/hFogchAPBk+KgIMSEA8F74/yAjIQDwWvhoCAEhCEABMCUhAPBT+AEg
KECpMAwhAPBN+AAgJCEA8En4BJ4nJx8gMEB2CTkAAPBB+AE3LS/20QAmcQBhGEmIMgAlSCdPAPA6
+AE2BC700SFIJU8A8DP4JU43eB5IBi8D0yRLAPAq+AvgZCF5Qx9KiRhgIoNYi1AEOvvVATc3cAAg
D0kIgAIoDNAgiBpLAPAV+AYAAiEZTwDwEfgwAAMhAPAN+Aaw8L0HSHBDB0lGGDAMcEcFkAWqB0gJ
SxhHOEfMcAMCcYYECP1DAwDDniYAelQAALqcAAAoQAIC/REECCU7BAgNIQQIJRwECIBCAgIlQAIC
OUMECEFqBAjZxQgIvcK7yMi/xv+ZAREBXQCcAAAAwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR3wEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRkQEAbFcBA5REQQIeToECGEgBAh5G3QECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBREQEAvGFXAQOeREECKE6BAiJIAQIoRt0BAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSUQEAt2FXAQOZREECI06BAh1IAQIjRt0BAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU0QEAcVcBA5lEQQIjToECHUgBAiNG3QECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZEBAGxXAQOUREECHk6BAhhIAQIeRt0BAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUREBALxhVwEDnkRBAihOgQIiSAECKEbdAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlEBALdhVwEDmURBAiNOgQIdSAECI0bdAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNEBAHFXAQOZREECI06BAh1IAQIjRt0BAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-box-eggs',
        label: 'Pokémon Box Eggs (Special Moves)',
        description: 'One of the Eggs Pokémon Box Ruby & Sapphire gave: Swablu with False Swipe, Zigzagoon with ExtremeSpeed, Skitty with Pay Day or Pichu with Surf. The deliveryman offers them in turn. It hatches as yours, as those did. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`LgScAUYAAAAQAMrJxRvHycgAvMnSAL/Bwc3///////////////////////////////+/wcHNAOvd
6NwA5+TZ193V4ADh4+rZ5///////////////////////zdG7vMbPuADUw8HUu8HJyci4AM3Fw87O
0wDj5v///////////////8rDvcLPAOvd6NwA1QDn5NnX3dXgAOHj6tnw///////////////////X
3OPj59kA4+LZAOPiAKPAAOPaANX/////////////////////////ysnFG8fJyAC9v8jOv8yt////
/////////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsF3AAACB+vAAAIRbsF3AAACB+8AAAICrsF3AAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBvgAACBYEgAAAIxUQAAOFAiENgAAAuwHSAAAIfQAGgL1tAQAIZm4UCCENgAEAuwGF
AAAIFwSAAQC5UQAACEMjFRAAAyABIQ2AAgC7AcgAAAgp2AMxAQG95gAACGYybSENgAEAuwG0AAAI
aGwCvfoAAAhmbWhsAr0RAQAIZm1obAK9OQEACGZtaGwCvVkBAAhmbWhsAr2GAQAIZm1obAL9AQDm
2dfZ3erZ2ADV4gC/wcGr/8PoAOvV5wDn2eLoAOjjAOjc2QDKva3/zNnX2d3q2QDo3NkA19Xm2ADV
29Xd4gDa4+b+1eLj6NzZ5gC/wcGr/9Pj6eYA5NXm6O0A1eLYAOjc2QDKvQDV5tkA2ung4Kv/vePh
2QDW1dffANXi7QDo3eHZq//R4+ng2ADt4+kA4N3f2QDVAP0C/r/Bwaz/ztzd5wDb3droAN3n4rTo
ANrj5v7o3N3nANvV4dmt/wAAAPC1hrBkpGFIAIgKIUhDJBhPSwDwi/gGBDYMTUsA8Ib4AAQGQwDw
ePgHBADwdfgHQwGXASAAkAAgApBJSAOQAPBr+EcEfwwA8Gf4QASACDhDBJBESCGIBSIAI0NPAPBn
+EuiByEA8GD4ASAxIQDwWvj/ICMhAPBW+AIgACAkIQDwUfgBIAMhAPBN+E2iAiEA8Ev4IIgcIUhD
MElAGEB8ICEA8ED4ASAtIQDwPPgEnicnHyAwQHYJOQAA8DT4ATctL/bRACZxAGEYSYgyACVIKE8A
8C34ATYELvTRIkglTwDwJvgmTjd4H0gGLwPTJEsA8B34C+BkIXlDH0qJGGAig1iLUAQ6+9UBNzdw
ACAPSQiAAij/0Aaw8L0OSHBDDklGGDAMcEcFkAWqDkgQSxhHOEcWSAGIACIEKQXSCiJRQxaiUVqB
gAEiAUgCgHBHzHADAnGGBAhtTsZBc2AAAFAjJQgAAAAAKEACAv0RBAglOwQIDSEECCUcBAiAQgIC
JUACAjlDBAhBagQI2cUICLxwAwK71M/Nu////2YBQAAtAM4AAAAgASEALQAnAPUAOwEtACEAJwAG
AKwAVADMADkAAABgb4v/////////wEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBR1QEASyABAGt`),
                'BPRF 1.10': decodeBase64(`aAEBRkgEAbFUBAP8zSRgBA5REQQIeToECGEgBAh5G3gECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBREgEAvGFVAQC0ChgBA55EQQIoToECIkgBAihG3gECrVCBAi9aQQIocc=`),
                'BPRI 1.10': decodeBase64(`aAEBSUgEAt2FVAQDjLokYAQOZREECI06BAh1IAQIjRt4BAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU0gEAcVUBAN04SRgBA5lEQQIjToECHUgBAiNG3gECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUZIBAGxVAQD2M0kYAQOUREECHk6BAhhIAQIeRt4BAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAURIBALxhVQEAqwoYAQOeREECKE6BAiJIAQIoRt4BAq1QgQIvWkECHXH`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUlIBALdhVQEA2i6JGAEDmURBAiNOgQIdSAECI0beAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVNIBAHFVQQC4SRgBA5lEQQIjToECHUgBAiNG3gECqFCBAipaQQIScg=`),
            }),
        },
    },
    {
        id: 'custom-colosseum-pikachu',
        label: 'Pikachu (Japanese Colosseum Bonus Disc)',
        description: 'The Pikachu that the Japanese Pokémon Colosseum bonus disc gave: a Japanese Pikachu, OT コロシアム, ID 31121, level 10, never shiny, made the way Colosseum made it. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`LwQZAEcAAAAUAL3JxsnNzb/PxwDKw8W7vcLP///////////////////////////////A5uPhAMTV
5NXitOcAvMnIz80AvsPNvf//////////////////////ztzZAMrDxbu9ws8A49oA6NzZAMTV5NXi
2efZ/////////////////73JxsnNzb/PxwC8ycjPzQC+w829rf/////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+0AIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbgAAAhmbWhsAr3PAAAIZm1obAK9+wAACGZtaGwCvRsBAAhmbWhsAv0BAObZ19nd6tnYAMrD
xbu9ws+r/8PoAOvV5wDn2eLoAOjjAOjc2QDKva3/zNnX2d3q2QDo3NkA19Xm2ADV29Xd4gDa4+b+
1eLj6NzZ5gDKw8W7vcLPq//T4+nmAOTV5ujtANXi2ADo3NkAyr0A1ebZANrp4OCr/87c3ecA293a
6ADd5+K06ADa4+b+6Nzd5wDb1eHZrf8AAPC1hrBgpFBLAPCa+AYENgxNSwDwlfgABAZDNQAuAADw
hfhHBH8MAPCB+EAEgAgHQwSXAPB7+ADwefgHBADwdvgHQzgMeEBDSUhAAATADATRLgAA8Gv4NQDg
5wGXASAAkAEgApA7SAOQO0ghiAoiACM6TwDwZvhCogchAPBf+AAgMSEA8Fn4/yAjIQDwVfgCICUh
APBR+AEgAyEA8E34PaICIQDwS/gEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACRIJk8A
8Dr4ATYELvTRIEgkTwDwM/gkTjd4HUgGLwPTI0sA8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdw
ACAPSQiAAigM0CCIGUsA8BX4BgACIRhPAPAR+DAAAyEA8A34BrDwvQdIcEMHSUYYMAxwRwWQBaoG
SAhLGEc4R8xwAwJxhgQI/UMDAMOeJgCReQAAKEACAv0RBAglOwQIDSEECCUcBAiAQgICJUACAjlD
BAhBagQI2cUICFp7XFFx/wAAGQBUAC0AJwBWAMBGnFZhhVP/AAAAAMBG`), {
                'BPGE 1.10': decodeBase64(`XAEBRwgEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRtQDAbHoAw5REQQIeToECGEgBAh5GwAECo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRNQDAvGF6AMOeREECKE6BAiJIAQIoRsABAq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSdQDAt2F6AMOZREECI06BAh1IAQIjRsABAqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU9QDAcXoAw5lEQQIjToECHUgBAiNGwAECqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbUAwGx6AMOUREECHk6BAhhIAQIeRsABAqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTUAwLxhegDDnkRBAihOgQIiSAECKEbAAQKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnUAwLdhegDDmURBAiNOgQIdSAECI0bAAQKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPUAwHF6AMOZREECI06BAh1IAQIjRsABAqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-ageto-celebi',
        label: 'Celebi (Japanese Colosseum Bonus Disc)',
        description: 'The Ageto Celebi of the Japanese Pokémon Colosseum bonus disc: a Japanese Celebi, OT アゲト, ID 31121, level 10, never shiny, made the way Colosseum made it. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`MAT7AEgAAAAMALvBv87JAL2/xr+8w//////////////////////////////////////A5uPhAMTV
5NXitOcAvMnIz80AvsPNvf//////////////////////ztzZAL2/xr+8wwDj2gDo3NkAxNXk1eLZ
59n//////////////////73JxsnNzb/PxwC8ycjPzQC+w829rf/////////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbcAAAhmbWhsAr3OAAAIZm1obAK9+QAACGZtaGwCvRkBAAhmbWhsAv0BAObZ19nd6tnYAL2/
xr+8w6v/w+gA69XnAOfZ4ugA6OMA6NzZAMq9rf/M2dfZ3erZAOjc2QDX1ebYANXb1d3iANrj5v7V
4uPo3NnmAL2/xr+8w6v/0+Pp5gDk1ebo7QDV4tgA6NzZAMq9ANXm2QDa6eDgq//O3N3nANvd2ugA
3efitOgA2uPm/ujc3ecA29Xh2a3/8LWGsGCkUEsA8Jr4BgQ2DE1LAPCV+AAEBkM1AC4AAPCF+EcE
fwwA8IH4QASACAdDBJcA8Hv4APB5+AcEAPB2+AdDOAx4QENJSEAABMAMBNEuAADwa/g1AODnAZcB
IACQASACkDtIA5A7SCGICiIAIzpPAPBm+EKiByEA8F/4ASAxIQDwWfj/ICMhAPBV+AIgJSEA8FH4
ASADIQDwTfg9ogIhAPBL+ASeJycfIDBAdgk5AADwQfgBNy0v9tEAJnEAYRhJiDIAJEgmTwDwOvgB
NgQu9NEgSCRPAPAz+CRON3gdSAYvA9MjSwDwKvgL4GQheUMeSokYYCKDWItQBDr71QE3N3AAIA9J
CIACKAzQIIgZSwDwFfgGAAIhGE8A8BH4MAADIQDwDfgGsPC9B0hwQwdJRhgwDHBHBZAFqgZICEsY
RzhHzHADAnGGBAj9QwMAw54mAJF5AAAoQAIC/REECCU7BAgNIQQIJRwECIBCAgIlQAICOUMECEFq
BAjZxQgIUYpk/wAAAAD7AF0AaQDXANsAwEZeepeA/wAAAAAAwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBRwQEAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRtADAbHkAw5REQQIeToECGEgBAh5G/wDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRNADAvGF5AMOeREECKE6BAiJIAQIoRv8Awq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBSdADAt2F5AMOZREECI06BAh1IAQIjRv8AwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU9ADAcXkAw5lEQQIjToECHUgBAiNG/wDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbQAwGx5AMOUREECHk6BAhhIAQIeRv8AwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTQAwLxheQDDnkRBAihOgQIiSAECKEb/AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnQAwLdheQDDmURBAiNOgQIdSAECI0b/AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPQAwHF5AMOZREECI06BAh1IAQIjRv8AwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-mattle-ho-oh',
        label: 'Ho-Oh (Colosseum Mt. Battle Prize)',
        description: 'The Ho-Oh Pokémon Colosseum gives for beating all 100 trainers of Mt. Battle: OT MATTLE, ID 10048, level 70, with Recover, Fire Blast, Sunny Day and Swift, never shiny, made the way Colosseum made it. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`MQT6AEkAAAAcAMe7zs7GvwDCya7Jwv/////////////////////////////////////O3NkAx86t
ALy7zs7GvwDk5t3u2f//////////////////////////ztzZAMLJrsnCAL3JxsnNzb/PxwDb1erZ
ANrj5v///////////////+vd4uLd4tsAoqGhAMfOrQC8u87Oxr8A2t3b3Ojnrf/////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmQAACB+vAAAIRbsFmQAACB+8AAAICrsFmQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhQAACEMjFRAAA+kAIQ2AAgC7AY8AAAgp2AMxAQG9owAACGYybSENgAEAuwF7AAAI
aGwCvbYAAAhmbWhsAr3NAAAIZm1obAK99wAACGZtaGwCvRcBAAhmbWhsAv0BAObZ19nd6tnYAMLJ
rsnCq//D6ADr1ecA59ni6ADo4wDo3NkAyr2t/8zZ19nd6tkA6NzZANfV5tgA1dvV3eIA2uPm/tXi
4+jc2eYAwsmuycKr/9Pj6eYA5NXm6O0A1eLYAOjc2QDKvQDV5tkA2ung4Kv/ztzd5wDb3droAN3n
4rToANrj5v7o3N3nANvV4dmt/wAA8LWGsF6kTksA8Jb4BgQ2DEtLAPCR+AAEBkM1AC4AAPCB+EcE
fwwA8H34QASACAdDBJcA8Hf4APB1+AcEAPBy+AdDOAx4QEFJSEAABMAMBNEuAADwZ/g1AODnAZcB
IACQASACkDlIA5A5SCGIRiIAIzhPAPBi+ECiByEA8Fv4ACAxIQDwVfj/ICMhAPBR+AEgJSEA8E34
AiADIQDwSfgEnicnHyAwQHYJOQAA8EH4ATctL/bRACZxAGEYSYgyACRIJk8A8Dr4ATYELvTRIEgk
TwDwM/gkTjd4HUgGLwPTI0sA8Cr4C+BkIXlDHkqJGGAig1iLUAQ6+9UBNzdwACAPSQiAAigM0CCI
GUsA8BX4BgACIRhPAPAR+DAAAyEA8A34BrDwvQdIcEMHSUYYMAxwRwWQBaoGSAhLGEc4R8xwAwJx
hgQI/UMDAMOeJgBAJwAAKEACAv0RBAglOwQIDSEECCUcBAiAQgICJUACAjlDBAhBagQI2cUICMe7
zs7Gv///+gBpAH4A8QCBAMBG`), {
                'BPGE 1.10': decodeBase64(`XAEBR/wDAa0=`),
                'BPRF 1.10': decodeBase64(`aAEBRsgDAbHcAw5REQQIeToECGEgBAh5G/QDCo1CBAiVaQQIYcg=`),
                'BPRD 1.10': decodeBase64(`aAEBRMgDAvGF3AMOeREECKE6BAiJIAQIoRv0Awq1QgQIvWkECKHH`),
                'BPRI 1.10': decodeBase64(`aAEBScgDAt2F3AMOZREECI06BAh1IAQIjRv0AwqhQgQIqWkECI3H`),
                'BPRS 1.10': decodeBase64(`aAEBU8gDAcXcAw5lEQQIjToECHUgBAiNG/QDCqFCBAipaQQIdcg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbIAwGx3AMOUREECHk6BAhhIAQIeRv0AwqNQgQIlWkECDXI`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTIAwLxhdwDDnkRBAihOgQIiSAECKEb9AMKtUIECL1pBAh1xw==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnIAwLdhdwDDmURBAiNOgQIdSAECI0b9AMKoUIECKlpBAhhxw==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPIAwHF3AMOZREECI06BAh1IAQIjRv0AwqhQgQIqWkECEnI`),
            }),
        },
    },
    {
        id: 'custom-starter-egg',
        label: 'Starter Egg (Random First Partner)',
        description: 'An Egg with one of the nine first partners of Kanto, Johto and Hoenn inside, picked at random: Bulbasaur, Charmander, Squirtle, Chikorita, Cyndaquil, Totodile, Treecko, Torchic or Mudkip. It hatches like any Egg, with you as its trainer. Not an official event. It joins your party, or goes to the PC when the party is full. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`JAScATwAAAAQAM3Ou8zOv8wAv8HB///////////////////////////////////////R3N3X3ADj
4tkA693g4ADc1ejX3Kz/////////////////////////u+IAv8HBAOvd6NwA4+LZAOPaAOjc2QDi
3eLZ/////////////////9rd5ufoAOTV5uji2ebnAN3i593Y2a0A0N3n3ej////////////////o
3NkA2Nng3erZ5u3h1eIA4+IA6NzZAKPi2P//////////////////2uDj4+YA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFmwAACB+vAAAIRbsFmwAACB+8AAAICrsFmwAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARyvYA7sBhwAACCMVEAAD6gB6BIAhDYACALsBkQAACCnYAzEBAb2lAAAIZjJtIQ2AAQC7AX0A
AAhobAK9uQAACGZtaGwCvdAAAAhmbWhsAr34AAAIZm1obAK9GAEACGZtaGwC/QEA5tnX2d3q2dgA
1eIAv8HBq//D6ADr1ecA59ni6ADo4wDo3NkAyr2t/8zZ19nd6tkA6NzZANfV5tgA1dvV3eIA2uPm
/tXi4+jc2eYAv8HBq//T4+nmAOTV5ujtANXi2ADo3NkAyr0A1ebZANrp4OCr/87c3ecA293a6ADd
5+K06ADa4+b+6Nzd5wDb1eHZrf8AALUGSwDwCPgJIQbfSQAFoEBaA0kIgAC9GEfARnGGBAi8cAMC
AQAEAAcAmACbAJ4AFQEYARsBwEY=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRqQCAbE=`),
                'BPRD 1.10': decodeBase64(`aAEBRKQCAvGF`),
                'BPRI 1.10': decodeBase64(`aAEBSaQCAt2F`),
                'BPRS 1.10': decodeBase64(`aAEBU6QCAcU=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUakAgGx`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSkAgLxhQ==`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmkAgLdhQ==`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOkAgHF`),
            }),
        },
    },
    {
        id: 'custom-gift-box',
        label: 'Gift Box (Money, Rare Candies, Coins)',
        description: '¥100,000, 99 Rare Candies when your bag has room, and 1,000 Coins when you have the Coin Case and room for them. Money stops at ¥999,999 and Coins at 9,999, as in the game. One per card; receive the card again for another.',
        payloads: {
            ...romPayloads(decodeBase64(`JQRxAD0AAAAIAMHDwM4AvMnS///////////////////////////////////////////H4+LZ7bgA
19Xi2O0A1eLYAOHj5tn/////////////////////////t6KhobihoaG4AKqqAMy7zL8AvbvIvsO/
zQDV4tj//////////////6K4oaGhAL3Jw8jNrQDQ3efd6ADo3Nn////////////////////////Y
2eDd6tnm7eHV4gDj4gDo3NkAo+LYANrg4+Pm////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFogAACB+vAAAIRbsFogAACB+8AAAICrsFogAACCvYA7sBmAAACCnYA5CghgEAADEB
Ab2sAAAIZjJtRkQAYwAhDYAAALsBjAAACEREAGMAvcIAAAhmbUcEAQEAIQ2AAAC7AYkAAAi06AMh
DYAAALsFiQAACL0QAQAIZm1obAK93wAACGZtuWQAAAi9KQEACGZtaGwCvVYBAAhmbWhsAv0BAObZ
19nd6tnYALeioaG4oaGhq//9AQDm2dfZ3erZ2ACqqgDMu8y//r27yL7Dv82r/87c2ebZtOcA4uMA
5uPj4QDd4gDt4+nmANbV2/7a4+YAqqoAzLvMvwC9u8i+w7/Nq//9AQDm2dfZ3erZ2ACiuKGhoQC9
ycPIzav/zNnX2d3q2QDo3NkA19Xm2ADV29Xd4gDa4+b+1eLj6NzZ5gDBw8DOALzJ0qv/ztzd5wDb
3droAN3n4rToANrj5v7o3N3nANvV4dmt/w==`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRg==`),
                'BPRD 1.10': decodeBase64(`aAEBRA==`),
                'BPRI 1.10': decodeBase64(`aAEBSQ==`),
                'BPRS 1.10': decodeBase64(`aAEBUw==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM=`),
            }),
        },
    },
    {
        id: 'custom-rare-berries',
        label: 'Rare Event Berries',
        description: 'Gives an Enigma, a Lansat and a Starf Berry, which Gen 3 only ever handed out at events. One set per card; receive the card again for another. Without the e-Reader’s berry data the Enigma Berry has no effect in battle.',
        payloads: {
            ...romPayloads(decodeBase64(`CQQgASEAAAAUAMy7zL8AvL/MzMO/zf////////////////////////////////////+/yMPBx7u4
AMa7yM27zgDV4tgAzc67zMD/////////////////////ztzm2dkAvL/MzMO/zQDo3NXoAOvZ5tkA
4+Lg7f///////////////9nq2eYA293q2eIA4+noANXoANnq2eLo563////////////////////Q
3efd6ADo3NkA2Nng3erZ5u3h1eIA4+IAo8D/////////////////49oA1QDKycUbx8nIAL2/yM6/
zK3//////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFlAAACB+vAAAIRbsFlAAACB+8AAAICrsFlAAACCvYA7sBgAAACEavAAEAIQ2AAAC7
AYoAAAhGrQABACENgAAAuwGKAAAIRq4AAQAhDYAAALsBigAACESvAAEARK0AAQBErgABACnYA72e
AAAIZm1obAK90gAACGZtaGwCvfwAAAhmbWhsAr0iAQAIZm1obALC2ebZAO3j6QDb4/AA1eIAv8jD
wce7uADV/sa7yM27zgDV4tgA1QDNzrvMwAC8v8zM06v/zNnX2d3q2QDo3N3nANfV5tgA1dvV3eIA
2uPm/uHj5tkAvL/MzMO/zav/ztzZ5tm05wDi4wDm4+PhANrj5gDo3NnhAN3i/u3j6eYAvLvBq//O
3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRg==`),
                'BPRD 1.10': decodeBase64(`aAEBRA==`),
                'BPRI 1.10': decodeBase64(`aAEBSQ==`),
                'BPRS 1.10': decodeBase64(`aAEBUw==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUY=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUQ=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUk=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVM=`),
            }),
        },
    },
    {
        id: 'custom-gift-ribbons',
        label: 'Event Ribbons for Your Party',
        description: 'Gives every Pokémon in your party the seven gift ribbons, Marine, Land, Sky, Country, National, Earth and World, which Gen 3 only handed out at events. A ribbon whose caption came from a real event keeps it.',
        payloads: {
            ...romPayloads(decodeBase64(`DgRmASYAAAAAAMHDwM4AzMO8vMnIzf/////////////////////////////////////N2erZ4gDM
w7y8ycjNAOjjAOfc4+sA49ra////////////////////0+Pp5gDk1ebo7QDb2ejnAOjc2QDb3dro
/////////////////////8zDvLzJyM0A4+LX2QDj4uDtANvd6tniANXo///////////////////Z
6tni6OetANDd593oAOjc2QDY2eDd6tnm7eHV4v//////////////4+IAo8AA49oA1QDKycUbx8nI
AL2/yM6/zK3//////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFdQAACB+vAAAIRbsFdQAACB+8AAAICrsFdQAACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAAR71/AAAIZm4UCCENgAAAuwFrAAAIIxUQAAPDACk7CL2xAAAIZm1obAK95wAACGZtaGwCvfsA
AAhmbWhsAs3c1eDgAMMA293q2QDt4+nmAOTV5ujtAMrJxRvHycj+6NzZANvd2ugAzMO8vMnIzaz/
0+Pp5gDKycUbx8nIANvj6ADV4OAA59nq2eIA293a6P7Mw7y8ycjNqwDO1d/ZANUA4OPj363/vePh
2QDW1dffANXi7QDo3eHZq//O3N3nANvd2ugA3efitOgA2uPm/ujc3ecA29Xh2a3/AABwtYGwE0wk
aBNIJBgVpQAmoF0AKAHRqF2gVQE2By730QEgAJANTAYl4HxAB0APAigJ0UgmIAAxAGpGCUsA8Aj4
ATZPLvbRZDQBPe3RAbBwvRhHwEbYQgADnDAAAIBCAgIlOwQINzo7PTw+PwA=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRsACASjMAgJ5Og==`),
                'BPRD 1.10': decodeBase64(`aAEBRMACASjMAgKhOg==`),
                'BPRI 1.10': decodeBase64(`aAEBScACASjMAgKNOg==`),
                'BPRS 1.10': decodeBase64(`aAEBU8ACASjMAgKNOg==`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUbAAgEozAICeTo=`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUTAAgEozAICoTo=`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUnAAgEozAICjTo=`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVPAAgEozAICjTo=`),
            }),
        },
    },
    {
        id: 'custom-master-ball',
        label: 'Master Ball',
        description: 'Gives a Master Ball, once per card; receive the card again for another. Original Master Ball event by Decryptu.',
        payloads: {
            frlg: [decodeBase64(`9wMjAA8AAAAAAMfTzc6/zNMAwcPAzv////////////////////////////////////+7AObZ5ODV
19nh2eLoAMe7zc6/zAC8u8bG////////////////////uwDHu83Ov8wAvLvGxgDd5wDj4gDd6OcA
69XtAOjj/////////////+bZ5ODV19kA6NzZAOPi2QDt4+kA6efZ2K3////////////////////O
1eDfAOjjAOjc2QDY2eDd6tnm7QDh1eIA4+IA6NzZ////////////o+LYANrg4+PmAOPaANUAysnF
v8fJyAC9v8jOv8yt/////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
WivYA7sBZAAACL2AAAAIZm1oKrYCIbZAAAC7ASsAAAi5WQAACL2kAAAIZm1oRgEAAQAhDYAAALsB
cQAACBoAgAEAGgGAAQAJABa2QAEAuVkAAAgp2AMptgK5fgAACL3AAAAIZm1ouX4AAAi95wAACGZt
aLl+AAAIbAK7AMe7zc6/zAC8u8bGANjZ4N3q2ebtANzV5wDV5ubd6tnYq//T4+kA5tnX2d3q2dgA
1QDHu83Ov8wAvLvGxqv/0+PpANXg5tnV2O0A1+Pg4NnX6NnYAOjc2QDHu83Ov8wAvLvGxq3/yOMA
5uPj4asAx9Xf2QDn5NXX2bgA6NzZ4v7X4+HZANbV19+t/w==`)],
        },
    },
    {
        id: 'custom-pocket-casino',
        label: 'Pocket Casino (Slots)',
        description: 'Talk to the delivery person on Pokémon Center 2F and he opens the Game Corner slots where you stand. The Game Corner music plays while the reels spin. If you have no Coin Case, he lends one for this game and takes it back when you quit. Your coins stay with you. If you have fewer than three coins, he gives you 100 to start. Quitting the machine brings you back to the delivery man on Pokémon Center 2F.',
        payloads: {
            ...romPayloads(decodeBase64(`8QM0AAAAAAAAAMrJvcW/zgC9u83DyMn////////////////////////////////////B1eHZAL3j
5uLZ5qv/////////////////////////////////////uwDn5NnX3dXgANvV4dkA3ecA69Xd6N3i
2////////////////////97p5+gA2uPmAO3j6av///////////////////////////////////+8
5t3i2wDVAL3Jw8gAvbvNvwDV4tgAvcnDyM2t////////////////wdXh2QDj4qv/////////////
/////////////////////////////8G8rsbd4t8AztnV4f//////////////////////////////
////////////////////////////////////////////////////////////AAAAAAAAuAAAAAhq
Wh+uAAAIUrsFKgEACB+vAAAIRbsFKgEACB+8AAAICrsFKgEACA8A0HgAAg8BkXhAGA8CgBgEMg8D
mmAARxYBQAAAFgRAAAArQwK7AV4AAAgpQwIWBEABAEcEAQEAIQ2AAAC7BYgAAAhGBAEBACENgAAA
uwGIAAAIRAQBAQAWAUABABYDQAAAswJAIQJAAwC7BKMAAAi0ZAAWA0ABACEBQAEAuwHDAAAIIQNA
AQC7AeIAAAi9rwEACLnnAAAIIQNAAQC7AdgAAAi9YwEACLnnAAAIvTQBAAi55wAACL2TAQAIZm1o
MxEBACYNgB4BIxUQAANLAbj6AAAIiQ2AIQRAAQC7BRABAAgqQwIhAUABALsFKAEACEUEAQEAveUB
AAhmbWhsAr0fAgAIZm1obAK7AL3Jw8gAvbvNvwDV4tgAoqGhAL3Jw8jNuP7e6efoANrj5gDo3N3n
ANvV4dmt/9Pj6QDX1eIA1uPm5uPrANUAvcnDyAC9u82/uP7e6efoANrj5gDo3N3nANvV4dmt/8LZ
5tkA1ebZAKKhoQC9ycPIzQDo4wDk4NXtrf/C2dwA3NncuADg4+Pf5wDg3d/ZAOfj4dnj4tn+69Xi
6OcA6OMA5ODV7QDn4+HZAOfg4+jnrf/DAOvd4OAA6NXf2QDo3NkAvcnDyAC9u82/ANbV19+t/tPj
6eYAvcnDyM0A5+jV7QDr3ejcAO3j6a3/ztzd5wDb3droAN3n4rToANrj5v7o3N3nANvV4dmt/wAA
B0gAaAdJQBgHSZpoEhpSGJpg+SKSAAQ6g1iLUPvRcEfYQgADJDYAAAD8AwI=`), {
                'BPGE 1.10': decodeBase64(`XAEBRw==`),
                'BPRF 1.10': decodeBase64(`aAEBRrADASg=`),
                'BPRD 1.10': decodeBase64(`aAEBRLADASg=`),
                'BPRI 1.10': decodeBase64(`aAEBSbADASg=`),
                'BPRS 1.10': decodeBase64(`aAEBU7ADASg=`),
                'BPGF 1.10': decodeBase64(`XAEBR2gBAUawAwEo`),
                'BPGD 1.10': decodeBase64(`XAEBR2gBAUSwAwEo`),
                'BPGI 1.10': decodeBase64(`XAEBR2gBAUmwAwEo`),
                'BPGS 1.10': decodeBase64(`XAEBR2gBAVOwAwEo`),
            }),
        },
    },
];
