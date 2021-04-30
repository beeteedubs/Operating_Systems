/*
 *  Copyright (C) 2021 CS416 Rutgers CS
 *	Tiny File System
 *	File:	tfs.c
 *
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>

#include "block.h"
#include "tfs.h"

char diskfile_path[PATH_MAX];

// Declare your in-memory data structures here
struct superblock* sb;//since not modified much, safe to keep here, other
int num_ibit_bytes = MAX_INUM/8;
int num_dbit_bytes = MAX_DNUM/8;
int numDirectPtr = 16; // hardcode since hardcoded in tfs.h
int numDirentsPerBlk = BLOCK_SIZE/sizeof(struct dirent);

/* 
 * Get available inode number from bitmap
 */
int get_avail_ino() {
	printf("STARTING get_avail_ino()\n");

	// Step 1: Read inode bitmap from disk
	char *buffer = (char*)malloc(sizeof(char)*BLOCK_SIZE); // entire block
	char *ibuff = (char*)malloc(sizeof(char)*num_ibit_bytes);// part of entire block that contains bitmap
	bio_read(1,(void*)buffer);
	int i;
	for(i = 0;i<num_ibit_bytes;i++){//run for however many bytes makeup the inode bitmap
		ibuff[i] = buffer[i];//CHANGE: DON'T NEED IBUFF 
	}
	bitmap_t ibit = (bitmap_t) ibuff; // cast to non-pointer since set_bitmap is weird and doesn't take in pointer but indexes it
	
	// Step 2: Traverse inode bitmap to find an available slot
	for(i = 0; i<MAX_INUM;i++){//note: not num_ibit_bytes since now checking at bit level, by byte by byte
	
	// Step 3: Update inode bitmap and write to disk 
		if(get_bitmap(ibit,i)==0){//if i'th bit is 0, found a free inode index at index "i"
			set_bitmap(ibit,i);
			bio_write(1,(void*)ibit);//write immediately back to disk
			free(ibuff);
			free(buffer);
			printf("ENDING get_avail_ino()\n");
			return i;
		}
	}
	free(ibuff);
	free(buffer);
	printf("ENDING get_avail_ino()\n");
	return -1;
}

/* 
 * Get available data block number from bitmap
 */
int get_avail_blkno() {

	// Step 1: Read data block bitmap from disk	

	// Step 2: Traverse data block bitmap to find an available slot

	// Step 3: Update data block bitmap and write to disk 

	return 0;
}

/* 
 * inode operations
 */
