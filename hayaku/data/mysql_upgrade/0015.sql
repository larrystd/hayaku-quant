ALTER TABLE `hayaku_base`.`coderuletype` modify `id` int(11)  auto_increment;
INSERT INTO `hayaku_base`.`coderuletype` (`marketid`,`codepre`,`type`,`description`) VALUES (3,'92',11,'北证A股');
INSERT INTO `hayaku_base`.`coderuletype` (`marketid`,`codepre`,`type`,`description`) VALUES (1,'880',2,'通达信板块指数');
UPDATE `hayaku_base`.`version` set `version` = 15;