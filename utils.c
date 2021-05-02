//
// Created by xiaotian on 2021/4/30.
//
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/**
 * Do some log things
 * @param message
 */
void doLog(char* message) {
    int i = 0;
    FILE* stream = fopen("dns_svr.log", "a");
    if(stream  == NULL) {
        perror(" error in log file");
        exit(EXIT_FAILURE);
    }

    // get timestamp
    char time_str[sizeof ("2021-04-24T05:12:32+0000")];
    time_t now;
    time(&now);
    struct tm *tm_now = localtime(&now);
    strftime(time_str, sizeof time_str, "%FT%T%z", tm_now);

    fprintf(stream, "%s ", time_str);

    while(message[i]) {
        fputc(message[i], stream);
        i++;
    }

    fputc('\n', stream);
    fflush(stream);
    fclose(stream);
}


