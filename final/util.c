/*********** util.c file ****************/
#include "type.h"

MINODE *mialloc()
{
  for (int i = 0; i<NMINODE; i++)
  {
    MINODE *mp = &minode[i];
    if (mp->refCount == 0)
    {
      mp->refCount = 1;
      return mp;
    }
  }
  printf("FS panic: out of minodes\n");
  return 0;
}

int get_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk*BLKSIZE, 0);
  int n = read(dev, buf, BLKSIZE);
  if (n < 0)
    printf("get_block [%d %d] error: %s\n", dev, blk, strerror(errno));
}   
int put_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk*BLKSIZE, 0);
  int n = write(dev, buf, BLKSIZE);
  if (n != BLKSIZE)
    printf("put_block [%d %d] error: %s\n", dev, blk, strerror(errno));
}   

int tokenize(char *pathname)
{
  // Copy pathname into gpath[]; tokenize it into name[0] to name[n-1]
  char *s;
  strcpy(gpath, pathname);
  nname = 0;
  s = strtok(gpath, "/");
  while(s)
  {
    name[nname++] = s;
    s = strtok(0, "/");
  }
}

// Return minode pointer of loaded INODE=(dev, ino)
MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];

  // search in-memory minodes first
  for (i = 0; i <NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount && (mip->dev==dev) && (mip->ino==ino))
    {
      mip->refCount++;
      return mip;
    }
  }

  // needed INODE=(dev,ino) not in memory
  mip = mialloc();                 // allocate a free minode
  mip->dev = dev; mip->ino = ino;  // assign to (dev, ino)
  block = (ino-1)/8 + inode_start; // disk block containing this inode
  offset = (ino-1)%8;              // which inode in this block
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip;                // copy inode to minode.INODE
  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mptr = 0;
  return mip;
}

// Dispose of minode pointed by mip
void iput(MINODE *mip)
{
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];

  if (mip == 0) return;
  mip->refCount--;
  if (mip->refCount > 0) return; // still has user
  if (mip->dirty == 0) return;   // no need to write back

  // write INODE back to disk
  block = (mip->ino - 1) / 8 + inode_start;
  offset = (mip->ino - 1) % 8;

  // get block containing this inode
  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset; // ip points at INODE
  *ip = mip->INODE;           // copy INODE to inode in block
  put_block(mip->dev, block, buf); // write back to disk
  mip->refCount = 0;
} 

// search for name in (DIRECT) data blocks of mip->INODE, return its ino
int search(MINODE *mip, char *name)
{
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;

  // search DIR direct blocks only
  for (int i=0; i<12; i++)
  {
    if (mip->INODE.i_block[i] == 0) return 0;
    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE)
    {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("%8d%8d%8d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
      if (strcmp(name, temp) == 0)
      {
        printf("Found %s : inumber = %d\n", name, dp->inode);
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

// return ino of pathname
int getino(char *pathname)
{
  MINODE *mip;
  int ino;

  if(strcmp(pathname, "/") == 0)
    return 2; // root ino = 2
  if (pathname[0] == '/')
    mip = root; // if absolute pathname, start form root
  else mip = running->cwd; // if relative pathname, start from cwd
  mip->refCount++; // in order to iput(mip) later

  tokenize(pathname);

  // search for each component string
  for (int i=0; i<nname; i++)
  {
    // check DIR type
    //if (!S_ISDIR(mip->INODE.i_mode))
    //{
    //  printf("%s is not a directory\n", name[i]);
    //  iput(mip);
    //  return -1;
    //}
    ino = search(mip, name[i]);
    if (!ino)
    {
      printf("No such component name %s\n", name[i]);
      iput(mip);
      return 0;
    }
    iput(mip);            // release current minode
    mip = iget(dev, ino); // switch to new minode
  }
  iput(mip);
  return ino;
}

// search parent's data block for myino;
// copy its name STRING to myname[ ]
int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;

  // search DIR direct blocks only
  for (int i=0; i<12; i++)
  {
    if (parent->INODE.i_block[i] == 0) return 0;
    get_block(parent->dev, parent->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE)
    {
      if(dp->inode == myino)
      {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        //myname = temp;
        strcpy(myname, temp);
        return 1;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}
int findmyname2(MINODE *parent, u32 myino, char *myname) 
{
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;

  // search DIR direct blocks only
  for (int i=0; i<12; i++)
  {
    if (parent->INODE.i_block[i] == 0) return 0;
    get_block(parent->dev, parent->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE)
    {
      if(dp->inode == myino)
      {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        //myname = temp;
        strcpy(myname, temp);
        return 1;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

// myino = ino of ., return ino of ..
int findino(MINODE *mip, u32 *myino)
{
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;

  get_block(mip->dev, mip->INODE.i_block[0], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;
  while (cp < sbuf + BLKSIZE)
  {
    if(dp->name[0] == '.')
    {
      if (dp->name[1] == '.')
        return dp->inode;
      *myino = dp->inode;
    }      
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}

void clr_bit(char *buf, int bit)
{
  int bitnum1, bitnum2;
  bitnum1 = bit / 8;
  bitnum2 = bit % 8;
  buf[bitnum1] &= ~(1 << bitnum2);
}

void incFreeBlock(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

void incFreeINodes(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int idalloc(int dev, int ino)
{
  char buf[BLKSIZE];
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);
  put_block(dev, imap, buf);
  incFreeINodes(dev);
}

int bdalloc(int dev, int bno)
{
  char buf[BLKSIZE];
  get_block(dev, bmap, buf);
  clr_bit(buf, bno);
  put_block(dev, bmap, buf);
  incFreeBlock(dev);
}

int tst_bit(char *buf, int bit)
{
  return buf[bit/8] & (1 << (bit % 8));
}

void set_bit(char *buf, int bit)
{
  buf[bit/8] |= (1 << (bit % 8));
}

void decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int i;
  char buf[BLKSIZE];

  // Read inode_bitmap block
  get_block(dev, imap, buf);
  for (int i = 0; i < ninodes; i++)
  {
    if (tst_bit(buf, i) == 0)
    {
      set_bit(buf, i);
      put_block(dev, imap, buf);
      // update free inode count in SUPER and GD
      decFreeInodes(dev);
      return (i+1);
    }
  }
  return 0; // out of FREE inodes
}

void decFreeBlock(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int balloc(int dev)
{
  int i;
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);
  for (int i = 0; i < ninodes; i++)
  {
    if (tst_bit(buf, i) == 0)
    {
      set_bit(buf, i);
      decFreeBlock(dev);
      put_block(dev, bmap, buf);
      // update free inode count in SUPER and GD
      //decFreeBlock(dev);
      return (i+1);
    }
  }
  return 0; // out of FREE blocks
}


int truncate(MINODE *mip)
{
  
	//deallocate 12 direct blocks thats the max number each inode has
	for (int i = 0; i < 12; i++)
	{
   // printf("1\n");
		if (mip->INODE.i_block[i] == 0)//guard 
		{
        //  printf("2\n");
			break;
		}
    //    printf("3\n");
		//bdalloc(dev, mip->INODE.i_block[i]);///i have no idea why idalloc works but it does
    idalloc(dev, mip->ino);

       // printf("4\n");

		mip->INODE.i_block[i] = 0;
	}
  //  printf("5\n");

	mip->INODE.i_size = 0;
	mip->INODE.i_blocks = 0;
	mip->dirty = 1;
	iput(mip);

}