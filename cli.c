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
    int messageType[2];//tipo da requisição ex: envio, get users...
	//int countUserOnline;
    char user[100];
    char data[1024]; 
    char destName[100];
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
void * doRecieving(void * sockID){

	int clientSocket = *((int *) sockID);

	while(1){

		message data;
		memset(&data, '0', sizeof(data));
		int read = recv(clientSocket,(char *)&data, sizeof data, 0);
		printf("PPPPP %d - %d\n",data.messageType[0], data.messageType[1] );

		if(data.messageType[0] == 1){
			printf("\nUsuarios Online :\n");
			printf("- %s\n",data.data);
		}
		
		if(data.messageType[0] == 2){
			printf("[%s] - %s\n",data.user,data.data);
		}

		if(data.messageType[0] == 3){
			printf("\nUsuarios Online :\n");
			for(int i=0; i< data.messageType[1] ; i++){
				printf("- %s\n",data.userOnline[i].name);
			}
		}
	}
}

int main(){

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	message msg;	
	memset(&msg, '\0', sizeof(msg) );
	printf("\n%d", sizeof(msg));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, doRecieving, (void *) &clientSocket );
	printf("Login do Servidor...\n");
	printf("Digite seu Username: ");
	scanf("%[^\n]s",msg.user);
	
	msg.messageType[0] = 1;
	printf("SEGUNDO%d\n", sizeof(msg));
	int rtn = send(clientSocket,(char*)&msg, sizeof(msg),0);
	printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");
	char userChat[100];
	
	int option;
	char flagExit;

	while(1){
		printf("\nOpções:\n");
		printf("1 - Enviar mensagem para um usuário online(Para ver quem esta online escolha a opcao 2)\n");
		printf("2 - Exibir Lista de amigos\n");
		printf("3 - Exibir Lista de usuários online\n");
		printf("4 - sair");
		printf("\nOpcao : ");
		scanf("%d",&option);
		if(option == 1){
			printf("\nPara sair digite - \"exit\" - ");
			printf("\nDigite o user para envio: ");
			scanf("%[^\n]s",msg.destName);
			//strcpy(msg.originName,msg.user);
			while(1){
				scanf("%[^\n]s",msg.data);
				if(strcmp(msg.data,"exit")) break;
				msg.messageType[0] = 2;
				send(clientSocket,(char*)&msg, sizeof(msg),0);
			}
		}
		if(option == 3){
			msg.messageType[0] = 3;
			send(clientSocket,(char*)&msg, sizeof(msg),0);
		}
		
		//scanf("%[^\n]s",userChat);

		char input[1024];

		
	//	scanf("%s",input);


	}


}