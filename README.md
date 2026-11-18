# mod-temucraft-shop

The Temucraft Token is a currency (it shows in the currency tab), spent with the **Temucraft Curator**.

## Earning tokens

All amounts are in `conf/mod-temucraft-shop.conf.dist`; 0 turns a source off.

- **Levelling:** 1 per new level, 5 more at every 10th level. Each level pays once per character.
- **Bosses:** the first kill of each dungeon boss (2), heroic boss (3) and raid boss (10), per character and difficulty. Every group member in the instance gets them.
- **Achievements:** 1 per 5 achievement points.
- **Gold:** the curator sells tokens for 10 gold each (`GoldPerToken`, 0 = off).
- **Game masters:** `.shop add <count> [player]`.

## The curator

One stands beside the innkeeper of every capital (Stormwind, Orgrimmar, Ironforge, Undercity, Darnassus, Thunder Bluff, Exodar, Silvermoon, Dalaran, Shattrath). `.shop summon` calls one for 3 minutes, every 10 minutes. Its gossip opens a vendor window per category; prices are in tokens:

| Category | Items |
|---|---|
| Miscellaneous | 14 |
| Skins | 39 |
| Gameplay | 11 |
| Glyphs | 9 |
| Mounts | 59 |
| Companions | 36 |
| Fashion | 51 |
| Illusions | 24 |

## Service tokens

Rename, appearance change, race change (to the other faction it becomes a faction change), pet rename (call the pet first) and guild rename (`.shop guildname <name>`, guild master only).

## Form glyphs

While a glyph is known, druid forms and Ghost Wolf show another model (Frostsaber, Ice Bear, Orca, Frostkin, the treants and more). The list is `mod_temucraft_shop_form_glyph`.

## Commands

- `.shop` shows your tokens; `.shop summon`; `.shop guildname <name>`
- `.shop add <count> [player]` (GM), `.shop reload` (admin)

## Installing

- Needs **[mod-temucraft-lostlands](https://github.com/Temucraft/mod-temucraft-lostlands)** (the shop goods are its items).
- SQL: `data/sql/db-world/base` and `data/sql/db-characters/base`.
- Client: CurrencyTypes and ItemExtendedCost rows. They are already in configuration-main's `patch-5`; `client/patch-V.MPQ` has the same files if you install the module on its own.
- Shop goods that other vendors also sell keep their gold price on top of the tokens.

## Texts and translations

The 32 player-facing texts (chat messages, gossip options) are in the `module_string` table under the module name
`mod-temucraft-shop`, so they can be changed without rebuilding. Translations go in `module_string_locale` with the same
module and id plus the locale (`deDE`, `esES`, `frFR`, ...). If a row is missing the English text in the code is used.
