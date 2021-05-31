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
#include <netdb.h>
#include <errno.h>


char fname[100];
int serverSocket;
struct sockaddr_in clientAddr;

typedef struct Users
{
	char name[100];
} users;

typedef struct Message
{
	int messageType; 
	int countUserOnline;
	char user[100];
	char data[1024];
	char destName[100];
	int msgLen;
	int port;
	users userOnline[100];
} message;

void gotoxy(int x, int y)
{
	printf("%c[%d;%df", 0x1B, y, x);
}

void *doRecieving(void *sockID)
{

	int clientSocket = *((int *)sockID);
	int exit = 1;
	while (exit == 1)
	{

		message data;
		recv(clientSocket, (char *)&data, sizeof data, 0);

		if (data.messageType == 1)
		{
			printf("\nStatus :\n");
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
			printf("\nMensagem de retorno: \n");
			for (int i = 0; i < data.msgLen; i++)
			{
				printf("%c", data.data[i]);
			}
			printf("\n");
		}

		if (data.messageType == 6)
		{
			printf("\nAmigos online: \n");
			for (int i = 0; i < data.countUserOnline; i++)
			{
				printf("- %s \n", data.userOnline[i].name);
			}
			printf("\n");
		}
		if (data.messageType == 7)
		{
			printf("\nMensagem do Grupo de Onlines: \n");
			for (int i = 0; i < data.msgLen; i++)
			{
				printf("%c", data.data[i]);
			}
			printf("\n");
		}

		if (data.messageType == 8)
		{
			system("clear");
			struct sockaddr_in serv_addr;
			int sockfd;
			/* Create a socket first */
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("\n Error : Could not create socket \n");
				return 1;
			}

			/* Initialize sockaddr_in data structure */
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(data.port); 
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
			{
				printf("\n Error : Connect Failed \n");
				return 1;
			}
			char recvBuff[1024];
			int bytesReceived = 0;
			char fnameClinte[100];
			memset(recvBuff, '0', sizeof(recvBuff));
			FILE *fp;
			char fnameClient[100];
			read(sockfd, fnameClient, 256);

			printf("File Name: %s\n", data.data);
			printf("Receiving file...");
			fp = fopen(data.data, "ab");
			if (NULL == fp)
			{
				printf("Error opening file");
				return 1;
			}
			long double sz = 1;
			/* Receive data in chunks of 256 bytes */
			while ((bytesReceived = read(sockfd, recvBuff, 1024)) > 0)
			{
				sz++;
				gotoxy(0, 4);
				printf("Received: %llf Mb", (sz / 1024));
				fflush(stdout);
				fwrite(recvBuff, 1, bytesReceived, fp);
			}

			if (bytesReceived < 0)
			{
				printf("\n Read Error \n");
			}
			printf("\nFile OK....Completed\n");
		}
	}
}

void *SendFileToClient(int *arg)
{
	int len = sizeof(clientAddr);
	int mySocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &len);
	struct sockaddr_in c_addr;
	int connfd = mySocket;
	printf("Connection accepted and id: %d\n", connfd);
	write(connfd, fname, 256);
	printf("\nFNAME:%s\n",fname);
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL)
	{
		printf("File opern error");
		return 1;
	}

	while (1)
	{
		unsigned char buff[1024] = {0};
		int nread = fread(buff, 1, 1024, fp);

		if (nread > 0)
		{
			write(connfd, buff, nread);
		}
		if (nread < 1024)
		{
			if (feof(fp))
			{
				printf("End of file\n");
				printf("File transfer completed for id: %d\n", connfd);
			}
			if (ferror(fp))
				printf("Error reading\n");
			break;
		}
	}
	printf("Closing Connection for id: %d\n", connfd);
	close(connfd);
	shutdown(connfd, SHUT_WR);
	sleep(2);
}

