/*************** type.h file ************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

#define FREE        0
#define READY       1

#define BLKSIZE  1024
#define NMINODE   128
#define NMTABLE   100
#define NFD        16
#define NPROC       2
#define NOFT       40

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mount *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

typedef struct mtable{
  int dev;           // device number (0 for FREE)
  int ninodes;       // from superblock
  int nblocks;
  int free_blocks;   // from superblock and GD
  int free_inodes;
  int bmap;          // from group descriptor
  int imap;
  int iblock;        // inodes start block
  MINODE *mntDirPtr; // mount point DIR pointer
  char devName[64];  // device name
  char mntName[64];  // mount point DIR name
}MTABLE;

// globals
MINODE minode[NMINODE];
MTABLE mtable[NMTABLE];
MINODE *root;
OFT oft[NOFT];

PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int  nname;      // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;

MINODE *iget();