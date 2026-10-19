#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LEN 256

char *get_path_from_request(char *request) {
    if (!strncmp(request, "GET", 3)) {
        char *s_loc = strstr(request, " HTTP");
        if (s_loc == NULL) {
            fprintf(stderr, "Path not found, returning 404.\n");
            return NULL;
        }
        
        int path_size = s_loc - request - 3;
        char *path = malloc(sizeof(char) * path_size + 1); 

        path[0] = '.'; // local path

        strncpy(path+1, request+4, path_size);

        path[path_size] = '\0';

        return path;

    } else {
        return NULL;
    }
}

// int main(void) {
//     char *request = "GET / HTTP/1.1\r\n";
// 
//     char *path = get_path_from_request(request);
//     printf("%s\n", path);
// 
//     free(path);
//     return 0;
// 
// }
