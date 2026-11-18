-- mod-temucraft-shop: its game master commands as RBAC permissions (101201-101202). Each is linked to the
-- stock security role of its old level (194 moderator, 193 game master, 192 administrator) and to the matching
-- Temucraft GM tier (1011 Triager, 1012 Entertainer, 1014 Administrator). Grant or deny them per account with
-- .rbac account grant / deny, or link them to other roles.
DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 101201 AND 101299;
DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 101201 AND 101299;
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
(101201, 'Command: shop add'),
(101202, 'Command: shop reload');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
(193, 101201),
(1012, 101201),
(192, 101202),
(1014, 101202);
