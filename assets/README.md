# Assets

`party.json` is the party a fresh copy of the desktop app starts with, and the console
host too, when there is no `local/party.json` yet. It has that file's format: `selected`,
the offered slot, and six `slots` of hex PK3 data with `null` for the empty ones. Making
another party the default is a matter of copying a `local/party.json` over it and
building again; `host/tests` checks that it can start a trade as it is. It holds
user-provided Pokemon, whose data names their original trainer, so replace it before
publishing if that matters.

No artwork is kept here. The desktop app and the web client both load Pokémon pictures
from the [PokeAPI sprite collection](https://github.com/PokeAPI/sprites) when they show
them. Pokémon names and artwork belong to their respective rights holders, including
Nintendo, Game Freak and The Pokémon Company, and this project does not imply their
endorsement.
