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
	printf("STARTING get_avail_blkno(\n");
	// Step 1: Read data block bitmap from disk
    char* dbuff = (char*) malloc( sizeof(char) * (MAX_DNUM / 8));
    char* buffer = (char*) malloc( sizeof(char) * BLOCK_SIZE);
    bio_read(2,(void*) buffer);
    
    // copy first bytes to dbuff
    int i = 0;
    for(i = 0; i < MAX_DNUM / 8; i++){
        dbuff[i] = buffer[i];
    }
	
    // cast bitmap
    bitmap_t dbit = (bitmap_t) dbuff;
	
    // Step 2: Traverse data block bitmap to find an available slot
    for(i = 0; i < MAX_DNUM; i++){
        if( get_bitmap(dbit,i) == 0 ){
            // counter is now the ino that is fre
	        // Step 3: Update data block bitmap and write to disk 
            set_bitmap(dbit,i); 
            bio_write(2,(void*) dbit);
            // memset the data block
            char* newBuff = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            memset(newBuff, 0, BLOCK_SIZE);
            bio_write( i + sb->d_start_blk, (void*) newBuff );
            printf("ENDING get_avail_blkno(): success\n");
            // return the block number
            return i;
        }
    }
	printf("ENDING get_avail_blkno(): failure\n");
	return -1;
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
	int direntIndex = 0;
	int ptrIndex = 0;
	int byteIndex = 0;
	for(ptrIndex = 0; ptrIndex<numDirectPtr;ptrIndex++){//loop the inode's direct pointers
		if((inode->direct_ptr)[ptrIndex] >=0){//set to -1 if not used, so check if used
			//copy whole block to buffer
			char* buffer = (char*)malloc(sizeof(char)*BLOCK_SIZE);//CHANGE, DELETE + UNCOMMENT 132
			int blkNum = (inode->direct_ptr)[ptrIndex] + sb->d_start_blk; // need to add to start blk number
			bio_read(blkNum,(void*)buffer);

			for(direntIndex = 0; direntIndex < numDirentsPerBlk; direntIndex ++){//loop dirents in buffer
				char* dBuffer = (char*) malloc(sizeof(char)*sizeof(struct dirent));//CHANGE, MOVE WITH "buffer = malloc"
				
				for(byteIndex = 0; byteIndex<sizeof(struct dirent);byteIndex ++){
					dBuffer[byteIndex] = buffer[byteIndex + direntIndex*sizeof(struct dirent)];
				}

				if(((struct dirent*)dBuffer)->valid == 0){// check if dirent is invalid/0/not set
					continue;
				}
				
				if(strcmp(((struct dirent*)dBuffer)->name, fname)==0){ //name matches, copy into input
					dirent->ino = ((struct dirent*)dBuffer)->ino;
					dirent->valid = ((struct dirent*)dBuffer)->valid;
					dirent->len = ((struct dirent*)dBuffer)->len;
					strcpy(dirent->name, ((struct dirent*)dBuffer)->name);
					printf("ENDING dir_find(): found\n");
					return 0;
				}
			}
		}
	}
	
  // Step 3: Read directory's data block and check each directory entry.
  //If the name matches, then copy directory entry to dirent structure
	printf("ENDING dir_find(): not found\n");
	return -1;
}

