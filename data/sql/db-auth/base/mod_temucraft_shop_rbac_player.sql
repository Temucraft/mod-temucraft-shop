-- mod-temucraft-shop: its commands (all under .temucraft) as RBAC permissions 101250-101299. Every command is admin only by
-- default: linked to the administrator roles 192 and 1014. To give one to players, link it to 195 and 1010 here (or
-- grant it per account with .rbac account grant <account> <id>). "Temucraft: use ..." permissions are not
-- commands: they stay linked to the player roles so the items keep working.
DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 101250 AND 101299;
DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 101250 AND 101299;
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
(101250, 'Command: temucraft shop'),
(101251, 'Command: temucraft shop summon'),
(101252, 'Command: temucraft shop guildname');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
(192, 101250),
(1014, 101250),
(192, 101251),
(1014, 101251),
(192, 101252),
(1014, 101252);
