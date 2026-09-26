ALTER TABLE `hayaku_base`.`coderuletype` modify `id` int(11)  auto_increment;
INSERT INTO `hayaku_base`.`coderuletype` (`marketid`,`codepre`,`type`,`description`) VALUES (3,'899',2,'北证指数');
UPDATE `hayaku_base`.`market` SET `code`='899050' WHERE `marketid`=3;
INSERT INTO `hayaku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`)
SELECT 3, '899050', '北证50', 2, 1, 20220429, 99999999
WHERE NOT EXISTS (
    SELECT `stockid` FROM `hayaku_base`.`stock` WHERE `marketid`=3 and `code`='899050'
);
UPDATE `hayaku_base`.`version` set `version` = 26;