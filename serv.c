#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>

MYSQL *conn;

typedef struct Users
{
	char name[100];
} users;
typedef struct Message
{
	int messageType; //tipo da requisição ex: envio, get users...
	int countUserOnline;
	char user[100];
	char data[1024];
	char destName[100];
	int msgLen;
	int port;
	users userOnline[100];
} message;

int sockID;
struct sockaddr_in clientAddr;
int length = 0;
int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client
{
	int sockID;
};

struct client Client;
pthread_t thread[1024];

void registerDb(char msg[], int clientSocket)
{
	char query[100] = "INSERT INTO mydb.usuario values(";
	char sockString[10];
	sprintf(sockString, "%i", clientSocket);
	strncat(query, sockString, 99);
	strncat(query, ", \"", 99);
	strncat(query, msg, 99);
	strncat(query, "\", ", 99);
	strncat(query, "1)", 2);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
}

void alterAvailability(char *user, char *status)
{
	char query[100] = "UPDATE mydb.usuario SET status = ";
	strncat(query, status, 99);
	strncat(query, " where username = ", 99);

	strncat(query, "\"", 99);
	strncat(query, user, 99);
	strncat(query, "\"", 99);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
}
void deleteFieldBd(char *user)
{
	char query[100] = "DELETE FROM mydb.mensagens WHERE id_username = \"";
	strncat(query, user, 99);
	strncat(query, "\"", 99);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
}
void searchMessageSendedBeforeLogin(char *user, int sockUser)
{

	char query[100] = "SELECT remetente, mensagem FROM mydb.mensagens WHERE id_username = \"";
	strncat(query, user, 99);
	strncat(query, "\"", 99);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	int j = 0;
	message msg;

	while ((row = mysql_fetch_row(result)))
	{
		bzero(&msg, sizeof(message));
		msg.messageType = 2;
		if (row[0] != NULL)
		{
			strncpy(msg.data, row[0], strlen(row[0]));
			strcpy(msg.user, row[0]);

			snprintf(msg.data, 99, "%s", row[1]);
			msg.msgLen = strlen(msg.data);
			send(sockUser, (char *)&msg, sizeof(msg), 0);
		}
	}
	deleteFieldBd(user);
	return 0;
}

int insertFriend(char *user, char *friend)
{
	char query[100] = "INSERT INTO mydb.amigos (idusuario, idusuario1) values (\"";
	strncat(query, user, 99);
	strncat(query, "\",\"", 99);
	strncat(query, friend, 99);
	strncat(query, "\") ", 99);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return -1;
	}
	return 0;
}
int sendMessageUserOffiline(char *remetente, char *msg, char *user)
{
	char query[200] = "INSERT INTO mydb.mensagens (remetente, mensagem, id_username) VALUES(\"";
	strncat(query, remetente, 200);
	strncat(query, "\", \"", 200);
	strncat(query, msg, 200);
	strncat(query, "\", \"", 200);
	strncat(query, user, 200);
	strncat(query, "\")", 200);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return -1;
	}
	return 0;
}

int listFriends(message *msg, char *user)
{
	char query[200] = "SELECT username , status FROM mydb.usuario INNER JOIN mydb.amigos ON idusuario1 = usuario.username WHERE idusuario =\"";
	strncat(query, user, 200);
	strncat(query, "\"", 200);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return 0;
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	int j = 0;
	
	while (row = mysql_fetch_row(result))
	{
	
		if (row[0] != NULL)
		{
			strncpy(msg->userOnline[j].name, row[0], strlen(row[0]));
			if (strcmp(row[1], "0") == 0)
			{

				strncat(msg->userOnline[j].name, " - offline", strlen(row[1]) + strlen(" - offline"));
			}
			else{

				 strncat(msg->userOnline[j].name, " - online", strlen(row[1]) + strlen(" - online"));
			}

			j++;
		}
		printf("\n");
	}

	msg->countUserOnline = j;
	return 1;
}

void alterSockUser(char *user, int sock)
{
	char query[100] = "UPDATE mydb.usuario SET socket = "; //é socket?
	char sockString[100];
	sprintf(sockString, "%i", sock);
	strncat(query, sockString, 99);
	strncat(query, " where username = ", 99);

	strncat(query, "\"", 99);
	strncat(query, user, 99);
	strncat(query, "\"", 99);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
}
struct SockAndStatus
{
	int sock;
	int status;
} typedef sockAndStatus;

sockAndStatus searchSockUserDb(char *user)
{
	sockAndStatus sockAndStatusVar;
	char query[100] = "SELECT socket,status from mydb.usuario where username =\"";
	strncat(query, user, 99);
	strncat(query, "\"", 50);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	row = mysql_fetch_row(result);
	if (row != NULL)
	{
		sockAndStatusVar.sock = atoi(row[0]);
		sockAndStatusVar.status = atoi(row[1]);

		return sockAndStatusVar;
	}
	sockAndStatusVar.status = -1;
	return sockAndStatusVar;
}

