#include <dirent.h>

#define TEMPLATE_TOP "<!DOCTYPE HTML>\r\n<html lang=\"en\"> \r\n<head> \r\n<meta charset=\"utf-8\"> \r\n<style type=\"text/css\"> \r\n:root { \r\ncolor-scheme: light dark; \r\n} \r\n</style> \r\n<title>Directory listing for /</title> \r\n</head> \r\n<body> \r\n<h1>Directory listing for /</h1> \r\n<hr> \r\n<ul>\r\n"

#define TEMPLATE_BOTTOM "</ul> \r\n<hr> \r\n</body> \r\n</html>\r\n"
#define LIST_TEMPLATE_TOP "<li><a href=\""
#define LIST_TEMPLATE_MIDDLE "\">"
#define LIST_TEMPLATE_BOTTOM "</a></li>\r\n"

#define MAX_BUF_SIZE 65536

int directory_listing(const char *path, char *dir_buf) {
    struct dirent *de;
    DIR *dr = opendir(path);

    if (dr == NULL) {
        fprintf(stderr, "Could not open %s.", path);
        return -1;
    }

    size_t i = 0;

    while ((de = readdir(dr)) != NULL) {
        char* name = de->d_name;

        size_t total_len = strlen(LIST_TEMPLATE_TOP) + strlen(LIST_TEMPLATE_MIDDLE) + strlen(LIST_TEMPLATE_BOTTOM) + 2*strlen(name) + strlen(path+1) + 1; // +1 for \0 and +1 for "/"
        char* html_list_item = malloc(sizeof(char) * total_len);

        snprintf(html_list_item, total_len, "%s%s%s%s%s%s", LIST_TEMPLATE_TOP, path+1, name, LIST_TEMPLATE_MIDDLE, name, LIST_TEMPLATE_BOTTOM);

        strncpy(dir_buf+i, html_list_item, total_len);
        i += total_len - 1;
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

    size_t len = strlen(dir_list);
    size_t total_len = len + strlen(TEMPLATE_TOP) + strlen(TEMPLATE_BOTTOM) + 1; // +1 for final null terminator

    char *html_response = malloc(sizeof(char) * total_len);

    snprintf(html_response, total_len, "%s%s%s", TEMPLATE_TOP, dir_list, TEMPLATE_BOTTOM);

    return html_response;
}
