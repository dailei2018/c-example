#include <dl_base.h>
#include <pcre.h>


#define OVECMAX 30

const char    *err_msg;
int     err;

void pcre_1();
void p_error(int ret);

/* first one match */
void pcre_1(){
    pcre    *re;
    int      offsets[OVECMAX], s;
    
    char    *str = "11 10:23  hello --  11:21  hey";
    char    *pattern = "\\d+:\\d+\\s+\\w+";
    
    puts("---------------------match one-----------------------");
    
    re = pcre_compile(pattern, 0, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        p_error(s);
        exit(1);
    }
    
    dl_printf("%d matchs\n", s);
    
    dl_str tmp_s;
    
    tmp_s.data = str + offsets[0];
    tmp_s.len = offsets[1] - offsets[0];
    dl_printf("%V\n", &tmp_s);

    puts("---------------------match one-----------------------\n\n");
}

/* match all */
void pcre_2(){
    pcre    *re;
    int      offsets[OVECMAX], s, i;
    
    char    *str = "11 10:23  hello --  11:21  hey aa";
    char    *pattern = "\\d+:\\d+\\s+\\w+";
    
    puts("---------------------match all-----------------------");
    
    re = pcre_compile(pattern, 0, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        p_error(s);
        exit(1);
    }
    
    dl_str tmp_s;
    
    for(;;){
        tmp_s.data = str + offsets[0];
        tmp_s.len = offsets[1] - offsets[0];
        
        dl_printf("%V\n", &tmp_s);
        
        s = pcre_exec(re, NULL, str, strlen(str), offsets[1], 0, offsets, OVECMAX);
        
        if(s < 0){
            p_error(s);
            break;
        }
    }
    
    puts("---------------------match all-----------------------\n\n");
    
}

/* group */
void pcre_3(){
    pcre    *re;
    int      offsets[OVECMAX], s, i, j;
    
    char    *str = "11 10:23  hello --  11:21  hey aa";
    char    *pattern = "(\\d+):(\\d+)\\s+(\\w+)";
    
    char hour[3], min[3], desc[16];
    
    re = pcre_compile(pattern, 0, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    dl_str tmp_s;
    
    puts("---------------------match group-----------------------");
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    while(s > 0){
        
        for(i = 0, j = 0; i < s; i++){
            tmp_s.data = str + offsets[j];
            tmp_s.len = offsets[j+1] - offsets[j];
            dl_printf("%V\n", &tmp_s);
            
            j += 2;
        }
        
        puts("");
        s = pcre_exec(re, NULL, str, strlen(str), offsets[1], 0, offsets, OVECMAX);
        
    }
    
    puts("---------------------match group-----------------------\n\n");
    
}

/* caseless, multiline */
void pcre_4(){
    pcre    *re;
    int      offsets[OVECMAX], s, i;
    
    char    *str = "Dog\n"
                   "Cat Dog\n"
                   "Sheep Cat";
    
    char    *pattern = "^dog$";
    
    puts("---------------------caseless multiline-----------------------");
    
    re = pcre_compile(pattern, PCRE_CASELESS|PCRE_MULTILINE, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        p_error(s);
        exit(1);
    }
    
    dl_str tmp_s;
    
    for(;;){
        tmp_s.data = str + offsets[0];
        tmp_s.len = offsets[1] - offsets[0];
        
        dl_printf("%V\n", &tmp_s);
        
        s = pcre_exec(re, NULL, str, strlen(str), offsets[1], 0, offsets, OVECMAX);
        
        if(s < 0){
            p_error(s);
            break;
        }
    }
    
    puts("---------------------caseless multiline-----------------------\n\n");
    
}

/* first one match, not greedy */
void pcre_5(){
    pcre    *re;
    int      offsets[OVECMAX], s;
    
    char    *str = "11  22  aa  22";
    char    *pattern = "\\d+.*22";
    
    puts("---------------------not greedy-----------------------");
    
    re = pcre_compile(pattern, PCRE_UNGREEDY, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        p_error(s);
        exit(1);
    }
    
    dl_printf("%d matchs\n", s);
    
    dl_str tmp_s;
    
    tmp_s.data = str + offsets[0];
    tmp_s.len = offsets[1] - offsets[0];
    dl_printf("%V\n", &tmp_s);

    puts("---------------------not greedy-----------------------\n\n");
}

/* first one match, utf-8 */
void pcre_6(){
    pcre    *re;
    int      offsets[OVECMAX], s;
    
    char    *str = "begin 中国国国   你好 end";
    char    *pattern = "中国+\\s+你好";
    
    puts("---------------------uft-8-----------------------");
    
    re = pcre_compile(pattern, PCRE_UTF8, &err_msg, &err, NULL);
    if(re == NULL){
        exit_msg(1, "compile failed: %s\n", err_msg);
    }
    
    s = pcre_exec(re, NULL, str, strlen(str), 0, 0, offsets, OVECMAX);
    if(s < 0){
        p_error(s);
        exit(1);
    }
    
    dl_printf("%d matchs\n", s);
    
    dl_str tmp_s;
    
    tmp_s.data = str + offsets[0];
    tmp_s.len = offsets[1] - offsets[0];
    dl_printf("%V\n", &tmp_s);

    puts("---------------------uft-8-----------------------\n\n");
}

void p_error(int ret){
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
      
}

int main(){
    
    pcre_1();
    pcre_2();
    pcre_3();
    pcre_4();
    pcre_5();
    pcre_6();
    
    return 0;
}