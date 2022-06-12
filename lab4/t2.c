/* sample code for Task 2 of Lab4 */      
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
	u8 drive;             // drive number FD=0, HD=0x80, etc.
	u8  head;             // starting head 
	u8  sector;           // starting sector
	u8  cylinder;         // starting cylinder
	u8  sys_type;         // partition type: NTFS, LINUX, etc.
	u8  end_head;         // end head 
	u8  end_sector;       // end sector
	u8  end_cylinder;     // end cylinder
	u32 start_sector;     // starting sector counting from 0 
	u32 nr_sectors;       // number of of sectors in partition
};

char *dev = "vdisk";
int fd;
    
// read a disk sector into char buf[512]
int read_sector(int fd, int sector, char *buf)
{
    lseek(fd, sector*512, SEEK_SET);  // lssek to byte sector*512
    read(fd, buf, 512);               // read 512 bytes into buf[ ]
}

int main()
{
  struct partition *p;
  char buf[512];

  fd = open(dev, O_RDONLY);   // open dev for READ
  read_sector(fd, 0, buf);    // call read_sector()    

  p = (struct partition *)(&buf[0x1be]); // p->P1
  
  // print P1's start_sector, nr_sectors, sys_type;
  //printf("Device    Start   Sectors   systype\n");
  // Write code to print all 4 partitions;
  for (int i=1; i<5; i++)
  {
    printf("Device:%d      Start:%d      Sectors:%d      systype:%x\n",i,p->start_sector,p->nr_sectors,p->sys_type);//prints out all values
    if(p->sys_type!=5)//at p4 check to make sure p++ doesn't update to garbage value 
    {
      p++;
    }
  }

  int p4start_sector_tracker=0;
  // ASSUME P4 is EXTEND type, p->P4; 
  printf("P4 start_sector = %d\n", p->start_sector);
  p4start_sector_tracker=p->start_sector; 
  read_sector(fd, p->start_sector, buf);

  p = (struct partition *)&buf[0x1BE];    // p->localMBR
  printf("FIRST entry of localMBR\n");
  printf("start_sector=%d, nsectors=%d\n", p->start_sector, p->nr_sectors);

  printf("Start:%d      Sectors:%d      systype:%x\n",p->start_sector + p4start_sector_tracker,p->nr_sectors,p->sys_type);//prints out p5 values


  
  // Write YOUR code to get 2nd entry, which points to the next localMBR, etc.
  // NOTE: all sector numbers are relative to P4's start_sector
 
  int x=18 + p4start_sector_tracker;//keeps track of start values since they update to 1 each time we jump to new ptr

  for(int i=0;i<2;i++)//hard coded for p6 and p7
  {
    x=(p->nr_sectors + 18 + x);//update start value tracker

    p++;//update p to get ready to read next parititon
    read_sector(fd, (p->start_sector+p4start_sector_tracker), buf);//transition to next parition 
    p = (struct partition *)&buf[0x1BE];    // p->localMBR
    printf("Start:%d      Sectors:%d      systype:%x\n", (x),p->nr_sectors,p->sys_type);//print all values


  }
  

}
