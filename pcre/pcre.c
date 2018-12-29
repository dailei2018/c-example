#include <dl_base.h>
#include <pcre.h>

/*
https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html
https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html
*/


#define OVECMAX 30

char    *err_msg;
int     err;

int main(){
    pcre    *re;
    int     offsets[OVECMAX];
    
    re
    
    return 0;
}

void pcre_1(){
    pcre    *re;
    int      offsets[OVECMAX], s;
    
    char    *str = "10:23  hello";
    char    *pattern = "^\\d+:\\d+\\s+\\w+$";
    
    re = pcre_compile(pattern, 0, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        error_exit(s);
    }
    
}

void error_exit(int ret){
    switch(ret) {
        case PCRE_ERROR_NOMATCH      :
            printf("String did not match the pattern\n");        break;
            
        case PCRE_ERROR_NULL         :
            printf("Something was null\n");                      break;
            
        case PCRE_ERROR_BADOPTION    :
            printf("A bad option was passed\n");                 break;
            
        case PCRE_ERROR_BADMAGIC     :
            printf("Magic number bad (compiled re corrupt?)\n"); break;
            
        case PCRE_ERROR_UNKNOWN_NODE :
            printf("Something kooky in the compiled re\n");      break;
            
        case PCRE_ERROR_NOMEMORY     :
            printf("Ran out of memory\n");                       break;
            
        default                      :
            printf("Unknown error\n");                           break;
      }
      
      exit(1);
}