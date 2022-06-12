#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

char command[16];
char *pathname[MAX];

//globals that were in main for simplicity sake
char line[MAX], ans[MAX];
int n;

char *cmd[]={"get","put","ls","cd","pwd","mkdir","rmdir","rm","lcat","lls","lcd","lpwd",
"lmkdir","lrmdir","lrm"};

//fun prototypes
int init();
int findcmd(char *command);

///still to be implemented
int funget();
int funput();
int funls();
int funcd();
int funpwd();
int funmkdir();
int funrmdir();
int funrm();
int funlcat();
int funlls(char *path);
int ls_file(char *file);
int funlcd(char *file);
int funlpwd();
int funlmkdir(char *file);
int funlrmdir();
int funlrm(char *path);

int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
}
  
int findcmd(char *command)
{
    int i = 0;
	while (cmd[i]) {
		if (strcmp(command, cmd[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

int main(int argc, char *argv[], char *env[]) 
{ 
    //int  n;
    int index,x;
    //char line[MAX], ans[MAX];

    init();
  
    while (1){
      printf("input command:get|put|ls|cd|pwd|mkdir|rmdir|rm||lcat|lls|lcd|lpwd|lmkdir|lmrmdir|lrm ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill \n at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
      ///
      *pathname=NULL;
      sscanf(line,"%s %s",command,pathname);
      index=findcmd(command);
      printf("line in main: %s\n",line);
      switch(index)
      {
          case 0:funget();
            break;
          case 1:funput();
            break;
          case 2:funls();
            break;
          case 3:funcd();
            break;
          case 4:funpwd();
            break;
          case 5:funmkdir();
            break;
          case 6:funrmdir();
            break;
          case 7:funrm();
            break;
          case 8:funlcat(pathname);
            break;
          case 9:
           //check if ls for cwd or not
          if(strcmp(pathname,"")==0)
          {
              //printf("cwd option\n");
              //char buf[MAX];
             // getcwd(buf,MAX-1);
              //int x=sls(buf);
              x=funlls("./");
              if(x=1)
            {
               // strcat(line, "Error\n");
              //  n = write(client_sock, line, MAX);
               // printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
               // n = write(client_sock, line, MAX);
                //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);

            }
          }
          else
          {
          
          x=funlls(pathname);
          if(x=-1)
            {
               // strcat(line, "Error\n");
               // n = write(client_sock, line, MAX);
               // printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
               // n = write(client_sock, line, MAX);
                //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }

          }


            break;
          case 10:
          
          x=funlcd(pathname);
            break;
          case 11:funlpwd();
            break;
          case 12:
          
          x=funlmkdir(pathname);
            break;
          case 13:
          
          x=funlrmdir(pathname);
            break;
          case 14:
          
          x=funlrm(pathname);
            break;

      }

      ///


      /*

      // Send ENTIRE line to server
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      

      // Read a line from sock and show it
      bzero(ans, MAX);
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
      */
    }
}

int funget()
{

}

int funput()
{

}

int funls()
{
//send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);

}

int funcd()
{
//send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      

    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);

}

int funpwd()
{
//send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);


    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);

}

int funmkdir()
{
    //send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      

    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);

}

int funrmdir()
{
  //send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      

    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
}

int funrm()
{
//send whole line to server to tokenize there
    n = write(sock,line,MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      

    // Read a line from sock and show it
    bzero(ans, MAX);
     n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);

}

int funlcat(char *path)///
{

int i=0;
char buf[1000];
FILE *fd=open(path, O_RDONLY);
if(fd!=NULL)
{
while (fgets(buf, 512, fd) != NULL)
{

if(buf[i]=='\n')
{
putchar('\n');
i++;
}
else
puts(buf);

i++;
}

}
else
{
  printf("lcat open failed\n");
  return 1;
}
return 0;


}

int funlls(char *path)
{
 struct dirent *pdir;
DIR *dir;
if(path==NULL)
{
 printf("no such file or pathname\n");
    return(1);
}
dir=opendir(path);
pdir = readdir(dir);
char pathcopy[MAX];

while(pdir!=NULL)
{
if((pdir = readdir(dir)) != NULL)
        {
           
            strcpy(pathcopy, path);
            strcat(pathcopy, "/");
            strcat(pathcopy, pdir->d_name);
            ls_file(pathcopy);
        }

}

return 0;

}
int ls_file(char *file)//pretty much verbtim copied from KC's book
{
struct stat fstat, *sp;
int r, i;
char ftime[64];
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";
char linkname[MAX];
printf("inside lsfile\n");


sp=&fstat;
if((r=lstat(file,&fstat))<0)
{
    printf("can't stat %s\n",file);
    exit(1);
}
if((sp->st_mode & 0xF000)==0x8000)// if (S_ISREG())
{
printf("%c",'-');

}
if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
{
    printf("%c",'d');
}
if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
{
    printf("%c",'l');
}
for(i=8;i>=0;i--)
{
    if (sp->st_mode & (1 << i)) // print r|w|x
    printf("%c", t1[i]);
    else
    printf("%c", t2[i]); // or print -

}
printf("%4d ",sp->st_nlink); // link count
printf("%4d ",sp->st_gid); // gid
printf("%4d ",sp->st_uid); // uid
printf("%8d ",sp->st_size); // file size

// print time
strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
ftime[strlen(ftime)-1] = 0; // kill \n at end
printf("%s ",ftime);

// print name
printf("%s", basename(file)); // print file basename
// print -> linkname if symbolic file
if ((sp->st_mode & 0xF000)== 0xA000){
// use readlink() to read linkname
printf(" -> %s", linkname); // print linked name
}

printf("\n");

}

int funlcd(char *path)
{
int r=chdir(path);
return r;
}

int funlpwd()
{
char buf[MAX];
getcwd(buf,MAX);
printf("CWD: %s\n",buf);
//write(client_sock,buf,MAX);
}

int funlmkdir(char *path)
{
int r=mkdir(path,0755);
return r;
}

int funlrmdir(char *path)
{
int r=rmdir(path);
return r;
}

int funlrm(char *path)
{
int r=unlink(path);
return r;
}
