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

    char path_copy[strlen(pathname) + 1];                                       // copio el path para parsear
    strcpy(path_copy, pathname);

    char *token = strtok(path_copy, "/");                                       // parseo por "/", la primera llamada a strtok setea el string 
    int dirinumber = 1;

    struct direntv6 entry;

    while (token != NULL) {
        if (directory_findname(fs, token, dirinumber, &entry) < 0) {            // busco el nombre del próximo directorio en el directorio actual
            return -1;
        }
        dirinumber = entry.d_inumber;                                 
        token = strtok(NULL, "/");
    }

    return dirinumber;
}