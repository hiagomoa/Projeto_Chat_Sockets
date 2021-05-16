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


typedef struct Message
{
	//messageType tem na primeira posição o tipo, na sgunda é o contador de qtd de users(colocamos pois teve problemas)
    int messageType[2];//tipo da requisição ex: envio, get users...
    char user[1024];
    char data[1024]; 
    char destName[1024];
    int port;
    char userOnline[100][1024];
} message;

/*messageType[0] Para serv
1- cadastra no server
2- envia a message para alguem
3- desloga
messagetype[1] contador de qtd de usuarios
*/



/*messageType Para CLiente
1- imprime os usuarios online
*/
void * doRecieving(void * sockID){

	int clientSocket = *((int *) sockID);

	while(1){

		message data;

		int read = recv(clientSocket,(char *)&data, sizeof data, 0);
		printf("PPPPP %d - %d\n",data.messageType[0], data.messageType[1]);

		if(data.messageType[0] == 1){
			printf("\nUsuarios Online :\n");
			for(int i=0; i< 10 ; i++){
				printf("\n - %s",data.userOnline[i]);
			}
		}
	}

}

int main(){

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	message msg;	

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, doRecieving, (void *) &clientSocket );
	printf("Login do Servidor...\n");
	printf("Digite seu Username: ");
	scanf("%[^\n]s",msg.user);
	strcpy(msg.data,"QFOI?");
	msg.messageType[0] = 1;

	int rtn = send(clientSocket,(char*)&msg, sizeof(msg),0);
	printf("Retorn send %d",rtn);
	
	while(1){

		char input[1024];

		
		scanf("%s",input);


	}


}