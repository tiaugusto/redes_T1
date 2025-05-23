#include "utils.h"
#include <stdio.h>
const char *get_treasure_filename(int id) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "tesouro_%d.txt", id); // ou .dat
    return buf;
}