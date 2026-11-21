-- mod-temucraft-shop: the Toys list and stock mounts and companions added to the curator (stock 3.3.5 items, no
-- client change). Prices are the existing ItemExtendedCost rows: 3200 = 40, 3201 = 50, 3202 = 75, 3203 = 80,
-- 3205 = 100, 3208 = 150, 3209 = 180, 3213 = 2000 tokens. Stock items other vendors sell keep their gold price too.
SET @VENDOR := 608001;

DELETE FROM `creature_template_model` WHERE `CreatureID` = @VENDOR+8;
DELETE FROM `creature_template` WHERE `entry` = @VENDOR+8;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `unit_class`, `unit_flags`, `type`, `type_flags`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(@VENDOR+8, 'Temucraft Curator', 'Toys', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 2, 7, 0, 130, '', 0);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
SELECT @VENDOR+8, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, 0 FROM `creature_template_model` WHERE `CreatureID` = @VENDOR;

DELETE FROM `npc_vendor` WHERE `entry` = @VENDOR+8 OR (`entry` IN (@VENDOR+4, @VENDOR+5) AND `slot` >= 100);
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
(@VENDOR+8, 124, 35227, 0, 0, 3202, 0);
