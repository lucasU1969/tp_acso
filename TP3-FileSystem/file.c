#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"


int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    if (!fs) return -1;
    struct inode i_node; 
    if (inode_iget(fs, inumber, &i_node) < 0) return -1; 
    
    int abs_block_idx = inode_indexlookup(fs, &i_node, blockNum);
    if (abs_block_idx < 0) return -1;

    if (diskimg_readsector(fs-> dfd, abs_block_idx, buf)<0) return -1;

    return used_bytes_in_block(&i_node, blockNum);
}

