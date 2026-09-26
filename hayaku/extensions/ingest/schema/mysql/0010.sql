CREATE TABLE IF NOT EXISTS `hayaku_base`.`block` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `category` VARCHAR(100) NOT NULL, `name` VARCHAR(100) NOT NULL, `market_code` VARCHAR(30) NOT NULL, PRIMARY KEY (`id`), INDEX `ix_block` (`category`, `name`)
) COLLATE = 'utf8mb4_general_ci' ENGINE = InnoDB;

UPDATE `hayaku_base`.`version` set `version` = 10;