-- mod-temucraft-shop: the Toys, Auras and Helpers lists (the Auras goods come from mod-temucraft-auras, the helper items from mod-temucraft-courier) and stock mounts and companions added to the curator (stock 3.3.5 items, no
-- client change). Prices are the existing ItemExtendedCost rows: 3200 = 40, 3201 = 50, 3202 = 75, 3203 = 80,
-- 3205 = 100, 3208 = 150, 3209 = 180, 3213 = 2000 tokens. Stock items other vendors sell keep their gold price too.
SET @VENDOR := 608001;

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (@VENDOR+8, @VENDOR+9, @VENDOR+10);
DELETE FROM `creature_template` WHERE `entry` IN (@VENDOR+8, @VENDOR+9, @VENDOR+10);
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `unit_class`, `unit_flags`, `type`, `type_flags`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(@VENDOR+8, 'Temucraft Curator', 'Toys', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 2, 7, 0, 130, '', 0),
(@VENDOR+9, 'Temucraft Curator', 'Auras', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 2, 7, 0, 130, '', 0),
(@VENDOR+10, 'Temucraft Curator', 'Helpers', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 2, 7, 0, 130, '', 0);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
SELECT @VENDOR+8, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, 0 FROM `creature_template_model` WHERE `CreatureID` = @VENDOR
UNION ALL SELECT @VENDOR+9, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, 0 FROM `creature_template_model` WHERE `CreatureID` = @VENDOR
UNION ALL SELECT @VENDOR+10, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, 0 FROM `creature_template_model` WHERE `CreatureID` = @VENDOR;

-- Companions 109-113 and Illusions 100-102 are mod-temucraft-lostlands items (610200-610207, made on models we
-- already have): companions 75 tokens, illusion toys 80.
DELETE FROM `npc_vendor` WHERE `entry` = @VENDOR+8 OR (`entry` IN (@VENDOR+4, @VENDOR+5, @VENDOR+7) AND `slot` >= 100);
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) VALUES
(@VENDOR+4, 100, 38576, 0, 0, 3209, 0),
(@VENDOR+4, 101, 37719, 0, 0, 3209, 0),
(@VENDOR+4, 102, 23720, 0, 0, 3205, 0),
(@VENDOR+4, 103, 33225, 0, 0, 3213, 0),
(@VENDOR+5, 100, 49693, 0, 0, 3202, 0),
(@VENDOR+5, 101, 49362, 0, 0, 3202, 0),
(@VENDOR+5, 102, 34492, 0, 0, 3202, 0),
(@VENDOR+5, 103, 32588, 0, 0, 3202, 0),
(@VENDOR+5, 104, 56806, 0, 0, 3202, 0),
(@VENDOR+5, 105, 39656, 0, 0, 3202, 0),
(@VENDOR+5, 106, 39286, 0, 0, 3202, 0),
(@VENDOR+5, 107, 44819, 0, 0, 3202, 0),
(@VENDOR+5, 108, 49343, 0, 0, 3208, 0),
(@VENDOR+5, 109, 610200, 0, 0, 3202, 0),
(@VENDOR+5, 110, 610201, 0, 0, 3202, 0),
(@VENDOR+5, 111, 610202, 0, 0, 3202, 0),
(@VENDOR+5, 112, 610203, 0, 0, 3202, 0),
(@VENDOR+5, 113, 610204, 0, 0, 3202, 0),
(@VENDOR+7, 100, 610205, 0, 0, 3203, 0),
(@VENDOR+7, 101, 610206, 0, 0, 3203, 0),
(@VENDOR+7, 102, 610207, 0, 0, 3203, 0),
(@VENDOR+8, 100, 32566, 0, 0, 3201, 0),
(@VENDOR+8, 101, 34493, 0, 0, 3202, 0),
(@VENDOR+8, 102, 49287, 0, 0, 3202, 0),
(@VENDOR+8, 103, 33223, 0, 0, 3201, 0),
(@VENDOR+8, 104, 34499, 0, 0, 3200, 0),
(@VENDOR+8, 105, 32542, 0, 0, 3201, 0),
(@VENDOR+8, 106, 38578, 0, 0, 3201, 0),
(@VENDOR+8, 107, 46780, 0, 0, 3202, 0),
(@VENDOR+8, 108, 38301, 0, 0, 3202, 0),
(@VENDOR+8, 109, 45063, 0, 0, 3202, 0),
(@VENDOR+8, 110, 33219, 0, 0, 3202, 0),
(@VENDOR+8, 111, 38577, 0, 0, 3202, 0),
(@VENDOR+8, 112, 33079, 0, 0, 3203, 0),
(@VENDOR+8, 113, 18258, 0, 0, 3203, 0),
(@VENDOR+8, 114, 49704, 0, 0, 3203, 0),
(@VENDOR+8, 115, 45047, 0, 0, 3205, 0),
(@VENDOR+8, 116, 44606, 0, 0, 3202, 0),
(@VENDOR+8, 117, 45057, 0, 0, 3202, 0),
(@VENDOR+8, 118, 33927, 0, 0, 3202, 0),
(@VENDOR+8, 119, 46779, 0, 0, 3202, 0),
(@VENDOR+8, 120, 44430, 0, 0, 3201, 0),
(@VENDOR+8, 121, 21540, 0, 0, 3201, 0),
(@VENDOR+8, 122, 37254, 0, 0, 3202, 0),
(@VENDOR+8, 123, 44482, 0, 0, 3202, 0),
(@VENDOR+8, 124, 35227, 0, 0, 3202, 0),
(@VENDOR+8, 125, 40110, 0, 0, 3202, 0);

