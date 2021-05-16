#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char const *argv[])
{
    /* code */

    char query[100];
    char msg[20] = "alouaaaaaa\n";
    char msg2[20];
    fgets(msg2, 19, stdin);
   //printf("%s-",msg2);
	
    snprintf(query,strlen("alguma\n")-1,"%s","alguma\n");
	//snprintf(query,99,"SELECT socket from mydb.usuario where username =\"%s\"", msg);
    printf("\n%s-",query);
    return 0;
}
