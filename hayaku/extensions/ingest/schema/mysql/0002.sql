alter table `hayaku_base`.`stocktypeinfo` modify column `minTradeNumber` double not null default 1;
alter table `hayaku_base`.`stocktypeinfo` modify column `maxTradeNumber` double not null default 1;
UPDATE `hayaku_base`.`version` set `version` = 2;
