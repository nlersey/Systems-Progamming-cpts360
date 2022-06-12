/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/

#include "cd_ls_pwd.c"

//                  0     1      2       3        4        5       6      7//
char *command[] = {"ls", "cd", "pwd", "mkdir", "creat", "rmdir","link","unlink","symlink","readlink","chmod","utime", "quit", 0};

int findCmd(char *cmd)
{
  int i = 0;
  while(command[i])
  {
    if (strcmp(cmd, command[i]) == 0)
      return i;
    i++;
  }
  return -1;
}

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");
 
  // intialize all minodes as FREE
  for (i=0; i<NMINODE; i++)
  {
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }

  // initialize mtables as FREE
  for (i=0; i<NMTABLE; i++)
    mtable[i].dev = 0;

  // initialize ofts as FREE
  for (i=0; i<NOFT; i++)
    oft[i].refCount = 0;

  // initalize PROCs
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
    p->next = &proc[i+1];
  }
  proc[NPROC-1].next = &proc[0]; // circular list
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

char *disk = "diskimage";
int main(int argc, char *argv[ ])
{
  int ino, index;
  char buf[BLKSIZE];
  char line[128], cmd[32], pathname[128];
  char pathname2[128];
  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }
  dev = fd;   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0]; // P0 (superuser process) runs first
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  while(1)
  {
    printf("input command [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|chmod|utime|quit]: ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
      continue;
    pathname[0] = 0;

    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    index = findCmd(cmd);
    switch(index)
    {
      case 0: ls(pathname);             break;
      case 1: chdir(pathname);          break;
      case 2: pwd(running->cwd);        break;
      case 3: mkdir_creat(pathname, 1); break;
      case 4: mkdir_creat(pathname, 2); break;
      case 5: rmdir(pathname);          break;
      case 6: 
              sscanf(line,"%s %s %s",cmd,pathname,pathname2); 
              link(pathname,pathname2);
                                        break;
      case 7:unlink(pathname);          break;
      case 8:
              sscanf(line,"%s %s %s",cmd,pathname,pathname2); 
              symlink(pathname,pathname2);
                                        break;
      case 9: readLink(pathname);       break;
      case 10: funchmod(pathname);      break;
      case 11: utime(pathname);         break;
      case 12: quit();                  break;


      default:
        printf("Invalid command.\n");
        break;
      
    }
  }
}
