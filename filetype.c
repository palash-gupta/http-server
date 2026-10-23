#define FTYPE_PLAIN 0
#define FTYPE_JPEG  1
#define FTYPE_PNG   2
#define FTYPE_GIF   3
#define FTYPE_WEBP  4
#define FTYPE_MPEG  5
#define FTYPE_WAV   6
#define FTYPE_MP4   7
#define FTYPE_WEBM  8

const char *ftypes[] = {"jpg", "jpeg", "png", "gif", "webp", "mp3", "wav", "mp4", "webm"};
const uint8_t type_nums[] = {FTYPE_JPEG, FTYPE_JPEG, FTYPE_PNG, FTYPE_GIF, FTYPE_WEBP, FTYPE_MPEG, FTYPE_WAV, FTYPE_MP4, FTYPE_WEBM};

uint8_t filetype(const char* path) {

    size_t n = strlen(path);

    for (size_t i = 0; i < (sizeof(ftypes)/sizeof(char *)); i++) {
        size_t l = strlen(ftypes[i]);
        if (!strncmp(path+n-l, ftypes[i], l)) return type_nums[i];
    }

    return 0;
}
