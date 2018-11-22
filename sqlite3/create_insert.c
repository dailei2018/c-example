#include "base.h"

int main(int argc, char **argv){
    sqlite3 *db;
    char *errmsg;
    const char *sql;
    int rc;

    rc = sqlite3_open(DBNAME, &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    sql = "drop table if exists person;"
          "create table person("
          "   id integer primary key autoincrement,"
          "   name varchar(255),"
          "   age int not null default 0,"
          "   amount int not null default 0"
          ");";
    rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return rc;
    }
    puts("create table person successfully");
    
    sql = "insert into person (name,age,amount) values ('lyli',22, 1000);";
    rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return rc;
    }
    puts(sql);

    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 1;
}
