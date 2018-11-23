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

    int sum,i,t;
    char *name;
    //execute sql
    //assume we do not know the number of columns and the column type
    while(sqlite3_step(stmt) == SQLITE_ROW){
        sum = sqlite3_column_count(stmt);

        for(i = 0; i < sum; i++){
            t = sqlite3_column_type(stmt, i);
            name = (char *)sqlite3_column_name(stmt, i);
            printf("%s:", name);
            if(t == SQLITE_INTEGER){
                printf("%d ", sqlite3_column_int(stmt, i));
            }else if(t == SQLITE_TEXT){
                printf("%s ", (char *)sqlite3_column_text(stmt, i));
            }else{
                printf("null ");
            }
            puts("");
        }
        puts("");
        //printf("id:%d name:%s age:%d amount:%d\n", p.id, p.name, p.age, p.amount);
    }
    
    //Segmentation fault
    //printf("%s\n", p1.name);

    sqlite3_finalize(stmt);
    
    //close and free memory
    sqlite3_close(db);
    return 1;
}
