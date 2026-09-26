CREATE TABLE IF NOT EXISTS `hayaku_base`.`zh_bond10` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `date` INT UNSIGNED NOT NULL, `value` INT NOT NULL, PRIMARY KEY (`id`), INDEX `ix_date_on_zh_bond10` (`date`)
) COLLATE = 'utf8mb4_general_ci' ENGINE = InnoDB;

UPDATE `hayaku_base`.`version` set `version` = 11;