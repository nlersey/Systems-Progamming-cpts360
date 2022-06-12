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

#define MAX   256
#define BLK  1024

int server_sock, client_sock;      // file descriptors for sockets
char *serverIP = "127.0.0.1";      // hardcoded server IP address
int serverPORT = 1234;             // hardcoded server port number

struct sockaddr_in saddr, caddr;   // socket addr structs

char command[16];
char *pathname[MAX];
//globals that were in main for simplicity sake
char line[MAX], ans[MAX];
int n;

char *cmd[]={"get","put","ls","cd","pwd","mkdir","rmdir","rm"};

//fun prototypes
int init();
int findcmd(char *command);
int sget();
int sput();
int sls();
int ls_file(char *file);
int scd(char *path);
int spwd();
int smkdir(char *path);
int srmdir(char *path);
int srm(char *path);


int init()
{
    printf("1. create a socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    
    printf("3. bind socket to server\n");
    if ((bind(server_sock, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
    printf("4. server listen with queue size = 5\n");
    if ((listen(server_sock, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    printf("5. server at IP=%s port=%d\n", serverIP, serverPORT);
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
  
int main() 
{
    int n, length,x;
    char line[MAX];
    //set virtual root to current working dir
    chdir("./");
    chroot("./");

    int index;


    init();  

    while(1){
       printf("server: try to accept a new connection\n");
       length = sizeof(caddr);
       client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
       if (client_sock < 0){
          printf("server: accept error\n");
          exit(1);
       }
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         n = read(client_sock, line, MAX);
         if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
         }
         line[n]=0;
         printf("server: read  n=%d bytes; line=[%s]\n", n, line);

         ///
         *pathname=NULL;
      sscanf(line,"%s %s",command,pathname);
      index=findcmd(command);
      
      switch(index)
      {
          case 0:sget();

            break;
          case 1:sput();
            break;
          case 2:
          //check if ls for cwd or not
          if(strcmp(pathname,"")==0)
          {
              //printf("cwd option\n");
              char buf[MAX];
              getcwd(buf,MAX-1);
              //*pathname=buf;
              strcpy(pathname,buf);
              
              int x;
              x=sls();
              //int x=sls(buf);

              if(x=1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);

            }
          }
          else
          {
          //int x;
         // x=sls(pathname);
          if(x=1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }

          }
            break;
          case 3:
            //int x;
            x=scd(pathname);
            if(x=-1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }
            break;
          case 4:spwd();

            break;
          case 5:
          //int x;
          x=smkdir(pathname);
          if(x=-1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }
            break;
          case 6:
          //int x;
          x=rmdir(pathname);
           if(x=-1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }
            break;
          case 7:
         // int x;
          x=srm(pathname);
           if(x=-1)
            {
                strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            }
            else
            {
                //strcat(line, "Error\n");
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);


            }
            break;
      }

         ///

        /*
         strcat(line, " ECHO");
         // send the echo line to client 
         n = write(client_sock, line, MAX);

         printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
         */

       }
    }
}

int sget()
{

}
int sput()
{

}
int sls()/// check
{
   
struct stat buf;
//struct stat mystat, *sp=&mystat;
DIR *p_dir;
char *filename;
printf("inside ls\n");

p_dir=opendir(pathname);
printf("after opendir ls\n");

if(p_dir==NULL)
{
    printf("no such file or pathname\n");
    return(1);
}
printf("after p_dir==null ls\n");

//struct dirent *file=readdir(p_dir);

/*
if(path==NULL)
{
 printf("no such file or pathname\n");
    exit(1);///might need to change to return 1
}
*/
char pathcopy[MAX*2];
struct dirent *file;
memset(pathcopy, '\0', sizeof(pathcopy));///

printf("after pathcopy\n");

while(p_dir!=NULL)
{
    printf("after while ls\n");

if(file=readdir(p_dir)!=NULL)
{
    printf("after if readdir ls\n");
     memset(pathcopy, '\0', sizeof(pathcopy));///

    strcpy(pathcopy,pathname);
        printf("1\n");

    strcat(pathcopy,"/");
            printf("2\n");

    strcat(pathcopy,file->d_name);//segementation fault
    printf("before lsfile  ls\n");

    ls_file(pathcopy);

}

}

return 0;




/*
 struct dirent *dp;
    DIR *mydir;
    char fullPath[MAX * 2];
    memset(fullPath, '\0', sizeof(fullPath));

    if ((mydir = opendir(pathname)) == NULL)
    {
        perror("couldn't open pathname");
        return 1;
    }

    do
    {
        if ((dp = readdir(mydir)) != NULL)
        {
           // printf("after if ls\n");
            memset(fullPath, '\0', sizeof(fullPath));
            strcpy(fullPath, pathname);
            strcat(fullPath, "/");
            strcat(fullPath, dp->d_name);
          //  printf("before lsfile \n");
            ls_file(fullPath);
        }

    } while (dp != NULL);

    closedir(mydir);
    return 0;
*/




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

int scd(char *path)
{
int r=chdir(path);
return r;
}
int spwd()
{
char buf[MAX];
getcwd(buf,MAX);
printf("CWD: %s\n",buf);
write(client_sock,buf,MAX);

}
int smkdir(char * path)
{
int r=mkdir(path,0755);
return r;
}
int srmdir(char *path)
{
int r=rmdir(path);
return r;
}
int srm(char *path)
{
int r=unlink(path);
return r;
}