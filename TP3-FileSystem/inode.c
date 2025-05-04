#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "ino.h"

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) return -1;

    uint16_t s_isize = (fs -> superblock).s_isize;                  // size in blocks of I list
    int32_t  size_of_inode = sizeof(struct inode);                  // size of the inode struct
    int32_t bytes_offset = size_of_inode*(inumber-1);               // offset measured in bytes where the inode starts
    int32_t blocks_offset = bytes_offset / DISKIMG_SECTOR_SIZE;     //offset measured in blocks where the inode starts
    if (s_isize <= blocks_offset) return -1;                        // the inode number is invalid

    int32_t bytes_offset_wrt_block = bytes_offset - (blocks_offset * DISKIMG_SECTOR_SIZE);

    int32_t absolute_block_idx = INODE_START_SECTOR + blocks_offset;

    int8_t* block_data = (int8_t*) malloc(DISKIMG_SECTOR_SIZE);
    if (!block_data) return -1;

    int result =  diskimg_readsector(fs->dfd, absolute_block_idx, block_data); 
    if (result < 0) {
        free(block_data);  
        return -1;
    }
    memcpy(inp, block_data + bytes_offset_wrt_block, size_of_inode);
    free(block_data);
    return 0; 
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
    
    return 0;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
