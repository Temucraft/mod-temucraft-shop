# mod-temucraft-shop

The Temucraft Token is a currency (it shows in the currency tab), spent with the **Temucraft Curator**. Its icon is the Siamese cat (`INV_Pet_Cats_SiameseCat`, display 119564, a stock look).

## Earning tokens

All amounts are in `conf/mod-temucraft-shop.conf.dist`; 0 turns a source off.

- **Levelling:** 1 per new level, 5 more at every 10th level. Each level pays once per character.
- **Bosses:** the first kill of each dungeon boss (2), heroic boss (3) and raid boss (10), per character and difficulty. Every group member in the instance gets them.
- **Achievements:** 1 per 5 achievement points.
- **Gold:** the curator sells tokens for 10 gold each (`GoldPerToken`, 0 = off).
- **Game masters:** `.temucraft shop add <count> [player]` and `.temucraft shop remove <count> [player]`.
- **Selling back:** with `GoldPerTokenSold` above 0 the curator also buys tokens back for gold (off by default).

## The curator

One stands beside the innkeeper of every capital (Stormwind, Orgrimmar, Ironforge, Undercity, Darnassus, Thunder Bluff, Exodar, Silvermoon, Dalaran, Shattrath). `.temucraft shop summon` calls one for 3 minutes, every 10 minutes. Its gossip opens a vendor window per category; prices are in tokens:

| Category | Items |
|---|---|
| Miscellaneous | 14 |
| Skins | 39 |
| Gameplay | 11 |
| Glyphs | 9 |
| Mounts | 63 |
| Companions | 50 |
| Fashion | 51 |
| Illusions | 27 |
| Toys | 26 |
| Auras | from mod-temucraft-auras |
| Helpers | Courier's Bell, Folding Mirror, Almanac, Lantern (mod-temucraft-courier), Banker's Writ, Guild Vault Charter, Skyreader's Atlas, Hearthbound and Fel Waygate, Wayfarer's Beacon (mod-temucraft-travel) |

## Bringing back a deleted character

The curator can bring back a character the same account deleted, for 150 tokens (`RestoreCost`, 0 = off). It lists
the 15 most recent; if the name was taken meanwhile, the character asks for a new name at login. Needs
`CharDelete.Method = 1` in worldserver.conf (deleted characters are kept, not erased). The account must have a free
character place.

## Saddlemaster's Handbooks

Every riding trainer also sells Saddlemaster's Handbooks (Journeyman, Expert, Artisan, Cold Weather) for a quarter of
the trainer's gold price. A book teaches its rank only if another character of the same account already knows it (and
the usual level and previous rank); otherwise it says so and can be sold back for what it cost. Items 610100-610103.

## Service tokens

Rename, appearance change, race change (one token, same faction), faction change (one token, new faction and race), pet rename (call the pet first) and guild rename (`.temucraft shop guildname <name>`, guild master only).

## Form glyphs

While a glyph is known, druid forms and Ghost Wolf show another model (Frostsaber, Ice Bear, Orca, Frostkin, the treants and more). The list is `mod_temucraft_shop_form_glyph`.

## Commands

- `.temucraft shop` shows your tokens; `.temucraft shop summon`; `.temucraft shop guildname <name>`
- `.temucraft shop add <count> [player]`, `.temucraft shop remove <count> [player]` (GM), `.temucraft shop reload` (admin)

## Installing

- Needs **[mod-temucraft-lostlands](https://github.com/Temucraft/mod-temucraft-lostlands)** (the shop goods are its items).
- SQL: `data/sql/db-world/base` and `data/sql/db-characters/base`.
- Client: CurrencyTypes and ItemExtendedCost rows. They are already in configuration-main's `patch-5`; `client/patch-V.MPQ` has the same files if you install the module on its own.
- Shop goods that other vendors also sell keep their gold price on top of the tokens.

## Texts and translations

The 32 player-facing texts (chat messages, gossip options) are in the `module_string` table under the module name
`mod-temucraft-shop`, so they can be changed without rebuilding. Translations go in `module_string_locale` with the same
module and id plus the locale (`deDE`, `esES`, `frFR`, ...). If a row is missing the English text in the code is used.

Part of the Temucraft modules for AzerothCore 3.3.5a.
