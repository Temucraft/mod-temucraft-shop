/*
 * mod-temucraft-shop
 *
 * The Temucraft Token is a currency (it shows in the currency tab). Players earn it by levelling, by the first kill
 * of each dungeon and raid boss, and from achievements; the curator also trades tokens for gold. The Temucraft
 * Curator sells the shop goods in eight categories (vendor windows, prices in tokens).
 *
 *   npc_temucraft_shop_curator     gossip: category lists, buy tokens for gold, token balance
 *   item_temucraft_shop_service    rename, appearance change, race change, pet rename, guild rename tokens
 *   form glyphs                    shapeshift forms show another model while the glyph is known
 *   .temucraft shop                          balance; .temucraft shop summon; .temucraft shop add <count> [player] (GM); .temucraft shop guildname <name>
 */

#include "TemucraftLocked.h"
#include "AccountMgr.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "TemucraftModuleText.h"
#include "ChatCommand.h"
#include "CommandScript.h"
#include "Config.h"
#include "Creature.h"
#include "CreatureScript.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Mail.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Item.h"
#include "ItemScript.h"
#include "Log.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "TemporarySummon.h"
#include "UnitScript.h"
#include "WorldScript.h"
#include "World.h"
#include "WorldSession.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace Acore::ChatCommands;

namespace TemucraftShop
{
    constexpr uint32 CURATOR_ENTRY = 608000;
    constexpr uint32 VENDOR_FIRST = 608001;
    constexpr uint32 CURATOR_TEXT = 608000;

    struct Cfg
    {
        bool enable = true;
        uint32 token = 608000;
        uint32 perLevel = 1;
        uint32 per10Levels = 5;
        uint32 dungeonBoss = 2;
        uint32 heroicBoss = 3;
        uint32 raidBoss = 10;
        uint32 achievementPointsPerToken = 5;
        uint32 goldPerToken = 10;
        uint32 goldPerTokenSold = 0;   // gold a player gets for one token sold back (0 = selling is off)
        uint32 restoreCost = 150;      // tokens to bring back a deleted character (0 = off)
        bool summon = true;
        uint32 summonCooldown = 600;
        uint32 summonDuration = 180;
    } cfg;

    struct Service { uint8 type; uint8 race; };
    struct FormGlyph { uint32 spell; uint32 alliance; uint32 horde; };

    std::unordered_map<uint32, Service> sServices;
    std::unordered_map<uint32, std::vector<FormGlyph>> sGlyphs;   // form -> glyphs
    Temucraft::LockedMap<uint32, uint32> sMaxLevel;                 // guid -> highest level already rewarded
    Temucraft::LockedMap<uint32, std::unordered_set<uint32>> sBosses;   // guid -> boss entries already rewarded
    std::unordered_map<uint32, uint32> sLastSummon;               // guid -> game time

    constexpr uint8 CATEGORY_COUNT = 11;
    char const* CategoryNames[CATEGORY_COUNT] = { "Miscellaneous", "Skins", "Gameplay", "Glyphs", "Mounts", "Companions", "Fashion", "Illusions", "Toys", "Auras", "Helpers" };

    void LoadConfig()
    {
        cfg.enable = sConfigMgr->GetOption<bool>("Temucraft.Shop.Enable", true);
        cfg.token = sConfigMgr->GetOption<uint32>("Temucraft.Shop.TokenItem", 608000);
        cfg.perLevel = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.PerLevel", 1);
        cfg.per10Levels = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.Per10Levels", 5);
        cfg.dungeonBoss = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.DungeonBoss", 2);
        cfg.heroicBoss = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.HeroicBoss", 3);
        cfg.raidBoss = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.RaidBoss", 10);
        cfg.achievementPointsPerToken = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Tokens.AchievementPointsPerToken", 5);
        cfg.goldPerToken = sConfigMgr->GetOption<uint32>("Temucraft.Shop.GoldPerToken", 10);
        cfg.goldPerTokenSold = sConfigMgr->GetOption<uint32>("Temucraft.Shop.GoldPerTokenSold", 0);
        cfg.restoreCost = sConfigMgr->GetOption<uint32>("Temucraft.Shop.RestoreCost", 150);
        cfg.summon = sConfigMgr->GetOption<bool>("Temucraft.Shop.Summon.Enable", true);
        cfg.summonCooldown = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Summon.Cooldown", 600);
        cfg.summonDuration = sConfigMgr->GetOption<uint32>("Temucraft.Shop.Summon.Duration", 180);
    }