int searchAndSetNewSockUserDb(char *user, int sock)
{
	char query[100] = "SELECT socket from mydb.usuario where username =\"";
	strncat(query, user, 99);
	strncat(query, "\"", 50);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	row = mysql_fetch_row(result);
	if (row != NULL)
	{
		alterAvailability(user, "1");
		alterSockUser(user, sock);
		return atoi(row[0]);
	}

	return -1;
}
void sendMessageAllUserOnline(char *user, char *msg)
{
	char query[100];
	snprintf(query, 99, "SELECT socket from mydb.usuario where status = 1");
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return 0;
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	int j = 0;
	message msgSend;
	msgSend.messageType = 7;
	sprintf(msgSend.data, "[%s] - %s", user, msg);
	msgSend.msgLen = strlen(msgSend.data);
	while ((row = mysql_fetch_row(result)))
	{
		if (row[0] != NULL)
		{

			send(atoi(row[0]), (char *)&msgSend, sizeof(msgSend), 0);
		}
		printf("\n");
	}
}
int searchAllOnlineUsers(message *msg)
{
	char query[100];
	snprintf(query, 99, "SELECT username from mydb.usuario where status = 1");
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return 0;
	}
	MYSQL_RES *result;
	result = mysql_store_result(conn);

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	int j = 0;
	while ((row = mysql_fetch_row(result)))
	{
		for (int i = 0; i < num_fields; i++)
		{
			if (row[i] != NULL)
			{
				strncpy(msg->userOnline[j].name, row[i], strlen(row[i]));
				j++;
			}
		}
	}
	msg->countUserOnline = j;
	return 1;
}

void *doNetworking(void *ClientDetail)
{
	message msg;
	struct client *client = (struct client *)ClientDetail;
	int clientSocket = client->sockID;
	int exit = 1;
	while (exit == 1)
	{

		char data[1024];

		char output[1024];
		memset(&msg, '\0', sizeof(msg));

		int rtn = recv(clientSocket, (char *)&msg, sizeof(msg), 0);

		if (msg.messageType == 1)
		{
			int existClientInDataBase = searchAndSetNewSockUserDb(msg.user, clientSocket);
			if (existClientInDataBase == -1)
			{
				registerDb(msg.user, clientSocket);
			}
			message msgSend;
			msgSend.messageType = 1;
			strncpy(msgSend.data, "TUDO CONECTADO", strlen("TUDO CONECTADO"));

			send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);

			searchMessageSendedBeforeLogin(msg.user, clientSocket);
		}
		if (msg.messageType == 2)
		{
			message msgSend;
			msgSend.messageType = 2;
			strncpy(msgSend.user, msg.user, strlen(msg.user));
			strncpy(msgSend.destName, msg.destName, strlen(msg.destName));
			strncpy(msgSend.data, msg.data, strlen(msg.data));
			msgSend.msgLen = msg.msgLen;
			sockAndStatus socketUserDestin = searchSockUserDb(msg.destName);

			if (socketUserDestin.status == 1){
				send(socketUserDestin.sock, (char *)&msgSend, sizeof(msgSend), 0);
			}else if (socketUserDestin.status == -1){
				msgSend.messageType = 5;
				strncpy(msgSend.data, "Error Socket", strlen("Error Socket"));
				msgSend.msgLen = strlen("Error Socket");
				send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
			} else{
				int status = sendMessageUserOffiline(msg.user, msg.data, msg.destName);
			}
		}
		if (msg.messageType == 3)
		{
			message msgSend;
			bzero(&msgSend, sizeof(message));
			int status = searchAllOnlineUsers(&msgSend);
			if (status)
			{
				for (int i = 0; i < msgSend.countUserOnline; i++)
				{ 
					printf("User: %s\n", msgSend.userOnline[i].name);
				}
			}
			msgSend.messageType = 3;
			send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
		}
		if (msg.messageType == 4)
		{
			alterAvailability(msg.user, "0");
			message msgSend;
			msgSend.messageType = 4;
			send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
			close(clientSocket);
			exit = 0;
		}
		if (msg.messageType == 5)
		{ 
			message msgSend;
			int retorno = insertFriend(msg.user, msg.data);
			msgSend.messageType = 5;
			if (retorno == -1)
			{
				strncpy(msgSend.data, "Erro ao adicionar um amigo", strlen("Erro ao adicionar um amigo"));
				msgSend.msgLen = strlen("Erro ao adicionar um amigo");
				send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
			}
			else
			{
				strncpy(msgSend.data, "Adicionado com sucesso", strlen("Adicionado com sucesso"));
				msgSend.msgLen = strlen("Adicionado com sucesso");
				send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
			}
		}
		if (msg.messageType == 6)
		{
			message msgSend;
			bzero(&msgSend, sizeof(message));
			int retorno = listFriends(&msgSend, msg.user);
			msgSend.messageType = 6;
			send(clientSocket, (char *)&msgSend, sizeof(msgSend), 0);
		}
		if (msg.messageType == 7)
		{
			sendMessageAllUserOnline(msg.user, msg.data);
		}
		if (msg.messageType == 8)
		{
			message msgSend;
			sockAndStatus socketUserDestin = searchSockUserDb(msg.destName);
			msgSend.messageType = 8;
			msgSend.port = msg.port;
			strcpy(msgSend.data,msg.data);
			send(socketUserDestin.sock, (char *)&msgSend, sizeof(msgSend), 0);
		}
	}

	return NULL;
}

int connectionDb()
{
	if ((conn = mysql_init(NULL)) == NULL)
	{
		fprintf(stderr, "Could not init DB\n");
		return EXIT_FAILURE;
	}
	if (mysql_real_connect(conn, "192.168.0.106", "user", "user", "mydb", 3306, NULL, 0) == NULL)
	{
		fprintf(stderr, "DB Connection Error\n");
		return EXIT_FAILURE;
	}
	return 1;
}

int main()
{
	connectionDb();

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	if (listen(serverSocket, 1024) == -1)
		return 0;

	printf("Server started listenting on port 8888 ...........\n");

	while (1)
	{
		Client.sockID = accept(serverSocket, (struct sockaddr *)&clientAddr, &length);
		pthread_create(&thread[clientCount], NULL, doNetworking, (void *)&Client);

		clientCount++;
	}

	for (int i = 0; i < clientCount; i++)
		pthread_join(thread[i], NULL);
	mysql_close(conn);
}