#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <iostream>
#include <vector>

char *compute_get_request(char *host, char *url, string query_params,
                            vector<string> cookies, int cookies_count, string authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (!query_params.empty()) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params.c_str());
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (!authorization.empty()) {
        sprintf(line, "Authorization: ");
        strcat(line, authorization.c_str());

        compute_message(message, line);
    }

    if (cookies.size() != 0) {
        sprintf(line, "Cookie:");

        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i].c_str());
            strcat(line, "; ");
        }

        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char *compute_delete_request(char *host, char *url, string query_params,
                            vector<string> cookies, int cookies_count, string authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (!query_params.empty()) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params.c_str());
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (!authorization.empty()) {
        sprintf(line, "Authorization: ");
        strcat(line, authorization.c_str());

        compute_message(message, line);
    }

    if (cookies.size() != 0) {
        sprintf(line, "Cookie:");

        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i].c_str());
            strcat(line, "; ");
        }

        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}


char *compute_post_request(char *host, char *url, char *content_type, string body_data,
                           int body_data_fields_count, vector<string> cookies,
                           int cookies_count, string authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", body_data.length());
    compute_message(message, line);

    if (!authorization.empty()) {
        sprintf(line, "Authorization: ");
        strcat(line, authorization.c_str());

        compute_message(message, line);
    }

    if (cookies_count != 0) {
        sprintf(line, "Cookie:");

        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i].c_str());
            strcat(line, "; ");
        }

        line[strlen(line) - 1] = '\0';
        compute_message(message, line);
    }

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data.c_str());
    compute_message(message, line);

    free(line);
    return message;
}