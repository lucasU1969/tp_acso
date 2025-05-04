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

    int size_in_blocks = (inode_getsize(&i_node) + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE; 
    int dirent_size = sizeof(struct direntv6);
    struct direntv6 *block_data = (struct direntv6 *) malloc(DISKIMG_SECTOR_SIZE);

    if (!block_data) return -1;

    for (int i = 0; i < size_in_blocks; i++) {
        int used_bytes = file_getblock(fs, dirinumber, i, block_data); 
        if (used_bytes < 0) {
            free(block_data);
            return -1;
        }

        int dirents_in_block = used_bytes / dirent_size;

        for (int j = 0; j < dirents_in_block; j++) {
            if (strcmp(block_data[j].d_name, name) == 0) {
                memcpy(dirEnt, &block_data[j], dirent_size);
                free(block_data);
                return 0;
            }
        }
    }

    free(block_data);
    return -1;
}
