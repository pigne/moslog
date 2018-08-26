//
//  main.c
//  moslog
//
//  Created by Yoann Pigné on 25/08/2018.
//  Copyright © 2018 Yoann Pigné. All rights reserved.
//


#include <stdio.h>
#include <os/log.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>

const char * CMD_NAME = "moslog";

void usage() {
    fprintf(stderr,"\
%s: a command line tool to log text messages on the OS X logging system (os_log).\n\
\n\
Usage: %s [-h] [-s <subsystem>] [-c <category>] [-l <level>] [-f <file>] [<message> ...]\n\
\t-h    Prints this help message\n\
\t-s <subsystem>    Subsystem string. Empty by default. Example: \"com.example.widget\"\n\
\t-c <category>    Category string. Empty by default. Example: \"communication\"\n\
\t-l <level>    Log level. Can be any of \"info\",  \"debug\", \"error\", or \"fault\".\n\
\t\tIf not specified, the log level is \"default\".\n\
\t\tNote that the Apple Console App only displays \"default\", \"error\", and \"fault\" levels.\n\
\t-f <file> read the specified file and use its content as a message to log.\n\
\n\
The rest of the command is interpreted as the message to be logged (if no file is given with -f).\n\
\n\
See log(1) and os_log(3) for  more information.\n",
            CMD_NAME,
            CMD_NAME
            );
}
int main(int argc, char * const argv[]) {
    
    int ch;
    FILE * fd;
    long fs;
    char * subsystem = NULL;
    char * category = NULL;
    os_log_type_t level = OS_LOG_TYPE_DEFAULT;
    
    char * message = NULL;
    
    while ((ch = getopt(argc, argv, "l:s:c:f:h")) != -1) {
        switch (ch) {
            case 'l':
                if(strcmp("info",optarg) == 0){
                    level = OS_LOG_TYPE_INFO;
                } else if(strcmp("debug",optarg) == 0) {
                    level = OS_LOG_TYPE_DEBUG;
                } else if(strcmp("error",optarg) == 0) {
                    level = OS_LOG_TYPE_ERROR;
                } else if(strcmp("fault",optarg) == 0) {
                    level = OS_LOG_TYPE_FAULT;
                } else usage();
                break;
            case 's':
                subsystem = optarg;
                break;
            case 'c':
                category = optarg;
                break;
            case 'f':
                if ((fd = fopen(optarg, "r")) == NULL) {
                    (void)fprintf(stderr,
                                  "%s: problem with the specified log file. \"%s\": %s\n", CMD_NAME, optarg, strerror(errno));
                    exit(1);
                }
                fseek(fd, 0L, SEEK_END);
                fs = ftell(fd);
                rewind(fd);
                message = malloc(fs + 1);
                fread(message, fs, 1, fd);
                fclose(fd);
                break;
            case 'h':
                usage();
                exit(0);
            case '?':
            default:
                usage();
                exit(1);
        }
    }
    
    argc -= optind;
    argv += optind;
    
    if( argc > 0 && message == NULL){
        fs = 0;
        for(int i = 0; i < argc; i++){
            fs += strlen(argv[i]) +1 ;
        }
        message = malloc(fs + 1);
        for(int i = 0; i < argc; i++){
            strcat(message, argv[i]);
            strcat(message," ");
        }
    } else  {
        //
    }
    if(message == NULL){
        usage();
        exit(0);
    }
    
    os_log_t log = OS_LOG_DEFAULT;
    if( subsystem != NULL && category != NULL) {
        log = os_log_create(subsystem, category);
    }
    
#ifdef DEBUG
    switch (level) {
        case OS_LOG_TYPE_DEBUG:
            printf("DEBUG");
            break;
        case OS_LOG_TYPE_DEFAULT:
            printf("DEFAULT");
            break;
        case OS_LOG_TYPE_FAULT:
            printf("FAULT");
            break;
        case OS_LOG_TYPE_ERROR:
            printf("ERROR");
            break;
        case OS_LOG_TYPE_INFO:
            printf("INFO");
            break;
        default:
            break;
    }
    printf(" level %s.\n", os_log_type_enabled(log, level) ? "enabled" : "disabled");
#endif

    os_log_with_type(log, level, "%s\n", message);
    
    free(message);
    return 0;
}
