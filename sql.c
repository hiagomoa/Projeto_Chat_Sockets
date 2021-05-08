#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

MYSQL *conn;

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
int main(void)
{
  //MYSQL *conn;

//   MYSQL conexao;
//   mysql_init(&conexao);
// if ( mysql_real_connect(&conexao, "172.17.0.1", "user", "user", "mydb",3306, NULL, 0) ){
// 	printf("\nConexao ao banco realizada com sucesso!\n");
// 	}
// 	else{
// 		printf("Falha de conexao\n");
// 		printf("Erro %d : %s\n", mysql_errno(&conexao), mysql_error(&conexao));
// 	}

/*
  if ((conn = mysql_init(NULL)) == NULL)
  {
    fprintf(stderr, "Could not init DB\n");
    return EXIT_FAILURE;
  }
  if (mysql_real_connect(conn, "172.17.0.1", "user", "user", "mydb", 3306, NULL, 0) == NULL)
  {
    fprintf(stderr, "DB Connection Error\n");
    return EXIT_FAILURE;
  }*/

  connectionDb();
  // if (mysql_query(conn, "CREATE TABLE Persons (PersonID int, LastName varchar(255),FirstName varchar(255), Address varchar(255), City varchar(255))") != 0)
  // {
  //   fprintf(stderr, "Query Failure\n");
  //   return EXIT_FAILURE;                                                                             
  // }    
  char query[50];
  snprintf(query,49,"INSERT INTO mydb.usuario values(%d,\"%s\",%d)",2,"ALOU", 1);
  //printf("%s",query);
  if (mysql_query(conn, "SELECT username from mydb.usuario where username = \"kk1\"") != 0)
  {
    fprintf(stderr, "Query Failure\n");
    return EXIT_FAILURE;                                                                             
  }
  MYSQL_RES *result ;
  result = mysql_store_result(conn);     


 int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  MYSQL_ROW row2;
  row = mysql_fetch_row(result);
  if(row==NULL){
    printf("\nKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK");
   return "NULL" 
  }
  // printf("FOI?--- %s",row[0]);
  /*
  while ((row2 = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          printf("%s ", row2[i] ? row2[i] : "NULL");
      }

      printf("\n");
  }*/


  mysql_close(conn);                                                                                 
  return EXIT_SUCCESS;
}
