#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

const char *get_treasure_filename(int id);

bool ends_with(const char *name, const char *suf);