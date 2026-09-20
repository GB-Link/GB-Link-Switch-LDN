# Assets

`party.json` is the party a fresh copy of the desktop app starts with, and the console
host too, when there is no `local/party.json` yet. It has that file's format: `selected`,
the offered slot, and six `slots` of hex PK3 data with `null` for the empty ones. Making
another party the default is a matter of copying a `local/party.json` over it and
building again; `host/tests` checks that it can start a trade as it is. It holds
user-provided Pokemon, whose data names their original trainer, so replace it before
publishing if that matters.

`sprites/1.png` through `sprites/386.png` were downloaded from:
https://github.com/PokeAPI/sprites/tree/master/sprites/pokemon

Individual source URL: `https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/<national-id>.png`.
The images are packaged locally so the interface works offline. Pokemon names
and artwork belong to their respective rights holders, including Nintendo,
Game Freak and The Pokemon Company. The source project's code license does
not transfer ownership of game artwork. This interface does not imply endorsement.
