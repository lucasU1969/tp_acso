#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "ino.h"


#define min(a, b) ((a) < (b) ? (a) : (b))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1 || !(inp) || !(fs)) return -1;

    int32_t  size_of_inode = sizeof(struct inode);                                          // tamaño inode struct
    int32_t bytes_offset = size_of_inode*(inumber-1);                                       // offset en bytes de donde empieza el inode
    int32_t blocks_offset = bytes_offset / DISKIMG_SECTOR_SIZE;                             // offset en bloques de donde empieza el inode
    if ((fs -> superblock).s_isize <= blocks_offset) return -1;                             // el inumber es válido

    int32_t bytes_offset_wrt_block = bytes_offset - (blocks_offset * DISKIMG_SECTOR_SIZE);
    int32_t absolute_block_idx = INODE_START_SECTOR + blocks_offset;
    int8_t* block_data = (int8_t*) malloc(DISKIMG_SECTOR_SIZE);
    if (!block_data) return -1;
    
    int result =  diskimg_readsector(fs->dfd, absolute_block_idx, block_data);              
    if (result < 0) {                                                                       // verificar que el read no falle
        free(block_data);  
        return -1;
    }
    memcpy(inp, block_data + bytes_offset_wrt_block, size_of_inode);                        // copiar la memoria de disco a la sección apuntada por inp
    free(block_data);
    return 0; 
}


int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
    if (!(inp->i_mode & IALLOC) || blockNum < 0 || !(fs)) return -1;

    if (!(inp->i_mode & ILARG)) {                                                           // directo (cada bloque contiene los datos)
        if (blockNum > 7) return -1;
        return inp->i_addr[blockNum];
    }

    int pointers_per_block = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    int idx_in_i_addr = min(blockNum / pointers_per_block, 7);

    int idx_in_block_simple = blockNum % pointers_per_block;

    if (!(inp -> i_addr [idx_in_i_addr]) || 
        (blockNum >= (pointers_per_block*(7 + pointers_per_block))) || 
        ( inode_getsize(inp) < blockNum * DISKIMG_SECTOR_SIZE)) return -1;                  // el bloque no está asignado o es mayor al tamaño del archivo o el maximo posible.

    int16_t* si_block = (int16_t*) malloc(DISKIMG_SECTOR_SIZE); 
    if (!si_block) return -1;
    
    int result_si = diskimg_readsector(fs-> dfd, inp -> i_addr[idx_in_i_addr], si_block);   // cargo el bloque en la porción de memoria reservada por malloc.
    if (result_si<0){
        free(si_block);
        return -1;
    }

    if (idx_in_i_addr < 7) {                                                                // estoy en el caso de indirección simple
        int block_abs_idx = si_block[idx_in_block_simple];
        free(si_block);
        return block_abs_idx;
    }

    // indirección doble
    int idx_in_si_block = (blockNum - 7*pointers_per_block) / pointers_per_block; 
    int idx_in_di_block = (blockNum - 7*pointers_per_block) % pointers_per_block;

    if (!(si_block[idx_in_si_block])) {                                                     // no hay dato en el bloque
        free(si_block);
        return -1;
    }

    int16_t* di_block = (int16_t*) malloc(DISKIMG_SECTOR_SIZE);
    if (!(di_block)) {
        free(si_block);
        return -1;
    }

    if ((diskimg_readsector(fs->dfd, si_block[idx_in_si_block], di_block) < 0)) {
        free(si_block);
        free(di_block);
        return -1;
    }

    int block_abs_idx = di_block[idx_in_di_block];
    free(si_block);
    free(di_block);
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
