-- mod-temucraft-shop: its player commands (all under .temucraft) as RBAC permissions 101250-101299. Linked to the stock player
-- role 195 and the Temucraft player role 1010, so every account has them by default. To take one away, remove its
-- link here (or deny it per account with .rbac account deny <account> <id>); to give it to staff only, link it to
-- 1011-1014 instead.
DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 101250 AND 101299;
DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 101250 AND 101299;
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
(101250, 'Command: temucraft shop'),
(101251, 'Command: temucraft shop summon'),
(101252, 'Command: temucraft shop guildname');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
(195, 101250),
(1010, 101250),
(195, 101251),
(1010, 101251),
(195, 101252),
(1010, 101252);