int readi(uint16_t ino, struct inode *inode) {
	print("STARTING readi()\n");
  	// Step 1: Get the inode's on-disk block number
  	int blkNum = (ino *sizeof(struct inode))/BLOCK_SIZE;

  	// Step 2: Get offset of the inode in the inode on-disk block
  	int blkOffset = (ino *sizeof(struct inode))%BLOCK_SIZE;

  	// Step 3: Read the block from disk and then copy into inode structure
	char buffer[BLOCK_SIZE];
	int blkLocation = blkNum + sb->i_start_blk;

	bio_read(blkLocation, (void*)buffer);
	char *ibuff = (char*)malloc(sizeof(struct inode));
	for(int i = 0; i < sizeof(struct inode); i++){
		ibuff[i] = buffer[blkOffset * sizeof(struct inode) + i];
	}

	inode->ino = ((struct inode*) ibuff)->ino;
	inode->valid = ((struct inode*) ibuff)->valid;
	inode->size = ((struct inode*) ibuff)->size;
	inode->type = ((struct inode*) ibuff)->type;
	inode->link = ((struct inode*) ibuff)->link;
	inode->vstat = ((struct inode*) ibuff)-> vstat;

	memcpy(inode->direct_ptr, ((struct inode*) ibuff)->direct_ptr, sizeof(inode->direct_ptr));
	memcpy(inode->indirect_ptr, ((struct inode*) ibuff)->indirect_ptr, sizeof(inode->indirect_ptr);

	printf("ENDING readi()\n");
	return 0;
}

int writei(uint16_t ino, struct inode *inode) {
	printf("STARTING writei()\n");

	// Step 1: Get the block number where this inode resides on disk
	int blkNum = (ino *sizeof(struct inode))/BLOCK_SIZE; // ch 40 page 5
		
	// Step 2: Get the offset in the block where this inode resides on disk
	int blkOffset = (ino *sizeof(struct inode))%BLOCK_SIZE; 

	// Step 3: Write inode to disk 
	char buffer[BLOCK_SIZE]; // don't know y didn't do this before
	int blkLocation = blkNum + sb->i_start_blk;

	bio_read(blkLocation,(void*)buffer); // read into buffer
	char *ibuff = (char*)inode;
	for(int i = 0; i<sizeof(struct inode);i++){
		buffer[blkOffset * sizeof(struct inode) + i] = ibuff[i];
	}
	bio_write(blkLocation,(void*)buffer);

	printf("ENDING writei()\n");
	return 0;
}


/* 
 * directory operations
 */
int dir_find(uint16_t ino, const char *fname, size_t name_len, struct dirent *dirent) {
	printf("STARTING dir_find()\n");

 	// Step 1: Call readi() to get the inode using ino (inode number of current directory)
	struct inode *inode = (struct inode*)malloc(sizeof(struct inode)); // CHANGE, DON'T MALLOC
	readi(ino,inode);
	
  	// Step 2: Get data block of current directory from inode
	//char* buffer = (char*)malloc(sizeof(char)*BLOCK_SIZE);//CHANGE, DON'T MALLOC
	int blkIndex = 0;
	int ptrIndex = 0;
	for(ptrIndex = 0; ptrIndex<numDirectPtr;ptrIndex++){
		if((inode->direct_ptr)[ptrIndex] >=0){//set to -1 if not used
			//copy whole block to buffer
			char* buffer = (char*)malloc(sizeof(char)*BLOCK_SIZE);//CHANGE, DELETE + UNCOMMENT 132
			int blkNum = (inode->direct_ptr)[ptrIndex] + sb->d_start_blk; // need to add to start blk number
			bio_read(blkNum,(void*)buffer);

			for(blkIndex = 0; blkIndex < numDirentsPerBlock; blkIndex ++){





  // Step 3: Read directory's data block and check each directory entry.
  //If the name matches, then copy directory entry to dirent structure

	return 0;
}

int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	
	// Step 2: Check if fname (directory name) is already used in other entries

	// Step 3: Add directory entry in dir_inode's data block and write to disk

	// Allocate a new data block for this directory if it does not exist

	// Update directory inode

	// Write directory entry

	return 0;
}

int dir_remove(struct inode dir_inode, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and checks each directory entry of dir_inode
	
	// Step 2: Check if fname exist

	// Step 3: If exist, then remove it from dir_inode's data block and write to disk

	return 0;
}

/* 
 * namei operation
 */
int get_node_by_path(const char *path, uint16_t ino, struct inode *inode) {
	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// Note: You could either implement it in a iterative way or recursive way

	return 0;
}

/* 
 * Make file system
 */
int tfs_mkfs() {
	printf("STARTING tfs_mkfs()\n");
	// Call dev_init() to initialize (Create) Diskfile
	dev_init(diskfile_path);

	// write superblock information
	sb = (struct superblock*) malloc( sizeof(struct superblock));	
    sb->magic_num = MAGIC_NUM;
    sb->max_inum = MAX_INUM;
    sb->max_dnum = MAX_DNUM;
	sb->i_bitmap_blk = 1;
	sb->d_bitmap_blk = 2;
	sb->i_start_blk = 3; // 64 blocks (16 per block for 1024 inodes)
	sb->d_start_blk = 67;
	bio_write(0,(void*)sb);

	// initialize inode bitmap
	bitmap_t ibit = (bitmap_t) malloc(sizeof(char)*num_ibit_bytes);// MAX_INUM = # of bits, convert to # of bytes by /8, * by size of 1 byte

	memset(ibit,0,num_ibit_bytes);
	bio_write(1,(void*)ibit);

	// initialize data block bitmap
	bitmap_t dbit = (bitmap_t) malloc(sizeof(char)*num_dbit_bytes);//sizeof(char) just cuz it's same thing as 1 byte, could just as easily say 1
	memset(dbit,0,num_dbit_bytes);
	bio_write(2,(void*)dbit);

	// update bitmap information for root directory
	set_bitmap(ibit,0); // root directory at 0th node

	// update inode for root directory
	struct inode* rootI = (struct inode*)malloc(sizeof(struct inode));
	rootI->ino=0;
	rootI->valid = 1;
	rootI->size = 0;
	rootI->type = 1;
	rootI->link = 1; // typically directory
	int counter = 0;
    for(counter = 0; counter < (sizeof(rootI->direct_ptr) / sizeof(int)); counter++){
        (rootI->direct_ptr)[counter] = -1;//set all 16 ptrs to -1
    }
    for(counter = 0; counter < (sizeof(rootI->indirect_ptr) / sizeof(int)); counter++){
        (rootI->indirect_ptr)[counter] = -1;//set all 8 ptrs to -1
    }
    // set the vstat values
    (rootI->vstat).st_ino = 0;
    (rootI->vstat).st_mode = S_IFDIR | 0755;
    (rootI->vstat).st_uid = getuid();
    (rootI->vstat).st_gid = getgid();
    (rootI->vstat).st_nlink = 1;
    (rootI->vstat).st_size = 0;
	//writei(0,rootI);

	printf("ENDING tfs_mkfs()\n");
	return 0;
}


/* 
 * FUSE file operations
 */
static void *tfs_init(struct fuse_conn_info *conn) {

	// Step 1a: If disk file is not found, call mkfs
	if(dev_open(diskfile_path)==-1){
		puts("Running tfs_mkfs()");
		tfs_mkfs();
	}

 	// Step 1b: If disk file is found, just initialize in-memory data structures
  	// and read superblock from disk
	char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
    bio_read(0,(void*) buffer);
    sb = (struct superblock*) buffer;


	return NULL;
}

static void tfs_destroy(void *userdata) {

	// Step 1: De-allocate in-memory data structures

	// Step 2: Close diskfile
	dev_close();

}

static int tfs_getattr(const char *path, struct stat *stbuf) {

	// Step 1: call get_node_by_path() to get inode from path

	// Step 2: fill attribute of file into stbuf from inode

		stbuf->st_mode   = S_IFDIR | 0755;
		stbuf->st_nlink  = 2;
		time(&stbuf->st_mtime);

	return 0;
}

static int tfs_opendir(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

    return 0;
}

static int tfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: Read directory entries from its data blocks, and copy them to filler

	return 0;
}


static int tfs_mkdir(const char *path, mode_t mode) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target directory to parent directory

	// Step 5: Update inode for target directory

	// Step 6: Call writei() to write inode to disk
	

	return 0;
}

static int tfs_rmdir(const char *path) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of target directory

	// Step 3: Clear data block bitmap of target directory

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target directory in its parent directory

	return 0;
}

