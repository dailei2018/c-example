#include "base.h"

int main(int argc, char **argv){
    sqlite3 *db;
    const char *sql;
    int rc;
    
    sqlite3_stmt *stmt = NULL;

    rc = sqlite3_open(DBNAME, &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
    sql = "insert into person (name,age,amount) values (?,25,?);";
    sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);

    // 2 means the second field, because we omit the 'id' which is the first field
    sqlite3_bind_text(stmt, 2, "Json", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, 33);

    //execute sql
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        return rc;
    }else{
        puts("insert into person (name,age,amount) values ('Json',25,33)");
    }

    //reset for next bind
    sqlite3_reset(stmt);

    sqlite3_bind_text(stmt, 2, "Leo", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, 2000);

    //execute sql
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        return rc;
    }else{
        puts("insert into person (name,age,amount) values ('Leo',25,2000)");
    }

    sqlite3_finalize(stmt);

    sqlite3_close(db);
    return 1;
}
