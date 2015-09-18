/**
 * @file bug509.cpp regression case for bug 509 and 507 ( "Referring to a nonexisting server in servers=... doesn't even raise a warning"
 * and "rw-split router does not send last_insert_id() to master" )
 *
 * - "CREATE TABLE t2 (id INT(10) NOT NULL AUTO_INCREMENT, x int,  PRIMARY KEY (id));",
 * - do a number of INSERTs first using RWsplit, then directly Galera nodes.
 * - do "select @@wsrep_node_address, last_insert_id();" and "select last_insert_id(), @@wsrep_node_address;" and compares results.
 * - do "insert into t2 (x) values (i);" 1000 times and compares results of
 * "select @@wsrep_node_address, last_insert_id();" and "select last_insert_id(), @@wsrep_node_address;"
 *
 * Test fails if results are different (after 5 seconds of waiting after last INSERT)
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"

const char * sel1 = "select @@wsrep_node_address, last_insert_id();";
const char * sel2 = "select last_insert_id(), @@wsrep_node_address;";

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;
    int i;

    Test->read_env();
    Test->print_env();
    Test->galera->connect();
    Test->connect_maxscale();

    if (Test->galera->N < 3) {
        printf("There is not enoght nodes for test\n");
        Test->copy_all_logs();
        exit(1);
    }

    printf("Creating table\n");  fflush(stdout);
    global_result += execute_query(Test->conn_rwsplit, (char *) "DROP TABLE IF EXISTS t2; CREATE TABLE t2 (id INT(10) NOT NULL AUTO_INCREMENT, x int,  PRIMARY KEY (id));");
    printf("Doing INSERTs\n");  fflush(stdout);
    global_result += execute_query(Test->conn_rwsplit, (char *) "insert into t2 (x) values (1);");

    global_result += execute_query(Test->galera->nodes[0], (char *) "insert into t2 (x) values (2);");
    global_result += execute_query(Test->galera->nodes[0], (char *) "insert into t2 (x) values (3);");

    global_result += execute_query(Test->galera->nodes[1], (char *) "insert into t2 (x) values (4);");
    global_result += execute_query(Test->galera->nodes[1], (char *) "insert into t2 (x) values (5);");
    global_result += execute_query(Test->galera->nodes[1], (char *) "insert into t2 (x) values (6);");

    global_result += execute_query(Test->galera->nodes[2], (char *) "insert into t2 (x) values (7);");
    global_result += execute_query(Test->galera->nodes[2], (char *) "insert into t2 (x) values (8);");
    global_result += execute_query(Test->galera->nodes[2], (char *) "insert into t2 (x) values (9);");
    global_result += execute_query(Test->galera->nodes[2], (char *) "insert into t2 (x) values (10);");

    printf("Sleeping to let replication happen\n");  fflush(stdout);
    sleep(10);


    printf("Trying \n");  fflush(stdout);
    char last_insert_id1[1024];
    char last_insert_id2[1024];
    if ( (
             find_field(
                 Test->conn_rwsplit, sel1,
                 "last_insert_id()", &last_insert_id1[0])
             != 0 ) || (
             find_field(
                 Test->conn_rwsplit, sel2,
                 "last_insert_id()", &last_insert_id2[0])
             != 0 )) {
        printf("last_insert_id() fied not found!!\n");
        Test->copy_all_logs();
        exit(1);
    } else {
        printf("'%s' gave last_insert_id() %s\n", sel1, last_insert_id1);
        printf("'%s' gave last_insert_id() %s\n", sel2, last_insert_id2);
        if (strcmp(last_insert_id1, last_insert_id2) !=0 ) {
            global_result++;
            printf("last_insert_id() are different depending in which order terms are in SELECT\n");
        }
    }


    char id_str[1024];
    char str1[1024];

    for (int i = 100; i < 1100; i++) {
        sprintf(str1, "insert into t2 (x) values (%d);", i);
        global_result += execute_query(Test->conn_rwsplit, str1);
        sprintf(str1, "select * from t2 where x=%d;", i);
        find_field(
                    Test->conn_rwsplit, sel1,
                    "last_insert_id()", &last_insert_id1[0]);
        find_field(
                    Test->conn_rwsplit, str1,
                    "id", &id_str[0]);
        printf("last_insert_id is %s, id is %s\n", last_insert_id1, id_str);
        if (strcmp(last_insert_id1, id_str) !=0 ) {
            printf("replication is not happened yet, sleeping 5 seconds\n");
            sleep(5);
            find_field(
                        Test->conn_rwsplit, str1,
                        "id", &id_str[0]);
            printf("id after 5 seconds sleep is %s\n", id_str);
            if (strcmp(last_insert_id1, id_str) !=0 ) {
                global_result++;
                printf("last_insert_id is not equil to id even after waiting 5 seconds\n");}
        }
    }

    Test->close_maxscale_connections();
    Test->galera->close_connections();

    global_result += check_maxscale_alive();

    Test->copy_all_logs(); return(global_result);
}
