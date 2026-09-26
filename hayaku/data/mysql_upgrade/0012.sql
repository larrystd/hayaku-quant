UPDATE `hayaku_base`.`stocktypeinfo` SET `minTradeNumber`=1 where `id`=9;
INSERT INTO `hayaku_base`.`stocktypeinfo` (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (11, 11, 2, 0.01, 0.01, 1, 1000000, '北交所');
UPDATE `hayaku_base`.`stock` set `type`=1 where `marketid`=3;

UPDATE `hayaku_base`.`version` set `version` = 12;