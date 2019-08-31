#include "lib.h"
typedef unsigned int u64;
typedef int u32;
typedef short int s16;


#define MAX_OBJS 1<<20          // >10^6
#define MAX_BLOCKS 1 << 23   // 32GB/4KB
#define MAX_INODE 1<<20        // =One inode pr obj
#define OBJ_SIZE 48           // 4*4 + 32
#define OBJS_IN_BLOCK 85      // 4K/OBJ_SIZE
#define BLOCKS_FOR_BLOCK_BMAP 1<<5  // don't know 
#define BLOCKS_FOR_INODE_BMAP 1<<8 // number of block required to store info about a objects //
#define BLOCKS_FOR_OBJS 1<<11   // max obj / obj per block
#define BLOCKS_FOR_HASH_TABLE 15000 //don't know
u32 *block_bitmap;
u32 *hash_table;
u32 *inode_bitmap;

//15000 hashtable

#define BLOCKS_FOR_C_BITMAP 1 << 8

struct object{
     int id;
     int size;
     int cache_index;
     int dirty;
     char key[32];
     u32 indirectPtr[4];
};
typedef struct hash_table {
  int hash;
  int id[3];
}hash_map;


ids * arr[MAX_OBJS];
struct object *objs;
u32 *superBlock;
#define malloc_4k(x) do{\
                         (x) = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 
#define free_4k(x) munmap((x), BLOCK_SIZE)

#define modified_malloc(x,y) do{\
                         (x) = mmap(NULL, BLOCK_SIZE*y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 
#define modified_free(x,y) munmap((x), BLOCK_SIZE*y)


/*
Returns the object ID.  -1 (invalid), 0, 1 - reserved
*/

int hash(char *str)
{
    int hash = 9999;
    int c;
    while (c = *str++)
        hash = (((hash << 8) + 2*hash) + 3*c)% MAX_OBJS; 
    return hash;
}

long find_object_id(const char *key, struct objfs_state *objfs)
{
    int ctr;
    struct object *obj = objs;
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(obj->id && !strcmp(obj->key, key))
              return obj->id;
          obj++;
    }      
    return -1;  
}

/*
  Creates a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.

  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/
long create_object(const char *key, struct objfs_state *objfs)
{
    int ctr;
    u32 *super = objfs->objstore_data;

    struct object *obj = objs;
    struct object *free = NULL; 
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(!obj->id && !free){
                free = obj;
                free->id = ctr+2;
          }
          else if(obj->id && !strcmp(obj->key, key)){
              dprintf("%s:duplicates are not allowed ...\n",__func__);
                return -1;
          }
          obj++;
    }      
    
    if(!free){
               dprintf("%s: objstore full\n", __func__);
               return -1;
    } 
    strcpy(free->key, key);
    // init_object_cached(obj);
    return free->id;

   return -1;
}
/*
  One of the users of the object has dropped a reference
  Can be useful to implement caching.
  Return value: Success --> 0
                Failure --> -1
*/
long release_object(int objid, struct objfs_state *objfs)
{
    return 0;
}

/*
  Destroys an object with obj.key=key. Object ID is ensured to be >=2.

  Return value: Success --> 0
                Failure --> -1
*/
long destroy_object(const char *key, struct objfs_state *objfs)
{
    int ctr;
    struct object *obj = objs;
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(obj->id && !strcmp(obj->key, key)){
            //    remove_object_cached(obj);
               obj->id = 0;
               obj->size = 0;
               return 0;
          }
          obj++;
    }      
    return -1;
}

/*
  Renames a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.  
  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/

long rename_object(const char *key, const char *newname, struct objfs_state *objfs)
{
   
   return -1;
}

/*
  Writes the content of the buffer into the object with objid = objid.
  Return value: Success --> #of bytes written
                Failure --> -1
*/
long objstore_write(int objid, const char *buf, int size, struct objfs_state *objfs)
{
   return -1;
}

/*
  Reads the content of the object onto the buffer with objid = objid.
  Return value: Success --> #of bytes written
                Failure --> -1
*/
long objstore_read(int objid, char *buf, int size, struct objfs_state *objfs)
{
   return -1;
}

/*
  Reads the object metadata for obj->id = buf->st_ino
  Fillup buf->st_size and buf->st_blocks correctly
  See man 2 stat 
*/
int fillup_size_details(struct stat *buf)
{
   return -1;
}

