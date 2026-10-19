#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEMPLATE_TOP "<!DOCTYPE HTML>\r\n<html lang=\"en\"> \r\n<head> \r\n<meta charset=\"utf-8\"> \r\n<style type=\"text/css\"> \r\n:root { \r\ncolor-scheme: light dark; \r\n} \r\n</style> \r\n<title>Directory listing for /</title> \r\n</head> \r\n<body> \r\n<h1>Directory listing for /</h1> \r\n<hr> \r\n<ul>\r\n"

#define TEMPLATE_BOTTOM "</ul> \r\n<hr> \r\n</body> \r\n</html>\r\n"
#define LIST_TEMPLATE_TOP "<li><a href=\""
#define LIST_TEMPLATE_MIDDLE "\">"
#define LIST_TEMPLATE_BOTTOM "</a></li>\r"

#define MAX_BUF_SIZE 4096

int directory_listing(const char *path, char *dir_buf) {
    struct dirent *de;
    DIR *dr = opendir(path);

    if (dr == NULL) {
        fprintf(stderr, "Could not open %s.", path);
        return -1;
    }

    int i = 0;

    while ((de = readdir(dr)) != NULL) {
        char* name = de->d_name;

        strncpy(dir_buf+i, LIST_TEMPLATE_TOP, sizeof(LIST_TEMPLATE_TOP));
        i += sizeof(LIST_TEMPLATE_TOP);
        i--;

        for (int j = 0; j < strlen(name); j++) {
            dir_buf[i] = name[j];
            i++;
        }

        strncpy(dir_buf+i, LIST_TEMPLATE_MIDDLE, sizeof(LIST_TEMPLATE_MIDDLE));
        i += sizeof(LIST_TEMPLATE_MIDDLE);
        i--;

        for (int j = 0; j < strlen(name); j++) {
            dir_buf[i] = name[j];
            i++;
        }

        strncpy(dir_buf+i, LIST_TEMPLATE_BOTTOM, sizeof(LIST_TEMPLATE_BOTTOM));
        i += sizeof(LIST_TEMPLATE_BOTTOM);
        dir_buf[i-1] = '\n';
    }

    dir_buf[i] = '\0';
    //printf("%s", dir_buf);

    closedir(dr);
    return 0;
}

char *html_response_directory(const char* path) {
    char dir_list[MAX_BUF_SIZE];
    if (directory_listing(path, dir_list) != 0)
        return NULL;

    int len = strlen(dir_list);
    int total_len = len + strlen(TEMPLATE_TOP) + strlen(TEMPLATE_BOTTOM) + 1; // +1 for final null terminator

    char *html_response = malloc(sizeof(char) * total_len);

    snprintf(html_response, total_len, "%s%s%s", TEMPLATE_TOP, dir_list, TEMPLATE_BOTTOM);

    return html_response;
}

// int main() {
// 
//     char *a = html_response_directory(".");
//     printf("%s", a);
//     free(a);
//     return 0;
// }