int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {
	printf("STARTING dir_add()\n");
	/*
	- use dir_find() if file/directory already exists
	- 3x nested loop find next invalid block to associate with new file
	*/
	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	// Step 2: Check if fname (directory name) is already used in other entries

	struct dirent* dirent = (struct dirent*)malloc(sizeof(struct dirent));
	int isUsed = dir_find(dir_inode.ino, fname, name_len, dirent);

	if(isUsed == 0){
		printf("ENDING dir_add(): name in use\n");
		return -1;
	}

	// Step 3: Add directory entry in dir_inode's data block and write to disk
	int isWrote = 0;
	int firstPtr = -1; // keep track of first invalid block index of this inode
	// loop through direct ptrs
	int directIndex = 0;
	int direntIndex = 0;
	int byteIndex = 0;
	for(directIndex = 0; directIndex < numDirectPtr; directIndex ++){
		if(dir_inode.direct_ptr[directIndex] >= 0){// check if points to a block, cuz initialized to -1
			// copy whole block into buffer again
			char* buffer = (char*)malloc(sizeof(char)*BLOCK_SIZE);//CHANGE, DELETE + UNCOMMENT 132
			int blkNum = (dir_inode.direct_ptr)[directIndex] + sb->d_start_blk; // need to add to start blk number
			bio_read(blkNum,(void*)buffer);

			for(direntIndex = 0; direntIndex < numDirentsPerBlk; direntIndex ++){// loop each dirent in block
				char *dBuffer = (char*)malloc(sizeof(char)*sizeof(struct dirent));
			
				for(byteIndex = 0; byteIndex < sizeof(struct dirent); byteIndex++){// copy over dirent byte by byte
					dBuffer[byteIndex] = buffer[byteIndex + direntIndex*sizeof(struct dirent)];
				}

				if(((struct dirent*)dBuffer)->valid==0){//unlike dir_find(), need to add if NOT valid
					((struct dirent*) dBuffer)->valid = 1;
                    ((struct dirent*) dBuffer)->ino = f_ino;
                    ((struct dirent*) dBuffer)->len = name_len;
					strcpy(((struct dirent*)dBuffer)->name,fname);

					for(byteIndex = 0; byteIndex < sizeof(struct dirent);byteIndex++){//add dirent back to buffer
						buffer[byteIndex + directIndex*sizeof(struct dirent)] = dBuffer[byteIndex];
					}

					bio_write(blkNum,(void*)buffer);//write back to "disk"
					isWrote = 1;
					break;
				}
			}
		}else{
			if(firstPtr == -1){// really just care of -1 or not
				firstPtr = directIndex;//firstPtr = first index of direct ptr that "points" to invalid block index, if stays -1 then all directPtrs point to (totally) used blcok
			}
		}
		if(isWrote == 1){
			break;
		}

	}

	if(isWrote == 0){//not written
		if(firstPtr == -1){//because all direct ptrs weren't pointing to any valid block's indeces
			printf("ENDING dir_find(): no direct ptrs were valid\n");
			return -1;
		}
		// no more rom in allocated blocks, but there's a free direct ptr so use that to tpoint to a new block
		int newDataBlock = get_avail_blkno();
		if(newDataBlock == -1){
			printf("ENDING dir_find(): LMAO there's actually no free blocks left\n");
			return -1;
		}
		dir_inode.direct_ptr[firstPtr] = newDataBlock;
		// Allocate a new data block for this directory if it does not exist

		struct dirent* tempDirent = (struct dirent*) malloc( sizeof(struct dirent) );
        tempDirent->ino = f_ino;
        tempDirent->valid = 1;
        tempDirent->len = name_len;
        strcpy(tempDirent->name, fname);
        // cast it to a buffer
        char* direntBuffer = (char*) tempDirent;
        // create a block buffer and add this
        char* buffer = (char*) malloc( sizeof(char*) * BLOCK_SIZE);
        // copy over dirent to block buffer
        int counter = 0;
        for(counter = 0; counter < sizeof(struct dirent); counter++){
            buffer[counter] = direntBuffer[counter];
        }
        // write it to the disk (newDataBlock is 0 indexed from data segment. Have to incldue blocks before)
        bio_write( newDataBlock + sb->d_start_blk, (void*) buffer);

	    // Update directory inode
        // UDATE SIZE                                                       ********
        dir_inode.size += BLOCK_SIZE;
	}
	// Write directory entry

	writei(dir_inode.ino, &dir_inode);
	printf("ENDING dir_find(): success\n");

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
	printf("STARTING get_node_by_path()\n");	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// copy into temp var
	char tempPath[PATH_MAX];
	strcpy(tempPath,path);

	// tokenize
	char* token = strtok(tempPath,"/");

	// result of dir_find()
	int findResult;

	int curIno = ino;

	struct dirent* dirent = (struct dirent*)malloc(sizeof(struct dirent));
	
	while(token!=NULL){
		findResult = dir_find(curIno, token, strlen(token)+1,dirent);
		if(findResult == -1){//failed
			printf("ENDING get_node_by_path(): failed to find file/directory \n");
			return -1;
		}
		// shift root directory
		curIno = dirent->ino;
		// get next token
		token = strtok(NULL,"/");
	}
	// get inode data
	readi(curIno,inode);
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
	printf("STARTING tfs_create()\n");
	// Step 1: Use dirname() and basename() to separate parent directory path and target file name
	char* dirCopy = strdup(path);
	char* baseCopy = strdup(path);
	char* dirName = dirname(dirCopy);
	char* baseName = basename(baseCopy);

	// Step 2: Call get_node_by_path() to get inode of parent directory
	struct inode* tempInode = (struct inode*)malloc(sizeof(struct inode));
	int getNodePathRes = get_node_by_path(dirName,0,tempInode);
	if(getNodePathRes != 0){//if failed
		printf("ENDING tfs_create(): couldn't find directory\n");
		return -1;
	}
	
	// Step 3: Call get_avail_ino() to get an available inode number
	int inoNum = get_avail_ino();
	if(inoNum == -1){
		printf("ENDING tfs_create(): no available inodes left\n");
		return -1;
	}

	// Step 4: Call dir_add() to add directory entry of target file to parent directory
	int addRes = dir_add(*tempInode, inoNum, baseName,strlen(baseName)+1);
	if(addRes== -1){
		printf("ENDING tfs_create(): couldn't add\n");
		return -1;
	}

	// Step 5: Update inode for target file
    struct inode* targetInode = (struct inode*) malloc(sizeof(struct inode));
    targetInode->ino = inoNum;
    targetInode->valid = 1;
    targetInode->size = 0;
    targetInode->type = 0; // file
    targetInode->link = 1;
    /* loop through all values and set them to -1 */
    int counter = 0;
    for(counter = 0; counter < (sizeof(targetInode->direct_ptr) / sizeof(int)); counter++){
        (targetInode->direct_ptr)[counter] = -1;
    }
    for(counter = 0; counter < (sizeof(targetInode->indirect_ptr) / sizeof(int)); counter++){
        (targetInode->indirect_ptr)[counter] = -1;
    }	
    /* set the vstat values */
    (targetInode->vstat).st_ino = inoNum;
    (targetInode->vstat).st_mode = S_IFREG | mode;
    (targetInode->vstat).st_uid = getuid();
    (targetInode->vstat).st_gid = getgid();
    (targetInode->vstat).st_nlink = 1;
    (targetInode->vstat).st_size = 0;
    time( &((targetInode->vstat).st_atime) );
    time( &((targetInode->vstat).st_mtime) );

	// Step 6: Call writei() to write inode to disk
    writei(inoNum, targetInode);
	// Step 6: Call writei() to write inode to disk
	printf("ENDING tfs_create(): success\n");
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