/*
   Set your private pointeri, anyway you like.
*/
#define BLOCKS_FOR_BLOCK_BMAP 1<<11   
#define BLOCKS_FOR_INODE_BMAP 1<<8 // number of block required to store info about a objects 
#define BLOCKS_FOR_INODE_TABLE 1<<13 // =MAX_INODE*SIZE OF INODE(32 B)/4K
#define BLOCKS_FOR_OBJS 1<<11   

int objstore_init(struct objfs_state *objfs)
{
   int ctr;
   dprintf("%s: malloc for objs helooooooooooooooooo\n", __func__);
   dprintf("starting address of disk%d",sizeof(objfs->blkdev));
   struct object *obj = NULL;
    modified_malloc(superBlock,1);
    if(!superBlock){
       dprintf("%s: malloc for superblock\n", __func__);
        return -1; 
    }
    modified_malloc(block_bitmap,BLOCKS_FOR_BLOCK_BMAP);
    if(!objs){
        dprintf("%s: malloc for objs\n", __func__);
        return -1;
    }
    obj = objs; 
    modified_malloc(inode_bitmap,BLOCKS_FOR_INODE_BMAP);
    if(!block_bitmap){
        dprintf("%s: malloc for Bmap\n", __func__);
        return -1;
    }
    modified_malloc(hash_map,BLOCKS_FOR_HASH_TABLE);
    if(!inode_bitmap){
        dprintf("%s: malloc for bit map table\n", __func__);
        return -1;
    }
    if(read_block(objfs,0,superBlock)<0){
        dprintf("%s: read block for super block\n", __func__);
        return -1;
    }
    for(int i=0; i< BLOCKS_FOR_BLOCK_BMAP; i++){
        if(read_block(objfs, 1+i, ((char *)block_bitmap) + i * BLOCK_SIZE) < 0){
            dprintf("%s: read block for block for objs\n", __func__);
            return -1;
        }
        
    }
    for(int i=0; i<BLOCKS_FOR_INODE_BMAP;i++){
        if(read_block(objfs, 1+BLOCKS_FOR_BLOCK_BMAP + i, ((char *)inode_bitmap) + i * BLOCK_SIZE) < 0){
             dprintf("%s: read block for block for objs\n", __func__);
             return -1;
        }
    }
    for(int i=0;i<BLOCKS_FOR_HASH_TABLE;i++){
        if(read_block(objfs,1+BLOCKS_FOR_BLOCK_BMAP+BLOCKS_FOR_INODE_BMAP+i,((char *)hash_table)+i*BLOCK_SIZE)<0){
            dprintf("%s: read block for hash table\n", __func__);
             return -1;
        }
    }

    objfs->objstore_data = superBlock;
   dprintf("Done objstore init\n");
   return 0;
}

/*
   Cleanup private data. FS is being unmounted
*/
int objstore_destroy(struct objfs_state *objfs)
{
   struct object *obj = objs;

   for(int i=0; i< BLOCKS_FOR_OBJS; i++){
        if(write_block(objfs, 2+i, ((char *)objs) + i * BLOCK_SIZE) < 0){
            dprintf("%s: read block for block for objs\n", __func__);
            return -1;
        }
        
    }
    for(int i=0; i<BLOCKS_FOR_INODE_BMAP;i++){
        if(write_block(objfs, 2+BLOCKS_FOR_OBJS + i, ((char *)inode_bitmap) + i * BLOCK_SIZE) < 0){
             dprintf("%s: read block for block for objs\n", __func__);
             return -1;
        }
    }
    for(int i=0;i<BLOCKS_FOR_HASH_TABLE;i++){
        if(write_block(objfs,2+BLOCKS_FOR_BLOCK_BMAP+BLOCKS_FOR_INODE_BMAP+i,((char *)hash_table)+i*BLOCK_SIZE)<0){
            dprintf("%s: read block for hash table\n", __func__);
             return -1;
        }
    }
    modified_free(objs,BLOCKS_FOR_OBJS);
    modified_free(inode_bitmap,BLOCKS_FOR_INODE_BMAP);
    modified_free(hash_table,BLOCKS_FOR_HASH_TABLE);
    objfs->objstore_data = NULL;
   dprintf("Done objstore destroy\n");
   return 0;
}
