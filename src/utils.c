#include "utils.h"
#include <stdio.h>
const char* get_treasure_filename(int id) {
    static char path[64];  // ← será a cópia segura
    DIR *dir = opendir("objetos");
    if (!dir) return NULL;

    char prefix[16];
    snprintf(prefix, sizeof(prefix), "%d.", id + 1);  // ex: "3."

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            strncpy(path, entry->d_name, sizeof(path));
            path[sizeof(path) - 1] = '\0';  // força terminação
            closedir(dir);
            return path;
        }
    }

    closedir(dir);
    return NULL;  // não encontrado
}



bool ends_with(const char *name, const char *suf)
{
    size_t n = strlen(name), m = strlen(suf);
    if (m > n) return false;
    return memcmp(name + n - m, suf, m) == 0;
}