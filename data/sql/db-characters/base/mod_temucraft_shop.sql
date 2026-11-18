-- mod-temucraft-shop: what each character has already been paid for
CREATE TABLE IF NOT EXISTS `mod_temucraft_shop_progress` (
  `guid` int unsigned NOT NULL,
  `MaxLevel` tinyint unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`guid`)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `mod_temucraft_shop_boss` (
  `guid` int unsigned NOT NULL,
  `entry` int unsigned NOT NULL COMMENT 'creature entry * 4 + difficulty',
  PRIMARY KEY (`guid`, `entry`)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