    void LoadTables()
    {
        sServices.clear();
        if (QueryResult r = WorldDatabase.Query("SELECT `ItemID`, `Service`, `Race` FROM `mod_temucraft_shop_service`"))
            do
            {
                Field* f = r->Fetch();
                sServices[f[0].Get<uint32>()] = { f[1].Get<uint8>(), f[2].Get<uint8>() };
            } while (r->NextRow());

        sGlyphs.clear();
        if (QueryResult r = WorldDatabase.Query("SELECT `Form`, `SpellID`, `DisplayAlliance`, `DisplayHorde` FROM `mod_temucraft_shop_form_glyph`"))
            do
            {
                Field* f = r->Fetch();
                sGlyphs[f[0].Get<uint8>()].push_back({ f[1].Get<uint32>(), f[2].Get<uint32>(), f[3].Get<uint32>() });
            } while (r->NextRow());

        LOG_INFO("module", "mod-temucraft-shop: {} service tokens, {} form glyph forms", sServices.size(), sGlyphs.size());
    }

    // ---------------------------------------------------------------- tokens
    uint32 Balance(Player* player)
    {
        return player->GetItemCount(cfg.token, true);
    }

    void Give(Player* player, uint32 count, char const* reason)
    {
        if (!count || !cfg.enable)
            return;

        ItemPosCountVec dest;
        uint32 noSpace = 0;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, cfg.token, count, &noSpace);
        uint32 stored = count - (msg == EQUIP_ERR_OK ? 0 : noSpace);
        if (msg != EQUIP_ERR_OK && stored)
        {
            dest.clear();
            msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, cfg.token, stored);
        }
        if (msg == EQUIP_ERR_OK && stored)
            if (Item* item = player->StoreNewItem(dest, cfg.token, true))
                player->SendNewItem(item, stored, true, false);

        if (stored < count)
        {
            // currency cap or no room: send the rest by mail
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            MailDraft draft("Temucraft Tokens", "Tokens you earned that did not fit.");
            if (Item* item = Item::CreateItem(cfg.token, count - stored, player))
            {
                item->SaveToDB(trans);
                draft.AddItem(item);
            }
            draft.SendMailTo(trans, MailReceiver(player), MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM));
            CharacterDatabase.CommitTransaction(trans);
        }

        ChatHandler(player->GetSession()).PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", player->GetSession(), 1, "|cff66ccffTemucraft:|r +{} token{} ({})."), count, count == 1 ? "" : "s", reason);
    }

    bool Take(Player* player, uint32 count)
    {
        if (Balance(player) < count)
            return false;
        player->DestroyItemCount(cfg.token, count, true);
        return true;
    }

    // ---------------------------------------------------------------- curator
    enum Actions
    {
        ACTION_CATEGORY = 100,     // + index
        ACTION_BUY_1 = 200,
        ACTION_BUY_10 = 201,
        ACTION_BALANCE = 202,
        ACTION_SELL_1 = 203,
        ACTION_SELL_10 = 204,
        ACTION_RESTORE_LIST = 205,
        ACTION_RESTORE = 300,      // + index in the list
    };

    // deleted characters of this account the curator can bring back (needs CharDelete.Method = 1, which keeps them)
    std::unordered_map<uint32, std::vector<uint32>> sRestoreList;   // account -> guids shown in the last list

    uint32 DeletedCount(Player* player)
    {
        QueryResult r = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE deleteInfos_Account = {} AND deleteDate IS NOT NULL", player->GetSession()->GetAccountId());
        return r ? uint32(r->Fetch()[0].Get<uint64>()) : 0;
    }

    void ShowRestoreList(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        std::vector<uint32>& guids = sRestoreList[player->GetSession()->GetAccountId()];
        guids.clear();
        if (QueryResult r = CharacterDatabase.Query("SELECT guid, deleteInfos_Name, level FROM characters WHERE deleteInfos_Account = {} AND deleteDate IS NOT NULL ORDER BY deleteDate DESC LIMIT 15",
                player->GetSession()->GetAccountId()))
            do
            {
                Field* f = r->Fetch();
                guids.push_back(f[0].Get<uint32>());
                std::string text = Acore::StringFormat(TemucraftModuleText("mod-temucraft-shop", player, 44, "Bring back {} (level {})"), f[1].Get<std::string>(), f[2].Get<uint8>());
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, text, GOSSIP_SENDER_MAIN, ACTION_RESTORE + uint32(guids.size() - 1),
                    Acore::StringFormat(TemucraftModuleText("mod-temucraft-shop", player, 45, "Bring back {} for {} tokens?"), f[1].Get<std::string>(), cfg.restoreCost), 0, false);
            } while (r->NextRow());
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, TemucraftModuleText("mod-temucraft-shop", player, 46, "Back"), GOSSIP_SENDER_MAIN, ACTION_BALANCE + 1000);
        SendGossipMenuFor(player, CURATOR_TEXT, creature->GetGUID());
    }

    void Restore(Player* player, uint32 index)
    {
        ChatHandler handler(player->GetSession());
        uint32 account = player->GetSession()->GetAccountId();
        std::vector<uint32> const& guids = sRestoreList[account];
        if (index >= guids.size() || !cfg.restoreCost)
            return;
        uint32 guid = guids[index];
        QueryResult r = CharacterDatabase.Query("SELECT deleteInfos_Name, race, class, gender, level FROM characters WHERE guid = {} AND deleteInfos_Account = {} AND deleteDate IS NOT NULL", guid, account);
        if (!r)
            return;
        Field* f = r->Fetch();
        std::string name = f[0].Get<std::string>();
        if (AccountMgr::GetCharactersCount(account) >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        {
            handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 47, "|cff66ccffTemucraft:|r this account has no free character place on this realm."));
            return;
        }
        if (Balance(player) < cfg.restoreCost)
        {
            handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 48, "|cff66ccffTemucraft:|r that costs {} tokens."), cfg.restoreCost);
            return;
        }
        bool rename = ObjectMgr::CheckPlayerName(name) != CHAR_NAME_SUCCESS || sCharacterCache->GetCharacterGuidByName(name);
        player->DestroyItemCount(cfg.token, cfg.restoreCost, true);
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        trans->Append("UPDATE characters SET name = '{}', account = {}, deleteDate = NULL, deleteInfos_Name = NULL, deleteInfos_Account = NULL{} WHERE guid = {} AND deleteDate IS NOT NULL",
            [&] { std::string n = name; CharacterDatabase.EscapeString(n); return n; }(), account, rename ? ", at_login = at_login | 1" : "", guid);
        player->SaveInventoryAndGoldToDB(trans);
        CharacterDatabase.CommitTransaction(trans);
        sCharacterCache->AddCharacterCacheEntry(ObjectGuid(HighGuid::Player, guid), account, name, f[3].Get<uint8>(), f[1].Get<uint8>(), f[2].Get<uint8>(), f[4].Get<uint8>());
        handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 49, "|cff66ccffTemucraft:|r {} is back on your character list{}."), name,
            rename ? TemucraftModuleText("mod-temucraft-shop", handler, 50, " (the name was taken: pick a new one when you log in)") : "");
    }

    void ShowMenu(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        for (uint8 i = 0; i < CATEGORY_COUNT; ++i)
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, std::string(TemucraftModuleText("mod-temucraft-shop", player, 2, "Browse: ")) + CategoryNames[i], GOSSIP_SENDER_MAIN, ACTION_CATEGORY + i);
        if (cfg.goldPerToken)
        {
            uint32 copper = cfg.goldPerToken * GOLD;
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, TemucraftModuleText("mod-temucraft-shop", player, 3, "Buy 1 token for gold."), GOSSIP_SENDER_MAIN, ACTION_BUY_1, TemucraftModuleText("mod-temucraft-shop", player, 4, "Trade gold for 1 Temucraft Token?"), copper, false);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, TemucraftModuleText("mod-temucraft-shop", player, 5, "Buy 10 tokens for gold."), GOSSIP_SENDER_MAIN, ACTION_BUY_10, TemucraftModuleText("mod-temucraft-shop", player, 6, "Trade gold for 10 Temucraft Tokens?"), copper * 10, false);
        }
        if (cfg.goldPerTokenSold)
        {
            uint32 have = Balance(player);
            if (have >= 1)
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, TemucraftModuleText("mod-temucraft-shop", player, 33, "Sell 1 token for gold."), GOSSIP_SENDER_MAIN, ACTION_SELL_1, TemucraftModuleText("mod-temucraft-shop", player, 34, "Sell 1 Temucraft Token for gold?"), 0, false);
            if (have >= 10)
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, TemucraftModuleText("mod-temucraft-shop", player, 35, "Sell 10 tokens for gold."), GOSSIP_SENDER_MAIN, ACTION_SELL_10, TemucraftModuleText("mod-temucraft-shop", player, 36, "Sell 10 Temucraft Tokens for gold?"), 0, false);
        }
        if (cfg.restoreCost && DeletedCount(player))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormat(TemucraftModuleText("mod-temucraft-shop", player, 43, "Bring back a deleted character ({} tokens)"), cfg.restoreCost), GOSSIP_SENDER_MAIN, ACTION_RESTORE_LIST);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, TemucraftModuleText("mod-temucraft-shop", player, 7, "How do I earn tokens?"), GOSSIP_SENDER_MAIN, ACTION_BALANCE);
        SendGossipMenuFor(player, CURATOR_TEXT, creature->GetGUID());
    }

    class npc_temucraft_shop_curator : public CreatureScript
    {
    public:
        npc_temucraft_shop_curator() : CreatureScript("npc_temucraft_shop_curator") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            if (!cfg.enable)
                return false;
            ShowMenu(player, creature);
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            if (action >= ACTION_CATEGORY && action < ACTION_CATEGORY + CATEGORY_COUNT)
            {
                CloseGossipMenuFor(player);
                player->GetSession()->SendListInventory(creature->GetGUID(), VENDOR_FIRST + (action - ACTION_CATEGORY));
                return true;
            }

            if (action == ACTION_RESTORE_LIST)
            {
                ShowRestoreList(player, creature);
                return true;
            }
            if (action >= ACTION_RESTORE && action < ACTION_RESTORE + 15)
            {
                Restore(player, action - ACTION_RESTORE);
                ShowMenu(player, creature);
                return true;
            }

            ChatHandler handler(player->GetSession());
            if (action == ACTION_BUY_1 || action == ACTION_BUY_10)
            {
                uint32 count = action == ACTION_BUY_1 ? 1 : 10;
                uint32 copper = cfg.goldPerToken * GOLD * count;
                if (!cfg.goldPerToken || !player->HasEnoughMoney(copper))
                    player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, 0, 0);
                else
                {
                    player->ModifyMoney(-int32(copper));
                    Give(player, count, "bought with gold");
                }
            }
            else if (action == ACTION_SELL_1 || action == ACTION_SELL_10)
            {
                uint32 count = action == ACTION_SELL_1 ? 1 : 10;
                uint64 copper = uint64(cfg.goldPerTokenSold) * GOLD * count;
                if (cfg.goldPerTokenSold && Balance(player) >= count && player->ModifyMoney(int32(copper), false))
                {
                    player->DestroyItemCount(cfg.token, count, true);
                    handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 37, "|cff66ccffTemucraft:|r sold {} token(s) for {} gold."), count, cfg.goldPerTokenSold * count);
                }
            }
            else if (action == ACTION_BALANCE)
            {
                handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 8, "|cff66ccffTemucraft:|r you have {} token{}."), Balance(player), Balance(player) == 1 ? "" : "s");
                if (cfg.perLevel || cfg.per10Levels)
                    handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 9, "  Levelling: {} per level, {} more every 10 levels."), cfg.perLevel, cfg.per10Levels);
                if (cfg.dungeonBoss || cfg.heroicBoss || cfg.raidBoss)
                    handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 10, "  First kill of each boss: dungeon {}, heroic {}, raid {}."), cfg.dungeonBoss, cfg.heroicBoss, cfg.raidBoss);
                if (cfg.achievementPointsPerToken)
                    handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 11, "  Achievements: 1 per {} achievement points."), cfg.achievementPointsPerToken);
                if (cfg.goldPerToken)
                    handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 12, "  Or buy them here: {} gold each."), cfg.goldPerToken);
            }
            ShowMenu(player, creature);
            return true;
        }
    };

    // ---------------------------------------------------------------- service tokens
    enum ServiceType : uint8
    {
        SERVICE_RENAME = 1,
        SERVICE_CUSTOMIZE = 2,
        SERVICE_RACE = 3,
        SERVICE_PET_RENAME = 4,
        SERVICE_GUILD_RENAME = 5,
        SERVICE_BANK_SLOTS = 6,     // Banker's Writ: every bank bag slot
        SERVICE_GUILD_TAB = 7,      // Guild Vault Charter: the guild's next bank tab, no gold
    };

    constexpr uint8 MAX_BANK_BAG_SLOTS = 7;

    // buys the guild's next bank tab for the guild master without costing gold: the tab's price is handed over first
    // and the stock purchase takes it again (it only goes through for the right tab, so every tab is tried)
    bool BuyFreeGuildTab(Player* player, Guild* guild)
    {
        static ServerConfigs const COSTS[GUILD_BANK_MAX_TABS] = { CONFIG_GUILD_BANK_TAB_COST_0, CONFIG_GUILD_BANK_TAB_COST_1, CONFIG_GUILD_BANK_TAB_COST_2,
                                                           CONFIG_GUILD_BANK_TAB_COST_3, CONFIG_GUILD_BANK_TAB_COST_4, CONFIG_GUILD_BANK_TAB_COST_5 };
        for (uint8 tab = 0; tab < GUILD_BANK_MAX_TABS; ++tab)
        {
            uint32 cost = sWorld->getIntConfig(COSTS[tab]);
            if (!cost || uint64(player->GetMoney()) + cost > MAX_MONEY_AMOUNT)
                continue;
            uint32 before = player->GetMoney();
            player->ModifyMoney(int32(cost));
            guild->HandleBuyBankTab(player->GetSession(), tab);
            if (player->GetMoney() == before)
                return true;                        // bought: the money went back out
            player->ModifyMoney(-int32(player->GetMoney() - before));
        }
        return false;
    }

    class item_temucraft_shop_service : public ItemScript
    {
    public:
        item_temucraft_shop_service() : ItemScript("item_temucraft_shop_service") { }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
        {
            auto it = sServices.find(item->GetEntry());
            if (it == sServices.end())
                return false;

            ChatHandler handler(player->GetSession());
            Service const& s = it->second;
            switch (s.type)
            {
                case SERVICE_RENAME:
                    player->SetAtLoginFlag(AT_LOGIN_RENAME);
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 13, "|cff66ccffTemucraft:|r log out: you can pick a new name on the character screen."));
                    break;
                case SERVICE_CUSTOMIZE:
                    player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 14, "|cff66ccffTemucraft:|r log out: you can change your appearance on the character screen."));
                    break;
                case SERVICE_RACE:
                {
                    if (s.race && s.race == player->getRace())
                    {
                        handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 15, "|cff66ccffTemucraft:|r you already are of that race."));
                        return true;
                    }
                    bool otherFaction = s.race && Player::TeamIdForRace(s.race) != player->GetTeamId(true);
                    player->SetAtLoginFlag(otherFaction ? AT_LOGIN_CHANGE_FACTION : AT_LOGIN_CHANGE_RACE);
                    handler.SendSysMessage(otherFaction
                        ? TemucraftModuleText("mod-temucraft-shop", handler, 16, "|cff66ccffTemucraft:|r log out: you can change race (and faction) on the character screen."
                        ) : TemucraftModuleText("mod-temucraft-shop", handler, 17, "|cff66ccffTemucraft:|r log out: you can change race on the character screen."));
                    break;
                }
                case SERVICE_PET_RENAME:
                {
                    Pet* pet = player->GetPet();
                    if (!pet || pet->getPetType() != HUNTER_PET)
                    {
                        handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 18, "|cff66ccffTemucraft:|r call the hunter pet you want to rename first."));
                        return true;
                    }
                    pet->SetByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED);
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 19, "|cff66ccffTemucraft:|r right-click your pet's portrait and choose Rename."));
                    break;
                }
                case SERVICE_GUILD_RENAME:
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 20, "|cff66ccffTemucraft:|r as guild master, type .temucraft shop guildname <new name> to use this token."));
                    return true;
                case SERVICE_BANK_SLOTS:
                    if (player->GetBankBagSlotCount() >= MAX_BANK_BAG_SLOTS)
                    {
                        handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 51, "|cff66ccffTemucraft:|r you already have every bank bag slot."));
                        return true;
                    }
                    player->SetBankBagSlotCount(MAX_BANK_BAG_SLOTS);
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 52, "|cff66ccffTemucraft:|r every bank bag slot is open."));
                    break;
                case SERVICE_GUILD_TAB:
                {
                    Guild* guild = player->GetGuild();
                    if (!guild || guild->GetLeaderGUID() != player->GetGUID())
                    {
                        handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 53, "|cff66ccffTemucraft:|r only a guild master can use this."));
                        return true;
                    }
                    if (!BuyFreeGuildTab(player, guild))
                    {
                        handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 54, "|cff66ccffTemucraft:|r your guild has every bank tab already (or you carry too much gold to take the tab price)."));
                        return true;
                    }
                    handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 55, "|cff66ccffTemucraft:|r your guild has a new bank tab."));
                    break;
                }
                default:
                    return false;
            }
            player->DestroyItemCount(item->GetEntry(), 1, true);
            return true;
        }
    };

    // ---------------------------------------------------------------- earning
    class TemucraftShopPlayer : public PlayerScript
    {
    public:
        TemucraftShopPlayer() : PlayerScript("TemucraftShopPlayer", {
            PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_DELETE, PLAYERHOOK_ON_LEVEL_CHANGED,
            PLAYERHOOK_ON_CREATURE_KILL, PLAYERHOOK_ON_ACHI_COMPLETE }) { }

        void OnPlayerLogin(Player* player) override
        {
            uint32 guid = player->GetGUID().GetCounter();
            sMaxLevel[guid] = player->GetLevel();
            sBosses[guid].clear();
            if (QueryResult r = CharacterDatabase.Query("SELECT `value` FROM `mod_temucraft_character_setting` WHERE `guid` = {} AND `setting` = 'shopmaxlevel'", guid))
                sMaxLevel[guid] = std::max<uint32>(uint32(r->Fetch()[0].Get<int32>()), player->GetLevel());
            else
                CharacterDatabase.Execute("INSERT IGNORE INTO `mod_temucraft_character_setting` (`guid`, `setting`, `value`) VALUES ({}, 'shopmaxlevel', {})", guid, player->GetLevel());
            if (QueryResult r = CharacterDatabase.Query("SELECT `entry` FROM `mod_temucraft_shop_boss` WHERE `guid` = {}", guid))
                do sBosses[guid].insert(r->Fetch()[0].Get<uint32>()); while (r->NextRow());
        }

        void OnPlayerLogout(Player* player) override
        {
            uint32 guid = player->GetGUID().GetCounter();
            sMaxLevel.erase(guid);
            sBosses.erase(guid);
        }

        void OnPlayerDelete(ObjectGuid guid, uint32 /*accountId*/) override
        {
            CharacterDatabase.Execute("DELETE FROM `mod_temucraft_character_setting` WHERE `guid` = {} AND `setting` = 'shopmaxlevel'", guid.GetCounter());
            CharacterDatabase.Execute("DELETE FROM `mod_temucraft_shop_boss` WHERE `guid` = {}", guid.GetCounter());
        }

        void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
        {
            if (!cfg.enable)
                return;
            uint32 guid = player->GetGUID().GetCounter();
            uint32& best = sMaxLevel[guid];
            uint32 level = player->GetLevel();
            if (level <= best)
                return;
            uint32 tokens = 0;
            for (uint32 l = best + 1; l <= level; ++l)
                tokens += cfg.perLevel + (l % 10 == 0 ? cfg.per10Levels : 0);
            best = level;
            CharacterDatabase.Execute("REPLACE INTO `mod_temucraft_character_setting` (`guid`, `setting`, `value`) VALUES ({}, 'shopmaxlevel', {})", guid, level);
            Give(player, tokens, "level up");
        }

        void OnPlayerCreatureKill(Player* killer, Creature* killed) override
        {
            if (!cfg.enable || !killed)
                return;
            Map* map = killed->GetMap();
            if (!map || !map->IsDungeon())
                return;
            bool raid = map->IsRaid();
            bool boss = killed->IsDungeonBoss() || (raid && killed->isWorldBoss());
            if (!boss)
                return;
            uint32 amount = raid ? cfg.raidBoss : (map->IsHeroic() ? cfg.heroicBoss : cfg.dungeonBoss);
            if (!amount)
                return;
            // key per difficulty so heroic counts again
            uint32 key = killed->GetEntry() * 4 + (map->GetSpawnMode() & 3);

            auto reward = [&](Player* p)
            {
                if (!p || !p->IsInWorld() || p->GetMap() != map)
                    return;
                uint32 guid = p->GetGUID().GetCounter();
                if (!sBosses[guid].insert(key).second)
                    return;
                CharacterDatabase.Execute("INSERT IGNORE INTO `mod_temucraft_shop_boss` (`guid`, `entry`) VALUES ({}, {})", guid, key);
                Give(p, amount, raid ? "first raid boss kill" : "first boss kill");
            };

            if (Group* group = killer->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                    reward(ref->GetSource());
            }
            else
                reward(killer);
        }

        void OnPlayerAchievementComplete(Player* player, AchievementEntry const* achievement) override
        {
            if (!cfg.enable || !cfg.achievementPointsPerToken || !achievement)
                return;
            uint32 tokens = achievement->points / cfg.achievementPointsPerToken;
            if (tokens)
                Give(player, tokens, "achievement");
        }
    };

    // ---------------------------------------------------------------- form glyphs
    class TemucraftShopForms : public UnitScript
    {
    public:
        TemucraftShopForms() : UnitScript("TemucraftShopForms", true, { UNITHOOK_ON_AURA_APPLY }) { }

        void OnAuraApply(Unit* unit, Aura* aura) override
        {
            if (!cfg.enable || !unit || !aura || unit->GetTypeId() != TYPEID_PLAYER)
                return;
            if (!aura->GetSpellInfo()->HasAura(SPELL_AURA_MOD_SHAPESHIFT))
                return;
            Player* player = unit->ToPlayer();
            auto it = sGlyphs.find(uint32(player->GetShapeshiftForm()));
            if (it == sGlyphs.end())
                return;
            for (FormGlyph const& g : it->second)
                if (player->HasAura(g.spell))
                {
                    uint32 display = player->GetTeamId() == TEAM_ALLIANCE ? g.alliance : g.horde;
                    player->SetDisplayId(display ? display : player->GetNativeDisplayId());
                    return;
                }
        }
    };

    // ---------------------------------------------------------------- commands
    class TemucraftShopCommands : public CommandScript
    {
    public:
        TemucraftShopCommands() : CommandScript("TemucraftShopCommands") { }

        ChatCommandTable GetCommands() const override
        {
            static ChatCommandTable shop =
            {
                { "",          HandleBalance,   101250,     Console::No },
                { "summon",    HandleSummon,    101251,     Console::No },
                { "guildname", HandleGuildName, 101252,     Console::No },
                { "add",       HandleAdd,       101201 /* SEC_GAMEMASTER -> RBAC */, Console::Yes },
                { "remove",    HandleRemove,    101201 /* SEC_GAMEMASTER -> RBAC */, Console::Yes },
                { "reload",    HandleReload,    101202 /* SEC_ADMINISTRATOR -> RBAC */, Console::Yes },
            };
            static ChatCommandTable root = { { "temucraft shop", shop } };
            return root;
        }

        static bool HandleBalance(ChatHandler* handler)
        {
            Player* player = handler->GetPlayer();
            uint32 n = Balance(player);
            handler->PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 21, "|cff66ccffTemucraft:|r you have {} token{}. Visit a Temucraft Curator by any capital's innkeeper{}."),
                n, n == 1 ? "" : "s", cfg.summon ? ", or type .temucraft shop summon" : "");
            return true;
        }

        static bool HandleSummon(ChatHandler* handler)
        {
            Player* player = handler->GetPlayer();
            if (!cfg.enable || !cfg.summon)
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 22, "Summoning the curator is turned off."));
                return true;
            }
            if (player->IsInCombat() || player->GetMap()->IsBattlegroundOrArena())
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 23, "Not now."));
                return true;
            }
            uint32 now = uint32(GameTime::GetGameTime().count());
            // drop cooldowns that ran out, so the map stays small
            for (auto it = sLastSummon.begin(); it != sLastSummon.end();)
                it = now >= it->second + cfg.summonCooldown ? sLastSummon.erase(it) : std::next(it);
            uint32& last = sLastSummon[player->GetGUID().GetCounter()];
            if (last && now < last + cfg.summonCooldown && !handler->GetSession()->GetSecurity())
            {
                handler->PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 24, "The curator is busy. Try again in {} seconds."), last + cfg.summonCooldown - now);
                return true;
            }
            float x, y, z;
            player->GetClosePoint(x, y, z, player->GetObjectSize(), 2.0f);
            if (TempSummon* s = player->SummonCreature(CURATOR_ENTRY, x, y, z, player->GetAngle(x, y) + M_PI, TEMPSUMMON_TIMED_DESPAWN, cfg.summonDuration * IN_MILLISECONDS))
            {
                s->SetFacingToObject(player);
                last = now;
            }
            return true;
        }

        static bool HandleGuildName(ChatHandler* handler, Tail name)
        {
            Player* player = handler->GetPlayer();
            Guild* guild = player->GetGuild();
            if (!guild || guild->GetLeaderGUID() != player->GetGUID())
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 25, "Only a guild master can rename the guild."));
                return true;
            }
            uint32 tokenItem = 0;
            for (auto const& [itemId, s] : sServices)
                if (s.type == SERVICE_GUILD_RENAME && player->HasItemCount(itemId, 1))
                    tokenItem = itemId;
            if (!tokenItem)
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 26, "You need a Guild Name Change Token."));
                return true;
            }
            std::string newName(name);
            if (newName.empty() || !ObjectMgr::IsValidCharterName(newName) || sGuildMgr->GetGuildByName(newName))
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 27, "That name is not valid or is taken."));
                return true;
            }
            if (!guild->SetName(newName))
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 28, "The guild could not be renamed."));
                return true;
            }
            player->DestroyItemCount(tokenItem, 1, true);
            handler->PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 29, "|cff66ccffTemucraft:|r your guild is now <{}>."), newName);
            return true;
        }

        static bool HandleAdd(ChatHandler* handler, uint32 count, Optional<PlayerIdentifier> target)
        {
            if (!target)
                target = PlayerIdentifier::FromTargetOrSelf(handler);
            if (!target || !target->IsConnected())
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 30, "Player not found or not online."));
                return true;
            }
            Give(target->GetConnectedPlayer(), count, "given by a game master");
            handler->PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 31, "Gave {} token(s) to {}."), count, target->GetName());
            return true;
        }

        static bool HandleRemove(ChatHandler* handler, uint32 count, Optional<PlayerIdentifier> target)
        {
            if (!target)
                target = PlayerIdentifier::FromTargetOrSelf(handler);
            if (!target || !target->IsConnected())
            {
                handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 30, "Player not found or not online."));
                return true;
            }
            Player* player = target->GetConnectedPlayer();
            uint32 taken = std::min(count, Balance(player));
            if (taken)
                player->DestroyItemCount(cfg.token, taken, true);
            handler->PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 38, "Took {} token(s) from {} ({} left)."), taken, target->GetName(), Balance(player));
            return true;
        }

        static bool HandleReload(ChatHandler* handler)
        {
            LoadConfig();
            LoadTables();
            handler->SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 32, "mod-temucraft-shop reloaded."));
            return true;
        }
    };

    // Saddlemaster's Handbooks: a riding rank for a quarter of the trainer's price, once another character of the
    // same account knows it. Sold by riding trainers for gold (data in mod_temucraft_shop_more.sql).
    struct Handbook { uint32 spell; uint32 previous; uint8 level; };
    std::unordered_map<uint32, Handbook> const HANDBOOKS =
    {
        { 610100, { 33391, 33388, 40 } },   // Journeyman Riding
        { 610101, { 34090, 33391, 60 } },   // Expert Riding
        { 610102, { 34091, 34090, 70 } },   // Artisan Riding
        { 610103, { 54197, 34090, 77 } },   // Cold Weather Flying
    };

    class item_temucraft_handbook : public ItemScript
    {
    public:
        item_temucraft_handbook() : ItemScript("item_temucraft_handbook") { }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
        {
            auto it = HANDBOOKS.find(item->GetEntry());
            if (it == HANDBOOKS.end())
                return false;
            Handbook const& h = it->second;
            ChatHandler handler(player->GetSession());
            if (player->HasSpell(h.spell))
                handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 39, "|cff66ccffTemucraft:|r you already know this."));
            else if (player->GetLevel() < h.level || !player->HasSpell(h.previous))
                handler.PSendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 40, "|cff66ccffTemucraft:|r you need level {} and the riding rank before this one."), h.level);
            else if (!CharacterDatabase.Query("SELECT 1 FROM character_spell s JOIN characters c ON c.guid = s.guid WHERE c.account = {} AND c.guid <> {} AND s.spell = {} LIMIT 1",
                         player->GetSession()->GetAccountId(), player->GetGUID().GetCounter(), h.spell))
                handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 41, "|cff66ccffTemucraft:|r another character of your account must know this first. Sell the book back and learn it from the trainer."));
            else
            {
                player->learnSpell(h.spell);
                player->DestroyItemCount(item->GetEntry(), 1, true);
                handler.SendSysMessage(TemucraftModuleText("mod-temucraft-shop", handler, 42, "|cff66ccffTemucraft:|r learned."));
            }
            return true;
        }
    };

    class TemucraftShopWorld : public WorldScript
    {
    public:
        TemucraftShopWorld() : WorldScript("TemucraftShopWorld", { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_STARTUP }) { }

        void OnAfterConfigLoad(bool /*reload*/) override { LoadConfig(); }
        void OnStartup() override { LoadTables(); }
    };
}

void AddTemucraftShopScripts()
{
    new TemucraftShop::TemucraftShopWorld();
    new TemucraftShop::npc_temucraft_shop_curator();
    new TemucraftShop::item_temucraft_shop_service();
    new TemucraftShop::item_temucraft_handbook();
    new TemucraftShop::TemucraftShopPlayer();
    new TemucraftShop::TemucraftShopForms();
    new TemucraftShop::TemucraftShopCommands();
}
