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
    int messageType[2];//tipo da requisição ex: envio, get users...
	//int countUserOnline;
    char user[100];
    char data[1024]; 
    char destName[100];
	//char originName[1024];
    users userOnline[100];
} message;


int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client{

	int index;
	int sockID;
	struct sockaddr_in clientAddr;
	int len;

};

struct client Client[1024];
pthread_t thread[1024];

void registerDb(char msg[], int clientSocket){
	char query[100];
	printf("\nValor do user eh: %s", msg);
	snprintf(query,99,"INSERT INTO mydb.usuario values(%d,\"%s\",%d)",clientSocket,msg,1);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");                                                                           
	}  
}

int searchSockUserDb(char* msg){
	char query[100];
	snprintf(query,99,"SELECT socket from mydb.usuario where username =\"%s\"", msg);
	printf("%s",query);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		//return EXIT_FAILURE;                                                                             
	}   
	MYSQL_RES *result ;
  	result = mysql_store_result(conn);     

 	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	row = mysql_fetch_row(result);
	 
	if(row[0] != NULL){
		return row[0];
	}
	return -1;
}

int searchAllOnlineUsers(message *msg){
	char query[100];
	snprintf(query,99,"SELECT username from mydb.usuario where status = 1");
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return 0;                                                                             
	}   
	MYSQL_RES *result ;
  	result = mysql_store_result(conn);     

 	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	int j = 0;
	while ((row = mysql_fetch_row(result)))
	{
		for(int i = 0; i < num_fields; i++)
		{
			if(row[i]!=NULL){
				strncpy(msg->userOnline[j].name, row[i], strlen(row[i]));
				j++;
				printf("---------%s------ ", msg->userOnline[j].name);
			} 
		}
		printf("\n");
	}

	for(int i =0; i< 10 ; i++){
		printf("DENTRO User: %s\n", msg->userOnline[i].name);
	}
	msg->messageType[1] = j;
	return 1;
}
void * doNetworking(void * ClientDetail){
	
	message msg;
	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail -> index;
	int clientSocket = clientDetail -> sockID;

	while(1){

		char data[1024];

		char output[1024];

		printf("\nkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");

		memset(&msg, '\0', sizeof(msg) );

		int rtn = recv(clientSocket,(message *)&msg, sizeof(msg),0);
		printf("\nRTN %d", rtn);
		//printf("\nznAISODAD %s",msg.data);
		
		if(msg.messageType[0] == 1){
			int existClientInDataBase = searchSockUserDb(msg.user);
			if(existClientInDataBase == -1){
			 	registerDb(msg.user,clientSocket);
			}
			message msgSend;
			msgSend.messageType[0] = 1;
			strncpy(msgSend.data,"TUDO CONECTADO",strlen("TUDO CONECTADO"));
			
			send(clientSocket,(void *)&msgSend,sizeof(msgSend),0);
		}
		if(msg.messageType[0] == 2){
			message msgSend;
			msgSend.messageType[0] = 2;

			//strcpy(msgSend.originName, msg.originName);
			strncpy(msgSend.user, msg.user,strlen(msg.user));
			strncpy(msgSend.destName, msg.destName,strlen(msg.destName));
			strncpy(msgSend.data, msg.data,strlen(msg.data));

			int sockerUserDestin = searchSockUserDb(msg.user);

			send(sockerUserDestin,(void *)&msgSend,sizeof(msgSend),0);
		}
		if(msg.messageType[0] == 3){

			message msgSend;
			int status = searchAllOnlineUsers(&msgSend);
			if(status){
				for(int i =0; i< msgSend.messageType[1] ; i++){
					printf("User: %s\n", msgSend.userOnline[i].name);
				}
			}
			msgSend.messageType[0] = 3;
			printf("\nOK %d - %d\n", msgSend.messageType[0], msgSend.messageType[1]);
		
			send(clientSocket,(void *)&msgSend,sizeof(msgSend),0);
		}
		printf("OOOOOOOOOOOOOOOOOOOOOO");

	}

	return NULL;
}

int connectionDb(){	
	if ((conn = mysql_init(NULL)) == NULL)
  {
    fprintf(stderr, "Could not init DB\n");
    return EXIT_FAILURE;
  }
  if (mysql_real_connect(conn, "172.17.0.1", "user", "user", "mydb", 3306, NULL, 0) == NULL)
  {
    fprintf(stderr, "DB Connection Error\n");
    return EXIT_FAILURE;
  }
  return 1;
}

int main(){
		
	connectionDb();

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);


	if(bind(serverSocket,(struct sockaddr *) &serverAddr , sizeof(serverAddr)) == -1) return 0;

	if(listen(serverSocket,1024) == -1) return 0;

	printf("Server started listenting on port 8080 ...........\n");

	while(1){

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *) &Client[clientCount]);

		clientCount ++;
 
	}
	  
	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);
	mysql_close(conn); 
}