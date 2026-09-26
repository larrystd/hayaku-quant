INSERT INTO `hayaku_base`.`coderuletype` (`marketid`, `codepre`, `type`, `description`)
SELECT 2, '302', 8, '创业板'
WHERE NOT EXISTS (
    SELECT id FROM `hayaku_base`.`coderuletype` WHERE `codepre`=302
);
UPDATE `hayaku_base`.`version` set `version` = 27;