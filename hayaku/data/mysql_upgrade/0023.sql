DELETE FROM `hayaku_base`.`stkweight`;
alter table `hayaku_base`.`stkweight` add `suogu` DOUBLE not null default 0;
ALTER TABLE `hayaku_base`.`stkweight` AUTO_INCREMENT = 1;
UPDATE `hayaku_base`.`version` set `version` = 23;