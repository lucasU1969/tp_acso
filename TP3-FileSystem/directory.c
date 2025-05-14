#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
    struct inode i_node; 
    if (inode_iget(fs, dirinumber, &i_node) < 0) return -1; 

    int size_in_blocks = (inode_getsize(&i_node) + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;              // ceil(bytes_del_archivo / tamaño_de_bloque) para calcular cuantos bloques ocupa el archivo.
    int dirent_size = sizeof(struct direntv6);
    struct direntv6 *block_data = (struct direntv6 *) malloc(DISKIMG_SECTOR_SIZE);                              // interpreto el sector como un "array" de dirents.
    if (!block_data) return -1;

    for (int i = 0; i < size_in_blocks; i++) {                                                                  // itero sobre todos los bloques de datos del directorio (donde están las dirents)
        int used_bytes = file_getblock(fs, dirinumber, i, block_data);                                          // levanto el bloque i y lo cargo en block_data
        if (used_bytes < 0) {
            free(block_data);
            return -1;
        }

        int dirents_in_block = used_bytes / dirent_size;

        for (int j = 0; j < dirents_in_block; j++) {                                                            // itero sobre los dirents en el bloque i
            if (strcmp(block_data[j].d_name, name) == 0) {                                                      // comparo los nombres
                memcpy(dirEnt, &block_data[j], dirent_size);                                                    // cargo el dirent en la posición apuntada por dirEnt
                free(block_data);
                return 0;
            }
        }
    }

    free(block_data);                                                                                           // no se encontró
    return -1;
}
