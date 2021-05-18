-- mydb.usuario definition

CREATE TABLE `usuario` (
  `socket` int(11) DEFAULT NULL,
  `username` varchar(255) NOT NULL,
  `status` int(11) DEFAULT NULL,
  PRIMARY KEY (`username`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


-- mydb.mensagens definition

CREATE TABLE `mensagens` (
  `remetente` varchar(255) NOT NULL,
  `mensagem` varchar(255) NOT NULL,
  `id_username` varchar(255) DEFAULT NULL,
  KEY `id_username` (`id_username`),
  CONSTRAINT `mensagens_ibfk_1` FOREIGN KEY (`id_username`) REFERENCES `usuario` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;