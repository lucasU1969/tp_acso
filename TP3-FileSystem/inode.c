#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "ino.h"

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1 || !(inp)) return -1;

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
    if (!(inp->i_mode & IALLOC) || blockNum < 0) return -1;

    if (!(inp->i_mode & ILARG)) {
        if (blockNum > 7) return -1;
        return inp->i_addr[blockNum];
    }

    int pointers_per_block = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    int idx_in_i_addr = blockNum / pointers_per_block;

    // Indirección simple
    if (idx_in_i_addr < 7) {
        int idx_in_block = blockNum % pointers_per_block;

        if (inp->i_addr[idx_in_i_addr] == 0) return -1;  // bloque de indirección no asignado

        int16_t* block = (int16_t*) malloc(DISKIMG_SECTOR_SIZE);
        if (!block) return -1;

        int result = diskimg_readsector(fs->dfd, inp->i_addr[idx_in_i_addr], block);
        if (result < 0) {
            free(block);
            return -1;
        }

        if (idx_in_block >= pointers_per_block) {  // seguridad por si hay corrupción
            free(block);
            return -1;
        }

        int block_abs_idx = block[idx_in_block];
        free(block);
        return block_abs_idx;
    }

    // Indirección doble
    int remaining = blockNum - 7 * pointers_per_block;
    int idx_in_1block = remaining / pointers_per_block;
    int idx_in_2block = remaining % pointers_per_block;

    if (inp->i_addr[7] == 0) return -1;  // bloque de doble indirección no asignado

    int16_t* indirect1 = (int16_t*) malloc(DISKIMG_SECTOR_SIZE);
    if (!indirect1) return -1;

    if (diskimg_readsector(fs->dfd, inp->i_addr[7], indirect1) < 0) {
        free(indirect1);
        return -1;
    }

    if (idx_in_1block >= pointers_per_block || indirect1[idx_in_1block] == 0) {
        free(indirect1);
        return -1;
    }

    int16_t* indirect2 = (int16_t*) malloc(DISKIMG_SECTOR_SIZE);
    if (!indirect2) {
        free(indirect1);
        return -1;
    }

    if (diskimg_readsector(fs->dfd, indirect1[idx_in_1block], indirect2) < 0) {
        free(indirect1);
        free(indirect2);
        return -1;
    }

    if (idx_in_2block >= pointers_per_block) {
        free(indirect1);
        free(indirect2);
        return -1;
    }

    int block_abs_idx = indirect2[idx_in_2block];
    free(indirect1);
    free(indirect2);
    return block_abs_idx;
}


int used_bytes_in_block(struct inode *inp, int blockNum){
    int file_size = inode_getsize(inp);
    int total_used_blocks = (file_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum < 0 || blockNum >= total_used_blocks) return -1;

    if (blockNum == total_used_blocks - 1) {
        int remainder = file_size % DISKIMG_SECTOR_SIZE;
        return (remainder == 0) ? DISKIMG_SECTOR_SIZE : remainder;
    }

    return DISKIMG_SECTOR_SIZE;
}



int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
