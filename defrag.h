#ifndef DEFRAG_H_INCLUDED
#define DEFRAG_H_INCLUDED

#define N_DBLOCKS 10
#define N_IBLOCKS 4

typedef struct superblock {
    int size; /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in bytes blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_inode; /* head of free inode list */
    int free_iblock; /* head of free block list */
}SB;

typedef struct inode {
    int next_inode; /* list for free inodes */
    int protect; /* protection field */
    int nlink; /* number of links to this file */
    int size; /* numer of bytes in file */
    int uid; /* owner's user ID */
    int gid; /* owner's group ID */
    int ctime; /* time field */
    int mtime; /* time field */
    int atime; /* time field */
    int dblocks[N_DBLOCKS]; /* pointers to data blocks */
    int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
    int i2block; /* pointer to doubly indirect block */
    int i3block; /* pointer to triply indirect block */
}IN;

#endif // DEFRAG_H_INCLUDED
