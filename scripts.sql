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


CREATE TABLE `amigos` (
  `idusuario` varchar(255) NOT NULL,
  `idusuario1` varchar(255) NOT NULL,
  PRIMARY KEY (`user`, `users`),
  CONSTRAINT `user_fk` FOREIGN KEY (`idusuario`) REFERENCES `usuario` (`username`)
  CONSTRAINT `Users_fk` FOREIGN KEY (`idusuario1`) REFERENCES `usuario` (`username`)
);


SELECT socket, username , status 
FROM usuario
INNER JOIN amigos
ON idusuario = "A";

SELECT NOME, SEXO, BAIRRO, CIDADE /* PROJECAO */ 
FROM CLIENTE /* ORIGEM */
	INNER JOIN ENDERECO /*JUNCAO */
	ON IDCLIENTE = ID_CLIENTE
WHERE SEXO = 'F'; /* SELECAO */

SELECT socket, username , status 
FROM usuario
INNER JOIN amigos
ON idusuario1 = usuario.username 
WHERE idusuario = "A";