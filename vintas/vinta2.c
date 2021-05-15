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

typedef struct Message
{
    int messageType[2];//tipo da requisição ex: envio, get users...
    char user[1024];
    char data[1024]; 
    char destName[1024];
    int port;
    char userOnline[100][1024];
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

int searchUserDb(char* msg){
	char query[100];
	snprintf(query,99,"SELECT username from mydb.usuario where username =\"%s\"", msg);
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
	 

	if(row != NULL){
		return 1;
	}
	return 0;
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
				strcpy(msg->userOnline[j],row[i]);
				j++;
				printf("---------%s------ ", msg->userOnline[j]);
			} 
		}
		printf("\n");
	}

	for(int i =0; i< 10 ; i++){
		printf("DENTRO User: %s\n", msg->userOnline[i]);
	}
	msg->messageType[1] = j;
	return 1;
}
void * doNetworking(void * ClientDetail){
	
	message msg;
	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail -> index;
	int clientSocket = clientDetail -> sockID;

	// printf("Client %d connected.\n",index + 1);
	// printf("\nkkkkkkkkk");
	// //int rtn = recv(clientSocket,(message *)&msg,sizeof(msg),0);
	// printf("\nRetorn send %d",rtn);
	// printf("\nValor de resposta %s, %d--", msg.user, msg.messageType);
	// printf("\nLKKKKKKKK");
	
	while(1){


		char data[1024];
		//int read = recv(clientSocket,data,1024,0);
		// data[read] = '\0';

		char output[1024];

		int rtn = recv(clientSocket,(message *)&msg,sizeof(msg),0);
		printf("\nznAISODAD %d",msg.messageType[0]);
		printf("\nznAISODAD--------- %s",msg.data);
		if(msg.messageType[0] == 1){
			int existClientInDataBase = searchUserDb(msg.user);
			if(existClientInDataBase == 0){
			 	registerDb(msg.user,clientSocket);
			}
			message msgSend;
			int status = searchAllOnlineUsers(&msgSend);
			if(status){
				for(int i =0; i< msgSend.messageType[1] ; i++){
					printf("User: %s\n", msgSend.userOnline[i]);
				}
			}
			msgSend.messageType[0] = 1;
			msgSend.messageType[1]= 4;
			printf("\nOK %d - %d\n", msgSend.messageType[0], msgSend.messageType[1]);
		
			send(clientSocket,(void *)&msgSend,sizeof(msgSend),0);
			
			
			//retornar todos ja cadastrados.
		}
	
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
	// if (mysql_query(conn, "CREATE TABLE Persons (PersonID int, LastName varchar(255),FirstName varchar(255), Address varchar(255), City varchar(255))") != 0)
	// {
	// 	fprintf(stderr, "Query Failure\n");
	// 	return EXIT_FAILURE;                                                                             
	// }    

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
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