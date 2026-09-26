DELETE FROM `hayaku_base`.`stkweight`;
UPDATE `hayaku_base`.`coderuletype` SET `codepre`=51 WHERE `codepre`=510 AND `marketid`=1;
UPDATE `hayaku_base`.`coderuletype` SET `codepre`=50 WHERE `codepre`=500 AND `marketid`=1;
UPDATE `hayaku_base`.`coderuletype` SET `codepre`=20 WHERE `codepre`=200 AND `marketid`=2;
UPDATE `hayaku_base`.`version` set `version` = 22;