int main()
{

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	message msg;
	memset(&msg, '\0', sizeof(msg));
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
	char user[100];
	strcpy(user,msg.user);

	msg.messageType = 1;
	int rtn = send(clientSocket, (char *)&msg, sizeof(msg), 0);
	char userChat[100];

	int option;
	char flagExit;
	int exit = 1;
	while (exit == 1)
	{	
		bzero(&msg, sizeof(message));
		__fpurge(stdin);
		printf("\nOpções:\n");
		printf("1 - Enviar mensagem para um usuário (Para ver quem esta online escolha a opcao 2)\n");
		printf("2 - Exibir Lista de usuários online\n");
		printf("3 - Adicionar um amigo pelo username\n");
		printf("4 - Visualizar amigos online\n");
		printf("5 - Mandar mensagem para Grupo de usuários online\n");
		printf("6 - Mandar arquivo para usuários online\n");
		printf("7 - sair\n");
		printf("\nOpcao : ");
		scanf("%d", &option);

		if (option == 1)
		{
			__fpurge(stdin);
			printf("\nPara sair digite - \"exit\" \n");
			printf("\nDigite o user para envio: ");
			scanf("%[^\n]s", msg.destName);
			strcpy(msg.user,user);
			while (1)
			{
				__fpurge(stdin);
				scanf("%[^\n]s", msg.data);
				msg.msgLen = strlen(msg.data);
		
				if (strcmp(msg.data, "exit") == 0)
				{
					break;
				}
				msg.messageType = 2;
			
				send(clientSocket, (char *)&msg, sizeof(msg), 0);
				__fpurge(stdin);
			}
		}
		if (option == 2)
		{
			msg.messageType = 3;
			strcpy(msg.user,user);
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			__fpurge(stdin);
		}
		if (option == 7)
		{
			msg.messageType = 4;
			strcpy(msg.user,user);
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			__fpurge(stdin);
			close(clientSocket);
			exit = 0;
		}
		if (option == 3)
		{
			__fpurge(stdin);
			printf("\nDigite o user para adicionar: ");
			scanf("%[^\n]s", msg.data);
			msg.msgLen = strlen(msg.data);
			msg.messageType = 5;
			strcpy(msg.user,user);
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			__fpurge(stdin);
		}

		if (option == 4)
		{
			strcpy(msg.user, user);
			msg.messageType = 6;
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			__fpurge(stdin);
		}
		if (option == 5)
		{
			__fpurge(stdin);
			printf("\nPara sair do grupo digite - \"exit\" - ");
			while (1)
			{
				__fpurge(stdin);
				scanf("%[^\n]s", msg.data);
				msg.msgLen = strlen(msg.data);
				if (strcmp(msg.data, "exit") == 0)
				{
					break;
				}
				msg.messageType = 7;
				strcpy(msg.user, user);
				send(clientSocket, (char *)&msg, sizeof(msg), 0);
			}
		}
		if (option == 6)
		{
			__fpurge(stdin);
			printf("\nDigite o user para envio: ");
			scanf("%[^\n]s", msg.destName);
			__fpurge(stdin);
			printf("\nDigite o nome do arquivo: ");
			scanf("%[^\n]s", fname);

			msg.messageType = 8;
			msg.port = 8080;

			serverSocket = socket(PF_INET, SOCK_STREAM, 0);
			struct sockaddr_in serverAddr;

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(8080);

			serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
				return 0;

			if (listen(serverSocket, 1024) == -1)
				return 0;

			pthread_t threadId;
			int err = pthread_create(&threadId, NULL, &SendFileToClient,NULL );
			sleep(3);
			strcpy(msg.user,user);
			strcpy(msg.data,fname);
			send(clientSocket, (char *)&msg, sizeof(msg), 0);
			
			pthread_join(threadId, NULL);
		} 
		if(option<1 || option>7){
		if (strcmp(msg.data, "exit") != 0)
		{
			printf("\n[ATENCAO] digite uma opcao valida\n");
			sleep(2);
		}
		__fpurge(stdin);
		}

	}
}