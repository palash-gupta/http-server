#define MAX_PATH_LEN 4096

int check_path(const char *path);

char *get_path_from_request(char *request) {
    if (!strncmp(request, "GET", 3)) {
        char *s_loc = strstr(request, " HTTP");
        if (s_loc == NULL) {
            fprintf(stderr, "Path not found, returning 404.\n");
            return NULL;
        }
        
        int path_size = s_loc - request - 3 + 1; // +1 just in case the path is a directory
        char *path = malloc(sizeof(char) * path_size); 

        strncpy(path+1, request+4, path_size);

        path[0] = '.'; // local path
        path[path_size-1] = '\0';

        if (check_path(path) == C_ISDIR && path_size > 3) {
            path[path_size-1] = '/';
            path[path_size] = '\0';
        }

        return path;

    } else {
        return NULL;
    }
}
