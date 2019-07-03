#include "/usr/include/mysql/mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


void m(){
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;


char *server = "35.164.140.47";
char *user = "seevider";
char *password = "seevider123";
char *database = "seevider";


if( !(conn = mysql_init((MYSQL*)NULL))){
printf("init fail\n");
exit(1);
}

printf("mysql_init success.\n");

if(!(mysql_real_connect(conn,server,user,password,database, 3306, NULL, 0))){
printf("connect error.\n");
exit(1);
}

printf("mysql_real_connect suc.\n");

if(mysql_select_db(conn, database) !=0){
mysql_close(conn);
printf("select_db fail.\n");
exit(1);
}
printf("select mydb syc.\n");

//if(mysql_query(conn,"select * from parking_spot")){
if(mysql_query(conn,"SELECT s.parking_spot_id, s.policy_id, p.time_limit FROM parking_spot AS s LEFT JOIN parking_policy AS p ON s.policy_id = p.policy_id")){
printf("query fail\n");
exit(1);
}

printf("query success\n");

res = mysql_store_result(conn);
printf("res success\n");

while((row = mysql_fetch_row(res))!=NULL){
printf("%s\n", row[2]);
}

mysql_close(conn);
}
