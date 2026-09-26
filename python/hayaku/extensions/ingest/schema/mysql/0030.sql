update `hayaku_base`.`stocktypeinfo` set `minTradeNumber`=200 where `type`=9 and `description`='科创板';
update `hayaku_base`.`stocktypeinfo` set `minTradeNumber`=100 where `type`=11 and `description`='北交所';
UPDATE `hayaku_base`.`version` set `version` = 30;