static int tfs_releasedir(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target file to parent directory

	// Step 5: Update inode for target file

	// Step 6: Call writei() to write inode to disk

	return 0;
}

static int tfs_open(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

	return 0;
}

static int tfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {

	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: copy the correct amount of data from offset to buffer

	// Note: this function should return the amount of bytes you copied to buffer
	return 0;
}

static int tfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: Write the correct amount of data from offset to disk

	// Step 4: Update the inode info and write it to disk

	// Note: this function should return the amount of bytes you write to disk
	return size;
}

static int tfs_unlink(const char *path) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of target file

	// Step 3: Clear data block bitmap of target file

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target file in its parent directory

	return 0;
}

static int tfs_truncate(const char *path, off_t size) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_release(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
	return 0;
}

static int tfs_flush(const char * path, struct fuse_file_info * fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_utimens(const char *path, const struct timespec tv[2]) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}


static struct fuse_operations tfs_ope = {
	.init		= tfs_init,
	.destroy	= tfs_destroy,

	.getattr	= tfs_getattr,
	.readdir	= tfs_readdir,
	.opendir	= tfs_opendir,
	.releasedir	= tfs_releasedir,
	.mkdir		= tfs_mkdir,
	.rmdir		= tfs_rmdir,

	.create		= tfs_create,
	.open		= tfs_open,
	.read 		= tfs_read,
	.write		= tfs_write,
	.unlink		= tfs_unlink,

	.truncate   = tfs_truncate,
	.flush      = tfs_flush,
	.utimens    = tfs_utimens,
	.release	= tfs_release
};


int main(int argc, char *argv[]) {
	int fuse_stat;

	getcwd(diskfile_path, PATH_MAX);
	strcat(diskfile_path, "/DISKFILE");

	fuse_stat = fuse_main(argc, argv, &tfs_ope, NULL);

	return fuse_stat;
}

