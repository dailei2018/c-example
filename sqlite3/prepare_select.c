#include "base.h"

int main(int argc, char **argv){
    sqlite3 *db;
    const char *sql;
    int rc;
    
    person_t p, p1;

    sqlite3_stmt *stmt = NULL;

    rc = sqlite3_open(DBNAME, &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
    sql = "select * from person where age = ?";
    sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);

    sqlite3_bind_int(stmt, 1, 25);

    // just for testing line 41 'Segmentation fault'
    if(sqlite3_step(stmt) == SQLITE_ROW){
        p1.id = sqlite3_column_int(stmt, 0);
        p1.name = (char *)sqlite3_column_text(stmt, 1);  //every time call it, will free the previous memory,if you want to reuse, use 'strdup'
        p1.age = sqlite3_column_int(stmt, 2);
        p1.amount = sqlite3_column_int(stmt, 3);
        printf("id:%d name:%s age:%d amount:%d\n", p1.id, p1.name, p1.age, p1.amount);
    }

    //execute sql
    while(sqlite3_step(stmt) == SQLITE_ROW){
        p.id = sqlite3_column_int(stmt, 0);
        p.name = (char *)sqlite3_column_text(stmt, 1);
        p.age = sqlite3_column_int(stmt, 2);
        p.amount = sqlite3_column_int(stmt, 3);
        printf("id:%d name:%s age:%d amount:%d\n", p.id, p.name, p.age, p.amount);
    }
    
    //Segmentation fault
    //printf("%s\n", p1.name);

    sqlite3_finalize(stmt);
    
    //close and free memory
    sqlite3_close(db);
    return 1;
}
