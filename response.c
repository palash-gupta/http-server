#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "directory.c"

#define OK_TEMPLATE "HTTP/1.0 200 OK\r\nServer: http_server.c by me\r\n"
#define NOT_FOUND_TEMPLATE "HTTP/1.0 404 Not Found\r\n"

#define TEXT_HTML "Content-type: text/html; charset=utf-8\r\n"
#define TEXT_PLAIN "Content-type: text/plain\r\n"

#define C_ISDIR 0 // it is a directory
#define C_ISREG 1 // it is a file
#define C_ISSTH 2 // it is something else

long long get_file_size(const char* path) {
    struct stat st;
    
    // stat returns 0 on success
    if (stat(path, &st) == 0) {
        return st.st_size; // st_size contains the file size in bytes
    }
    
    return -1;
}

int check_path(const char *path) {
    struct stat path_stat;
    
    // Get file/directory metadata
    if (stat(path, &path_stat) != 0) {
        printf("Error: Path '%s' does not exist or cannot be accessed.\n", path);
        return -1;
    }
    // Check type using POSIX macros
    if (S_ISDIR(path_stat.st_mode)) {
        return C_ISDIR;
    } else if (S_ISREG(path_stat.st_mode)) {
        return C_ISREG;
    } else {
        return C_ISSTH;
    }
}

char *directory_response(const char *path) {
    char* content = html_response_directory(path);
    int len = strlen(content);

    char content_length[64];
    snprintf(content_length, sizeof content_length, "Content-Length: %d\r\n\r\n", len);

    int total_len = strlen(OK_TEMPLATE) + strlen(TEXT_HTML) + strlen(content_length) + len + 1; // +1 for \0

    char *response = malloc(sizeof(char) * total_len);
    snprintf(response, total_len, "%s%s%s%s", OK_TEMPLATE, TEXT_HTML, content_length, content);

    free(content);
    return response;
}

char *file_response(const char *path) {
    long long filesize = get_file_size(path);
    FILE* fp = fopen(path, "r");

    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", path);
        return NULL;
    }

    char *filecontent = malloc(sizeof(char) * filesize + 1);
    fseek(fp, 0L, SEEK_SET);
    size_t newLen = fread(filecontent, sizeof(char), filesize, fp);
    filecontent[newLen++] = '\0';

    fclose(fp);

    char content_length[64];
    snprintf(content_length, sizeof content_length, "Content-Length: %d\r\n\r\n", filesize);

    long long total_len = strlen(OK_TEMPLATE) + strlen(TEXT_PLAIN) + strlen(content_length) + filesize + 1; 

    char *response = malloc(sizeof(char) * total_len);
    snprintf(response, total_len, "%s%s%s%s", OK_TEMPLATE, TEXT_PLAIN, content_length, filecontent);

    free(filecontent);

    return response;
}

char *unsupported_response() {
    char *response = OK_TEMPLATE TEXT_PLAIN "Content-length: 26\r\n\r\n" "This is not supported yet.";

    char *heap_response = malloc(sizeof(char) * sizeof(response));
    strncpy(heap_response, response, sizeof(response));

    return heap_response;
}

char *not_found_error() {
    char *response = NOT_FOUND_TEMPLATE;

    char *heap_response = malloc(sizeof(char) * sizeof(response));
    strncpy(heap_response, response, sizeof(response));

    return heap_response;

}

char *generate_response(const char *path) {

    int rv = check_path(path);
    if (rv == C_ISDIR) {
        return directory_response(path);
    }
    else if (rv == C_ISREG) {
        return file_response(path);
    }
    else {
        return unsupported_response();
    }

}

// int main(void) {
//     char *a = generate_response(".");
//     printf("%s", a);
//     free(a);
//     return 0;
// }
