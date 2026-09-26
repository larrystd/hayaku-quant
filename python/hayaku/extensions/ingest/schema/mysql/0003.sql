ALTER TABLE `hayaku_base`.`market` ADD `openTime1` INT(10) UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `hayaku_base`.`market` ADD `closeTime1` INT(10) UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `hayaku_base`.`market` ADD `openTime2` INT(10) UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `hayaku_base`.`market` ADD `closeTime2` INT(10) UNSIGNED NULL DEFAULT NULL;

UPDATE `hayaku_base`.`market` SET `openTime1`=930, `closeTime1`=1130, `openTime2`=1300, `closeTime2`=1500 WHERE `marketid`=1;
UPDATE `hayaku_base`.`market` SET `openTime1`=930, `closeTime1`=1130, `openTime2`=1300, `closeTime2`=1500 WHERE `marketid`=2;

UPDATE `hayaku_base`.`version` set `version` = 3;