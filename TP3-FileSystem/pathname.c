
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "direntv6.h"


int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') return -1;

    char path_copy[strlen(pathname) + 1];
    strcpy(path_copy, pathname);

    char *token = strtok(path_copy, "/");
    int dirinumber = 1;

    struct direntv6 entry;

    while (token != NULL) {
        if (directory_findname(fs, token, dirinumber, &entry) < 0) {
            return -1;
        }
        dirinumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return dirinumber;
}