#define PCRE2_CODE_UNIT_WIDTH 8

#include <dl_base.h>
#include <pcre2.h>

pcre2_code  *re;

/* PCRE2_SPTR is a pointer to unsigned code units of
   the appropriate width (in this case, 8 bits).
*/
PCRE2_SPTR name_table;

PCRE2_SPTR  pattern = "(*CRLF)\\d+:\\d+\\s+\\w+";     
PCRE2_SPTR  subject = "11 10:23  hello --  11:21  hey\r\n";     

int         errornumber, i,rc,utf8,crlf_is_newline;
PCRE2_SIZE  erroroffset;
PCRE2_SIZE  *ovector;

uint32_t option_bits;
uint32_t namecount;
uint32_t name_entry_size;
uint32_t newline;

size_t              subject_len;
pcre2_match_data    *match_data;

int pcre2_1(){
    
    re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* 0 indicates pattern is zero-terminated */
        0,                     /* default options */
        &errornumber,          /* for error number */
        &erroroffset,          /* for error offset */
        NULL);                 /* use default compile context */
        
    if (re == NULL){
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        return 1;
    }
    
    /* Using this function ensures that the block is exactly the right size for
       the number of capturing parentheses in the pattern.
    */
    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    
    subject_len = strlen(subject);
    
    rc = pcre2_match(
        re,                   /* the compiled pattern */
        subject,              /* the subject string */
        subject_len,          /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */
    
    if (rc < 0){
        
        switch(rc){
            case PCRE2_ERROR_NOMATCH:
                printf("No match\n");
                break;
            /* Handle other special cases if you like */
            default:
                printf("Matching error %d\n", rc);
                break;
        }
        
        pcre2_match_data_free(match_data);   /* Release memory used for the match */
        pcre2_code_free(re);                 /* data and the compiled pattern. */
        return 1;
        
    }
    
    /*  Match succeded. Get a pointer to the output vector,
        where string offsets are stored.
    */
    ovector = pcre2_get_ovector_pointer(match_data);
    printf("Match succeeded at offset %d\n", (int)ovector[0]);
    
    /* Show substrings stored in the output vector by number */
    
    dl_str  dlstr;
    PCRE2_SPTR substr_start;
    size_t substr_len;
    for (i = 0; i < rc; i++){
        substr_start = subject + ovector[2*i];
        substr_len = ovector[2*i+1] - ovector[2*i];
        
        dlstr.data = (char *)substr_start;
        dlstr.len = substr_len;
        
        dl_printf("%V\n", &dlstr);
    }
    puts("");
    
    
    /*  See if there are any named substrings, and if so, show them by name. First
        we have to extract the count of named parentheses from the pattern.
    */
    pcre2_pattern_info(
        re,                   /* the compiled pattern */
        PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
        &namecount);          /* where to put the answer */
    
    if (namecount == 0)
        printf("No named substrings\n");
    else{
        PCRE2_SPTR tabptr;
        printf("Named substrings\n");

        /* Before we can access the substrings, we must extract the table for
           translating names to numbers, and the size of each entry in the table.
        */
        PCRE2_SPTR  name_table;

        pcre2_pattern_info(
            re,                       /* the compiled pattern */
            PCRE2_INFO_NAMETABLE,     /* address of the table */
            &name_table);             /* where to put the answer */

        pcre2_pattern_info(
            re,                       /* the compiled pattern */
            PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
            &name_entry_size);        /* where to put the answer */

        /* Now we can scan the table and, for each entry, print the number, the name,
           and the substring itself. In the 8-bit library the number is held in two
           bytes, most significant first.
        */
        
        /* let pattern is "(?<hour>\\d+):(?<min>\\d+)\\s+\\w+";
           namecount == 2
           name_table == "?<hour>?<min>"
           name_entry_size == strlen("?<hour>") == strlen("hour") + 3;
           // the longest name plus 3
        */
        
        tabptr = name_table;
        for (i = 0; i < namecount; i++){
            int n = (tabptr[0] << 8) | tabptr[1];
            printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
            (int)(ovector[2*n+1] - ovector[2*n]), subject + ovector[2*n]);
            tabptr += name_entry_size;
        }
    }
    
    /* -g  */
    
    /* Before running the loop, check for UTF-8 and whether CRLF is a valid newline
       sequence. First, find the options with which the regex was compiled and extract
       the UTF state.
    */

    pcre2_pattern_info(re, PCRE2_INFO_ALLOPTIONS, &option_bits);
    utf8 = (option_bits & PCRE2_UTF) != 0;

    /* Now find the newline convention and see whether CRLF is a valid newline
        sequence.
    */

    pcre2_pattern_info(re, PCRE2_INFO_NEWLINE, &newline);
    crlf_is_newline = newline == PCRE2_NEWLINE_ANY ||
                  newline == PCRE2_NEWLINE_CRLF ||
                  newline == PCRE2_NEWLINE_ANYCRLF;
    
    /* Loop for second and subsequent matches */

    for(;;){
        uint32_t options = 0;                   /* Normally no options */
        PCRE2_SIZE start_offset = ovector[1];   /* Start at end of previous match */

        /* If the previous match was for an empty string, we are finished if we are
           at the end of the subject. Otherwise, arrange to run another match at the
           same point to see if a non-empty match can be found.
        */

        if(ovector[0] == ovector[1]){
            if (ovector[0] == subject_len) break;
        
            options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
        }

        /* Run the next matching operation */

        rc = pcre2_match(
            re,                   /* the compiled pattern */
            subject,              /* the subject string */
            subject_len,       /* the length of the subject */
            start_offset,         /* starting offset in the subject */
            options,              /* options */
            match_data,           /* block for storing the result */
            NULL);                /* use default match context */

          /* This time, a result of NOMATCH isn't an error. If the value in "options"
          is zero, it just means we have found all possible matches, so the loop ends.
          Otherwise, it means we have failed to find a non-empty-string match at a
          point where there was a previous empty-string match. In this case, we do what
          Perl does: advance the matching position by one character, and continue. We
          do this by setting the "end of previous match" offset, because that is picked
          up at the top of the loop as the point at which to start again.

          There are two complications: (a) When CRLF is a valid newline sequence, and
          the current position is just before it, advance by an extra byte. (b)
          Otherwise we must ensure that we skip an entire UTF character if we are in
          UTF mode. */

        if (rc == PCRE2_ERROR_NOMATCH){
            if (options == 0) break;                    /* All matches found */
    
            ovector[1] = start_offset + 1;              /* Advance one code unit */
    
            if (crlf_is_newline &&                      /* If CRLF is a newline & */
                start_offset < subject_len - 1 &&    /* we are at CRLF, */
                subject[start_offset] == '\r' &&
                subject[start_offset + 1] == '\n'){
                    
                ovector[1] += 1;                          /* Advance by one more. */
            
            }else if (utf8)                               /* Otherwise, ensure we */
            {                                             /* advance a whole UTF-8 */
                while (ovector[1] < subject_len)       /* character. */
                {
                    if ((subject[ovector[1]] & 0xc0) != 0x80) break;
                    
                    ovector[1] += 1;
                }
            }
            continue;    /* Go round the loop again */
        }

        /* Other matching errors are not recoverable. */

        if (rc < 0){
            printf("Matching error %d\n", rc);
            pcre2_match_data_free(match_data);
            pcre2_code_free(re);
            return 1;
        }

        /* Match succeded */

        printf("\nMatch succeeded again at offset %d\n", (int)ovector[0]);

        /* The match succeeded, but the output vector wasn't big enough. This
           should not happen.*/

        if (rc == 0)
            printf("ovector was not big enough for all the captured substrings\n");

        /* As before, show substrings stored in the output vector by number, and then
           also any named substrings. */

        for (i = 0; i < rc; i++){
            PCRE2_SPTR substring_start = subject + ovector[2*i];
            size_t substring_length = ovector[2*i+1] - ovector[2*i];
            printf("%d: [%.*s]\n", i, (int)substring_length, (char *)substring_start);
        }

        if (namecount == 0)
            printf("No named substrings\n");
        else{
            PCRE2_SPTR tabptr = name_table;
            printf("Named substrings\n");
            for (i = 0; i < namecount; i++){
                int n = (tabptr[0] << 8) | tabptr[1];
                printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
                (int)(ovector[2*n+1] - ovector[2*n]), subject + ovector[2*n]);
                tabptr += name_entry_size;
            }
        }
    }      /* End of loop to find second and subsequent matches */
  
  
    
}

int main(){
    pcre2_1();
}