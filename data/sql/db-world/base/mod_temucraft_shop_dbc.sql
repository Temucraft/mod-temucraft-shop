-- mod-temucraft-shop: DBC rows for the server (the client needs the same rows, see client/). Re-runnable.
-- currencytypes_dbc: 1 row(s) mod-temucraft-shop
DELETE FROM `currencytypes_dbc` WHERE `ID` = 343;
INSERT INTO `currencytypes_dbc` (`ID`, `ItemID`, `CategoryID`, `BitIndex`) VALUES (343,608000,1,30);

-- itemextendedcost_dbc: 14 row(s) mod-temucraft-shop
DELETE FROM `itemextendedcost_dbc` WHERE `ID` BETWEEN 3200 AND 3213;
INSERT INTO `itemextendedcost_dbc` (`ID`, `HonorPoints`, `ArenaPoints`, `ArenaBracket`, `ItemID_1`, `ItemID_2`, `ItemID_3`, `ItemID_4`, `ItemID_5`, `ItemCount_1`, `ItemCount_2`, `ItemCount_3`, `ItemCount_4`, `ItemCount_5`, `RequiredArenaRating`, `ItemPurchaseGroup`) VALUES
(3200,0,0,0,608000,0,0,0,0,40,0,0,0,0,0,0),
(3201,0,0,0,608000,0,0,0,0,50,0,0,0,0,0,0),
(3202,0,0,0,608000,0,0,0,0,75,0,0,0,0,0,0),
(3203,0,0,0,608000,0,0,0,0,80,0,0,0,0,0,0),
(3204,0,0,0,608000,0,0,0,0,90,0,0,0,0,0,0),
(3205,0,0,0,608000,0,0,0,0,100,0,0,0,0,0,0),
(3206,0,0,0,608000,0,0,0,0,125,0,0,0,0,0,0),
(3207,0,0,0,608000,0,0,0,0,130,0,0,0,0,0,0),
(3208,0,0,0,608000,0,0,0,0,150,0,0,0,0,0,0),
(3209,0,0,0,608000,0,0,0,0,180,0,0,0,0,0,0),
(3210,0,0,0,608000,0,0,0,0,200,0,0,0,0,0,0),
(3211,0,0,0,608000,0,0,0,0,250,0,0,0,0,0,0),
(3212,0,0,0,608000,0,0,0,0,500,0,0,0,0,0,0),
(3213,0,0,0,608000,0,0,0,0,2000,0,0,0,0,0,0);
