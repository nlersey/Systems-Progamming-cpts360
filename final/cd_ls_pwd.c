/************* cd_ls_pwd.c file **************/
#include "util.c"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int chdir(char *pathname)   
{
  printf("chdir %s\n", pathname);
  int ino = getino(pathname);
  if(ino == 0)
  {
    printf("Error, %s not found\n", pathname);
    return 0;
  }
  MINODE *mip = iget(dev, ino);
  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("%s is not a directory\n", pathname);
    iput(mip);
    return 0;
  }
  iput(running->cwd); // release old cwd
  running->cwd = mip; // change cwd to mip
}

int ls_file(MINODE *mip, char *name)
{
  if (S_ISREG(mip->INODE.i_mode))
    printf("%c",'-');
  if (S_ISDIR(mip->INODE.i_mode))
    printf("%c",'d');
  if (S_ISLNK(mip->INODE.i_mode))
    printf("%c",'l');

  for (int i=8; i >= 0; i--)
  {
    if (mip->INODE.i_mode & (1 << i)) // print r|w|x
      printf("%c", t1[i]);
    else printf("%c", t2[i]); // or print -
  }
  printf("\t%x\t%x\t%x\t", mip->INODE.i_links_count, mip->INODE.i_uid, mip->INODE.i_gid);

  // Print time
  char ftime[64];
  strcpy(ftime, ctime(&mip->INODE.i_mtime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("%s\t",ftime);

  printf("%x\t%s\n", mip->INODE.i_size, name);
}

int ls_dir(MINODE *mip)
{
  char buf[BLKSIZE], type[256], temp[256];
  DIR *dp;
  char *cp;

  // search DIR direct blocks only
  for (int i=0; i<13; i++)
  {
    if (mip->INODE.i_block[i] == 0) continue;
    get_block(mip->dev, mip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;
    while (cp < buf + BLKSIZE)
    {
      MINODE *cip = iget(dev, dp->inode);
      ls_file(cip, dp->name);
      iput(cip);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    printf("\n");
  }
  return 0;
}

void ls(char *pathname)  
{
  printf("ls %s\n", pathname);
  if(!pathname[0])
  {
    ls_dir(running->cwd);
    return;
  }
  int ino = getino(pathname);
  if(ino == 0)
    return;
  MINODE *mip = iget(dev, ino);
  if (!S_ISDIR(mip->INODE.i_mode))
    ls_file(mip, pathname);
  else ls_dir(mip);
  iput(mip);
}

void rpwd(MINODE *wd)
{
  if (wd == root) return;
  u32 myino = 0;
  char name[EXT2_NAME_LEN];
  int parent_ino = findino(wd, &myino);
  MINODE *pip = iget(dev, parent_ino);
  findmyname(pip, myino, name);
  rpwd(pip);
  printf("/%s", name);
}

void pwd(MINODE *wd)
{
  if (wd == root) printf("/\n");
  else rpwd(wd);
  printf("\n");
}

void enter_name(MINODE *pip, int pino, int ino, char *new)
{
  char buf[BLKSIZE];
  int ideal_length = 0, need_length = 0, remain = 0, i = 0;
  need_length = 4 * ((8 + strlen(new) + 3) / 4);
  
  for (i = 0; i < 12; i++)
  {
    if(pip->INODE.i_block[i] == 0) break;
    bzero(buf, BLKSIZE);

    // Step to the last entry in the data block
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    char *cp = buf;
    while(cp + dp->rec_len < buf + BLKSIZE)
    {
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    // dp now points at last entry in block
    ideal_length = dp->name_len;
    ideal_length = 4 * ((8 + dp->name_len + 3) / 4);
    remain = dp->rec_len - ideal_length;
    if (remain >= need_length)
    {
      dp->rec_len = ideal_length;
      cp += dp->rec_len;
      dp = (DIR *)cp;
      dp->name_len = strlen(new);
      strcpy(dp->name, new);
      dp->rec_len = remain;
      dp->inode = ino;
      put_block(pip->dev, pip->INODE.i_block[i], buf);
      return;
    }
  }
  if (i < 11)
  {
    balloc(pip->dev);
    pip->INODE.i_size += BLKSIZE;
    bzero(buf, BLKSIZE);
    dp = (DIR *)buf;
    dp->name_len = strlen(new);
    strcpy(dp->name, new);
    dp->rec_len = BLKSIZE;
    dp->inode = ino;
    put_block(pip->dev, pip->INODE.i_block[i], buf);
  }
  else printf("idk how but you're out of space bro\n");
}

void my_mc(MINODE *pmip, char *bname, int pino, int command)
{
  // 1. Allocate an INODE and a disk block
  int ino = ialloc(dev);
  printf("%d", ino);
  int bno = balloc(dev);

  // 2. Create an INODE
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_atime = ip->i_ctime = ip->i_mtime = time(NULL);
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group ID

  if (command == 1)
  {
    ip->i_mode = 0x41ED;    // 040755: DIR type and permissions
    ip->i_links_count = 2;  // . and ..
    ip->i_size = BLKSIZE;   // size in bytes
    ip->i_blocks = 2;       // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno;   // new DIR has one data block
    for (int i = 1; i < 15; i++)
      ip->i_block[i] = 0;
  }
  else
  {
    ip->i_mode = 0x0644;    // 000604: FILE type and permissions
    ip->i_links_count = 1;
    ip->i_size = 0;
    ip->i_blocks = 0;       // no data block is allocated
    for (int i = 0; i < 15; i++)
      ip->i_block[i] = 0;
  }
  mip->dirty = 1;           // mark minode dirty
  iput(mip);                // write INODE to disk

  // 3. Create data block for new directory containing . and ..
  if (command == 1)
  {
    char buf[BLKSIZE];
    bzero(buf, BLKSIZE);
    DIR *dp = (DIR *)buf;
    // make . entry
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    // make .. entry
    dp = (char *)dp + 12;
    dp->inode = pino;
    dp->rec_len = BLKSIZE-12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(dev, bno, buf);
  }

  // 4. Enter new dir_entry into parent directory
  enter_name(pmip, pino, ino, bname);
}

int mkdir_creat(char *pathname, int command)
{
  MINODE *start;
  if (pathname[0] == '/')
    start = root;
  else
  {
    start = running->cwd;
    dev = running->cwd->dev;
  }

  // 1. Divide pathname into dirname and basename
  int i;
  char dname[256], bname[64];
  tokenize(pathname);
  bzero(dname, 256); bzero(bname, 64);
  if (nname == 1)
  {
    dname[0] = '.';
    strcpy(bname, name[0]);
  }
  else
  {
    strcpy(dname, name[0]);
    for (i = 1; i < nname-1; i++)
    {
      strcat(dname, "/");
      strcat(dname, name[i]);
    }
    strcpy(bname, name[i]);
  }

  // 2. dirname must exist and is a DIR
  int pino = getino(dname);
  MINODE *pmip = iget(dev, pino);
  if (!S_ISDIR(pmip->INODE.i_mode))
  {
    printf("%s is not a directory\n", dname);
    iput(pmip);
    return -1;
  }

  // 3. basename must not exist in parent DIR
  int temp = search(pmip, bname);
  if (temp != 0)
  {
    printf("%s already exists\n", bname);
    iput(pmip);
    return -1;
  }

  // 4. Call kmkdir to create a DIR or FILE
  my_mc(pmip, bname, pino, command);

  // 5. Increment parent inode's link count
  if (command == 1)
    pmip->INODE.i_links_count++;
  pmip->INODE.i_atime = time(NULL);
  pmip->dirty = 1;

  iput(pmip);
  return 0;
}

void rm_child(MINODE *pmip, char *bname)
{
  char buf[BLKSIZE], *cp, *pre, *nex;
  int found = 0, i = 0, k = 0, j = -2, location = 0, len = 0, total;
  DIR *remove = NULL, *prev = NULL, *next = NULL, *final = NULL;
  char namevar[256];

  // 1. Search parent INODE's data block(s) for the entry of name
  for (i = 0; i < 12; i++)
  {
    if(pmip->INODE.i_block[i] == 0) break;
    bzero(buf, BLKSIZE);
    get_block(pmip->dev, pmip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;
    while(location< BLKSIZE)
    {
      j++;
      strncpy(namevar,dp->name,dp->name_len);
      namevar[dp->name_len] = 0;
      if(strcmp(bname, namevar) == 0)
      {
        k = i;
        found = j;
        remove = dp;
        pre = cp;
      }
      location += dp->rec_len;
      if (!found) prev = dp;
      final = dp;
      cp += dp->rec_len;
      dp = (DIR *)cp;
      if (found && next == NULL)
      {
        next = dp;
        nex = cp;
      }
    }
    if (j > found) break;
  }

  // 2. Delete name entry from parent directory
  if (found == j && j == 1) // First and only entry
  {
    for (k += 1; k < 12; i++)
    {
      bdalloc(pmip->dev, pmip->INODE.i_block[k-1]);
      bzero(buf, BLKSIZE);
      put_block(pmip->dev, pmip->INODE.i_block[k-1], buf);
      pmip->INODE.i_block[k-1] = 0;
      if(pmip->INODE.i_block[k] == 0) break;
      get_block(pmip->dev, pmip->INODE.i_block[k], buf);
      put_block(pmip->dev, pmip->INODE.i_block[k-1], buf);      
    }
    pmip->INODE.i_blocks--;
    pmip->INODE.i_size -= BLKSIZE;
  }
  else if (found == j) // Last entry
  {
    prev->rec_len += remove->rec_len;
    put_block(pmip->dev, pmip->INODE.i_block[k], buf);
  }
  else // Middle or first but not only
  {
    final->rec_len += remove->rec_len;
    memcpy(pre, pre + remove->rec_len, buf + BLKSIZE - pre - remove->rec_len);
    put_block(pmip->dev, pmip->INODE.i_block[k], buf);
  }
}

int rmdir(char *pathname)
{
  char buf[BLKSIZE];
  bzero(buf, BLKSIZE);

  // 0. Check for invalid pathname
  if(!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, ".."))
  {
    printf("Cannot delete such directory.\n");
    return -1;
  }

  // 1. Get in-memory INODE of pathname
  int ino = getino(pathname);
  if(ino==-1)
  {
    printf("Error: ino number incorrect\n");
    return -1;
  }
  MINODE *mip = iget(dev, ino);

  // 2. Verify INODE is an empty DIR
  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("%s is not a directory\n", pathname);
    iput(mip);
    return -1;
  }
  else if (mip->refCount > 1)
  {
    printf("%s is busy\n", pathname);
    iput(mip);
    return -1;
  }
  else if (mip->INODE.i_links_count > 2)
  {
    printf("%s is not empty\n", pathname);
    iput(mip);
    return -1;
  }
  else
  {
    int i = 0;
    get_block(mip->dev, mip->INODE.i_block[0], buf);

    dp = (DIR *)buf;
    char *cp = buf;
    while(cp + dp->rec_len < buf + BLKSIZE)
    {
      i++;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    if (i > 2)
    {
      printf("%s is not empty\n", pathname);
      iput(mip);
      return -1;
    }
  }

  // 3. Get parent's ino and inode
  int pino = findino(mip, &ino);
  MINODE *pmip = iget(mip->dev, pino);

  // 4. Get name from parent's DIR's data block
  char pname[BLKSIZE];
  findmyname(pmip, ino, pname);

  // 5. Remove name from parent directory
  rm_child(pmip, pname);

  // 6. Decrement parent inode's link count
  pmip->INODE.i_links_count--;
  pmip->dirty = 1;
  iput(pmip);

  // 7. Deallocate its data blocks and inode
  bdalloc(mip->dev, mip->INODE.i_block[0]);
  idalloc(mip->dev, mip->ino);
  iput(mip);

  return 0;
}



/////////////////////////////

///link_unlink.c
int link(char *oldFileName, char *newFileName)
{

printf("inside link\n");
int oino, nino, getinoint;//old inode and new inode
MINODE *omip, *nmip;//old mip and new mip
char parent[256], child[256];
//set dev 
//might want to add if check for if path is absolute vs relative
 dev=running->cwd->dev;

//1
oino=getino(oldFileName);
if(oino==0)//No such component name
    {
    printf("Error with linking to file\n");
    return 0;
    }

omip=iget(dev,oino);

//must not be dir
if (S_ISDIR(omip->INODE.i_mode)) 
{
    printf("Error: Directory not allowed\n");
    return 0;
}

//(2). 
// new_file must not exist yet:
getinoint=getino(newFileName);
if(getinoint!=0)
{
    printf("Error: NewFile already exists\n");
    return 0;
}


//(3). creat new_file with the same inode number of old_file:
strcpy(parent,dirname(newFileName));
strcpy(child,basename(newFileName));
nino = getino(parent);
nmip = iget(dev, nino);
enter_name(nmip,oino, oino, child);

//(4).
//increment link count
omip->INODE.i_links_count++; // inc INODE’s links_count by 1
omip->dirty = 1; // for write back by iput(omip)
iput(omip);
iput(nmip);

return 1;
}

int unlink(char *path)
{
int ino;
MINODE *mip, *pmip;
dev=running->cwd->dev;
ino=getino(path);

if(ino==0)
{
    printf("Error with linking to file\n");
    return 0;
}
mip=iget(dev,ino);
if(S_ISDIR(mip->INODE.i_mode)) 
{
    printf("Error: Dir\n");
    return 0;
}
//check if dir
if(S_ISDIR(mip->INODE.i_mode)) 
{
    printf("Error: Directory not allowed\n");
    return 0;
}

/* 2).
// remove name entry from parent DIR’s data block:
parent = dirname(filename); child = basename(filename);
pino = getino(parent);
pimp = iget(dev, pino);
rm_child(pmip, ino, child);
pmip->dirty = 1;
iput(pmip);
*/
char parent[256], child[256];
int pino;
MINODE *pimp;

strcpy(parent, dirname(path));
strcpy(child, basename(path));

pino= getino(parent);

if(pino==0)
{
    printf("Error: cant get link\n");
    return 0;
}
pmip=iget(dev,pino);

rm_child(pmip,child);

pmip->dirty=1;
iput(pmip);

/*3).
// decrement INODE’s link_count by 1
mip->INODE.i_links_count--;
*/
mip->INODE.i_links_count--;

/*4).
if (mip->INODE.i_links_count > 0)
mip->dirty = 1; // for write INODE back to disk
*/
if(mip->INODE.i_links_count > 0)
{

    mip->dirty=1;
}
/*5).
else{ // if links_count = 0: remove filename
deallocate all data blocks in INODE;
deallocate INODE;
}
iput(mip); // release mip
*/
else
{

    truncate(mip);

    idalloc(mip->dev,mip->ino);

}
mip->dirty=1;

iput(mip);

return 1;

}

int symlink(char *old_file, char *new_file)
{
  int oino, nino;
 MINODE *mip;
dev = running->cwd->dev;


//(1). check: old_file must exist and new_file not yet exist;
oino=getino(old_file);
if(oino==0)
{
    printf("Error: old does not exist\n");
    return 0;
}
//(2). creat new_file; 
mkdir_creat(new_file,2);

nino=getino(new_file);
if(nino==0)
{
    printf("Error: New  exists\n");
    return 0;
}

mip=iget(dev,nino);


//mip->INODE.i_mode=0xA;
mip->INODE.i_mode=0xA1FF;//gives all correct permissions like solution has whereas KC's version above give impartial

/*
(3). // assume length of old_file name <= 60 chars
store old_file name in newfile’s INODE.i_block[ ] area.
set file size to length of old_file name
mark new_file’s minode dirty;
iput(new_file’s minode);
*/
mip->INODE.i_size=strlen(old_file);

strcpy((char *)mip->INODE.i_block,old_file);

mip->dirty=1;

iput(mip);

}

int readLink(char *file)
{
    /*
    (1). get file’s INODE in memory; verify it’s a LNK file
    (2). copy target filename from INODE.i_block[ ] into buffer;
    (3). return file size;
*/
   //1
  
   int ino;
   MINODE *mp;
   ino=getino(file);

  char buf[BLKSIZE];
  bzero(buf, BLKSIZE);

   if(ino==0)
   {
       printf("Error: No file\n");
       return 0;
   }
   mp=iget(dev,ino);
   //2
   
   strcpy(buf,mp->INODE.i_block);
   


//3
printf("readlink size: ");
printf("%d \n",(strlen(buf)));
//return strlen(buf);
}


int funchmod(char *pathname) //chmod oct filename: Change filename’s permission bits to octal value
{
    int ino;
    MINODE *mip;
    ino=getino(pathname);
    mip=iget(dev,ino);
    mip->INODE.i_mode |=0b111 ;//hardcoded could be 7 but hardcode makes rwx
    mip->dirty=1;
    iput(mip);

    return 0;
}

int utime(char *pathname)//changes time of inode to current time
{
    int ino;
    MINODE *mip;
    ino=getino(pathname);
    mip=iget(dev,ino);
    //update time
    mip->INODE.i_ctime=time(0L);
    mip->INODE.i_mtime=time(0L);
    mip->INODE.i_atime=time(0L);
/*
    char *timestring;
    //timestring=ctime(mip->INODE.i_atime);
    //strcpy(timestring, mip->INODE.i_atime);
    printf("access time: %s\n",mip->INODE.i_atime);
    
    //timestring=ctime(mip->INODE.i_ctime);
    printf("creation time: %s\n",timestring);
    //timestring=ctime(mip->INODE.i_mtime);
    
    printf("modify time: %s\n",timestring);
*/


    mip->dirty=1;
    iput(mip);

    return 0;
}
