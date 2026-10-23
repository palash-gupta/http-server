#include <sys/stat.h>

#include "directory.c"
#include "filetype.c"

#define OK_TEMPLATE "HTTP/1.0 200 OK\r\nServer: http_server.c by me\r\n"
#define NOT_FOUND_TEMPLATE "HTTP/1.0 404 Not Found\r\n"

#define TEXT_HTML "Content-type: text/html; charset=utf-8\r\n"
#define TEXT_PLAIN "Content-type: text/plain\r\n"

#define IMAGE_JPEG "Content-type: image/jpeg\r\n"
#define IMAGE_PNG "Content-type: image/png\r\n"
#define IMAGE_GIF "Content-type: image/gif\r\n"
#define IMAGE_WEBP "Content-type: image/webp\r\n"

#define AUDIO_MPEG "Content-type: audio/mpeg\r\n"
#define AUDIO_WAV "Content-type: audio/wav\r\n"

#define VIDEO_MP4 "Content-type: video/mp4\r\n"
#define VIDEO_WEBM "Content-type: video/webm\r\n"

#define C_ISDIR 0 // it is a directory
#define C_ISREG 1 // it is a file
#define C_ISSTH 2 // it is something else

#define MAX_CONT_LEN 64

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

char *directory_response(const char *path, size_t *response_size) {
    char* content = html_response_directory(path);
    size_t len = strlen(content);

    char content_length[MAX_CONT_LEN];
    snprintf(content_length, sizeof content_length, "Content-Length: %d\r\n\r\n", len);

    size_t total_len = strlen(OK_TEMPLATE) + strlen(TEXT_HTML) + strlen(content_length) + len + 1; // +1 for \0
    *response_size = total_len;

    char *response = malloc(sizeof(char) * total_len);
    snprintf(response, total_len, "%s%s%s%s", OK_TEMPLATE, TEXT_HTML, content_length, content);

    free(content);

    return response;
}

char *file_response(const char *path, size_t *response_size) {
    size_t filesize = get_file_size(path);

    uint8_t ftype = filetype(path);

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

    char content_length[MAX_CONT_LEN];
    snprintf(content_length, sizeof content_length, "Content-Length: %d\r\n\r\n", filesize);

    char *content_type;
    switch (ftype) {
        case FTYPE_PLAIN:  content_type = TEXT_PLAIN; break;
        case FTYPE_JPEG:   content_type = IMAGE_JPEG; break;
        case FTYPE_PNG:    content_type = IMAGE_PNG;  break;
        case FTYPE_GIF:    content_type = IMAGE_GIF;  break;
        case FTYPE_WEBP:   content_type = IMAGE_WEBP; break;
        case FTYPE_MPEG:   content_type = AUDIO_MPEG; break;
        case FTYPE_WAV:    content_type = AUDIO_WAV;  break;
        case FTYPE_MP4:    content_type = VIDEO_MP4;  break;
        case FTYPE_WEBM:   content_type = VIDEO_WEBM; break;
        default:           content_type = TEXT_PLAIN;
    }

    size_t total_len = strlen(OK_TEMPLATE) + strlen(content_type) + strlen(content_length) + filesize; 
    *response_size = total_len;

    char *response = malloc(sizeof(char) * total_len);
    size_t i = 0;

    memcpy(response + i, OK_TEMPLATE, strlen(OK_TEMPLATE));
    i += strlen(OK_TEMPLATE);

    memcpy(response + i, content_type, strlen(content_type));
    i += strlen(content_type);

    memcpy(response + i, content_length, strlen(content_length));
    i += strlen(content_length);

    memcpy(response + i, filecontent, filesize);

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

    char *heap_response = malloc(sizeof(char) * strlen(response));
    strncpy(heap_response, response, strlen(response));

    return heap_response;

}

char *generate_response(const char *path, size_t *response_size) {

    int rv = check_path(path);
    if (rv == C_ISDIR) {
        return directory_response(path, response_size);
    }
    else if (rv == C_ISREG) {
        return file_response(path, response_size);
    }
    else {
        *response_size = strlen(NOT_FOUND_TEMPLATE);
        return not_found_error();
    }

}