-- Saddlemaster's Handbooks: a riding rank for a quarter of the trainer's gold price, once another character of the
-- account knows it (checked when used; the book can be sold back for what it cost). Sold by every riding trainer.
SET @BOOK := 610100;
DELETE FROM `item_template` WHERE `entry` BETWEEN @BOOK AND @BOOK+3;
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `ItemLevel`, `RequiredLevel`, `maxcount`, `stackable`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `bonding`, `description`, `Material`, `ScriptName`) VALUES
(@BOOK+0, 0, 8, -1, 'Saddlemaster\'s Handbook: Journeyman', 13108, 2, 0, 1, 125000, 125000, 0, 40, 40, 1, 1, 18282, 0, -1, 1, 'Teaches Journeyman Riding if another character of your account knows it.', 4, 'item_temucraft_handbook'),
(@BOOK+1, 0, 8, -1, 'Saddlemaster\'s Handbook: Expert', 13108, 3, 0, 1, 625000, 625000, 0, 60, 60, 1, 1, 18282, 0, -1, 1, 'Teaches Expert Riding if another character of your account knows it.', 4, 'item_temucraft_handbook'),
(@BOOK+2, 0, 8, -1, 'Saddlemaster\'s Handbook: Artisan', 37425, 4, 0, 1, 12500000, 12500000, 0, 70, 70, 1, 1, 18282, 0, -1, 1, 'Teaches Artisan Riding if another character of your account knows it.', 4, 'item_temucraft_handbook'),
(@BOOK+3, 0, 8, -1, 'Saddlemaster\'s Handbook: Cold Weather', 37425, 3, 0, 1, 2500000, 2500000, 0, 77, 77, 1, 1, 18282, 0, -1, 1, 'Teaches Cold Weather Flying if another character of your account knows it.', 4, 'item_temucraft_handbook');

-- riding trainers become vendors too, and sell the books they teach the rank of
DROP TEMPORARY TABLE IF EXISTS `tmp_temucraft_riding`;
CREATE TEMPORARY TABLE `tmp_temucraft_riding` (`entry` INT UNSIGNED PRIMARY KEY, `artisan` TINYINT, `cold` TINYINT);
INSERT IGNORE INTO `tmp_temucraft_riding` (`entry`, `artisan`, `cold`)
SELECT d.`CreatureId`, MAX(s.`SpellId` = 34091), MAX(s.`SpellId` = 54197) FROM `creature_default_trainer` d
JOIN `trainer_spell` s ON s.`TrainerId` = d.`TrainerId` WHERE s.`SpellId` IN (33391, 34090, 34091, 54197) GROUP BY d.`CreatureId`;
UPDATE `creature_template` SET `npcflag` = `npcflag` | 128 WHERE `entry` IN (SELECT `entry` FROM `tmp_temucraft_riding`);
DELETE FROM `npc_vendor` WHERE `item` BETWEEN @BOOK AND @BOOK+3;
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`)
SELECT `entry`, 0, @BOOK+0, 0, 0, 0, 0 FROM `tmp_temucraft_riding`
UNION ALL SELECT `entry`, 1, @BOOK+1, 0, 0, 0, 0 FROM `tmp_temucraft_riding`
UNION ALL SELECT `entry`, 2, @BOOK+2, 0, 0, 0, 0 FROM `tmp_temucraft_riding` WHERE `artisan` = 1
UNION ALL SELECT `entry`, 3, @BOOK+3, 0, 0, 0, 0 FROM `tmp_temucraft_riding` WHERE `cold` = 1;
DROP TEMPORARY TABLE `tmp_temucraft_riding`;

-- Helpers list (vendor 608011): the courier's helper items (mod-temucraft-courier, 610140-610143), the Banker's Writ
-- and the Guild Vault Charter (service tokens 6 and 7). The helper items are put on the list by mod-temucraft-courier's own SQL.
SET @WRIT := 610144; SET @CHARTER := 610145;
DELETE FROM `item_template` WHERE `entry` IN (@WRIT, @CHARTER);
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `ItemLevel`, `maxcount`, `stackable`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, `bonding`, `description`, `Material`, `ScriptName`) VALUES
(@WRIT, 15, 0, -1, 'Banker''s Writ', 1093, 3, 0, 1, 0, 0, 0, 1, 0, 20, 18282, 0, 0, 1500, 1, 'Opens every bank bag slot you have not bought yet. Used up.', 7, 'item_temucraft_shop_service'),
(@CHARTER, 15, 0, -1, 'Guild Vault Charter', 32845, 3, 0, 1, 0, 0, 0, 1, 0, 20, 18282, 0, 0, 1500, 1, 'Your guild gets its next guild bank tab without gold. Guild master only. Used up.', 7, 'item_temucraft_shop_service');
DELETE FROM `mod_temucraft_shop_service` WHERE `ItemID` IN (@WRIT, @CHARTER);
INSERT INTO `mod_temucraft_shop_service` (`ItemID`, `Service`, `Race`) VALUES (@WRIT, 6, 0), (@CHARTER, 7, 0);
DELETE FROM `npc_vendor` WHERE `entry` = @VENDOR+10 AND `item` IN (@WRIT, @CHARTER);
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) VALUES
(@VENDOR+10, 4, @WRIT, 0, 0, 3208, 0),
(@VENDOR+10, 5, @CHARTER, 0, 0, 3211, 0);
