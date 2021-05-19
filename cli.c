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

// na hr de conectar verificar msgs que lhe foram recebidas enquanto estava offline
// quando sair setar flag do banco para 0
// typedef struct Message
// {
//     int messageType[2];//tipo da requisição ex: envio, get users...
// 	//int countUserOnline;
//     char user[100];
//     char data[1024];
//     char destName[100];
// 	//char originName[1024];
//     char userOnline[100][1024];
// } message;

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
	//char originName[1024];
	users userOnline[100];
} message;

/*messageType[0] Para serv
1- cadastra no server
2- envia a message para alguem
3- lista de usuarios
4- desloga
messagetype[1] contador de qtd de usuarios
*/

/*messageType Para CLiente
1- status de logado
2- mensagem de alguem recebida
3- imprime os usuarios online
*/
void *doRecieving(void *sockID)
{

	int clientSocket = *((int *)sockID);
	int exit = 1;
	while (exit == 1)
	{

		message data;
		int read = recv(clientSocket, (char *)&data, sizeof data, 0);
		//printf("PPPPP %d - %d\n",data.messageType, data.countUserOnline );

		if (data.messageType == 1)
		{
			printf("\nUsuarios Online :\n");
			printf("- %s\n", data.data);
		}

		if (data.messageType == 2)
		{
			printf("[%s] - ", data.user);

			for (int i = 0; i < data.msgLen; i++)
			{
				printf("%c", data.data[i]);
			}
			printf("\n");
		}

		if (data.messageType == 3)
		{
			printf("\nUsuarios Online :\n");
			for (int i = 0; i < data.countUserOnline; i++)
			{
				printf("- %s\n", data.userOnline[i].name);
			}
		}

		if (data.messageType == 4)
		{
			exit = 0;
		}

		if (data.messageType == 5)
		{
			printf("\nOPAAAAAAA\n");
			printf("\nMensagem de retorno: \n");
			for (int i = 0; i < data.msgLen; i++)
			{
				printf("%c", data.data[i]);
			}
			printf("\n");
		}
	}
}

int main()
{

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	message msg;
	memset(&msg, '\0', sizeof(msg));
	printf("\n%d", sizeof(msg));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, doRecieving, (void *)&clientSocket);
	printf("Login do Servidor...\n");
	printf("Digite seu Username: ");
	scanf("%[^\n]s", msg.user);

	msg.messageType = 1;
	int rtn = send(clientSocket, (char *)&msg, sizeof(msg), 0);
	char userChat[100];

	int option;
	char flagExit;
	int exit = 1;
	while (exit == 1)
	{
		printf("\nOpções:\n");
		printf("1 - Enviar mensagem para um usuário online(Para ver quem esta online escolha a opcao 2)\n");
		printf("2 - Exibir Lista de amigos\n");
		printf("3 - Exibir Lista de usuários online\n");
		printf("4 - sair");
		printf("5 - Adicionar um amigo pelo username\n");
		printf("\nOpcao : ");
		scanf("%d", &option);
		if (option == 1)
		{
			__fpurge(stdin);
			printf("\nPara sair digite - \"exit\" - ");
			printf("\nDigite o user para envio: ");
			scanf("%[^\n]s", msg.destName);
			while (1)
			{
				__fpurge(stdin);
				scanf("%[^\n]s", msg.data);
				printf("\n------%d\n\n\n", strlen(msg.data));
				msg.msgLen = strlen(msg.data);
				if (strcmp(msg.data, "exit") == 0)
				{
					break;
				}
				msg.messageType = 2;

				send(clientSocket, (char *)&msg, sizeof(msg), 0);
			}
		}
		if (option == 3)
		{
			msg.messageType = 3;
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
		}
		if (option == 4)
		{
			msg.messageType = 4;
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			close(clientSocket);
			exit = 0;
		}
		if (option == 5)
		{
			__fpurge(stdin);
			printf("\nDigite o user para adicionar: ");
			scanf("%[^\n]s", msg.data);
			printf("\n------%d\n\n\n", strlen(msg.data));
			msg.msgLen = strlen(msg.data);
			msg.messageType = 5;
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
		}
		if (strcmp(msg.data, "exit") != 0)
		{
			printf("\n[ATENCAO] digite uma opcao valida\n");
			sleep(2);
		}
		__fpurge(stdin);
	}
}