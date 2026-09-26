delete from `hayaku_base`.`stkweight` where 1=1;
alter table `hayaku_base`.`stkweight` AUTO_INCREMENT=1;
alter table `hayaku_base`.`stkweight` modify column `countAsGift` DOUBLE not null default 0;
alter table `hayaku_base`.`stkweight` modify column `countForSell` DOUBLE not null default 0;
UPDATE `hayaku_base`.`version` set `version` = 17;
