#ifndef __base_h__
#define __base_h__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#define DBNAME "tmp.db"

static int callback(void *NotUsed, int argc, char **argv, char **azColName);

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

typedef struct person person_t;

struct person {
    int id;
    int age;
    int amount;
    char *name;
};

#endif
