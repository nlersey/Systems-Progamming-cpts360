#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h> 


char gpath[128];     // token strings
char *token[64];     // pointers to token strings
char dhome[300];     //stores home directory found from env
char tempdir[300];    //temporary for tokenizing to dir
int  n;              // number of token strings 

char *head, *tail;  //used for tokenizing pipe cmd

int pd[2];
int lpd[2];


int main(int argc, char *argv[], char *env[])
{
  char *dir[32];
  int ndir;
  
  char line[128];
  char *s;
  int i;
  
  int pid, status;

  char cmd[128];     // cmd to execve()

  //int oldFile=dup(1);//fix later
 
 
  // 1. find PATH, HOME from env[ ];
  // 2. decompose PATH into dir[0], dir[1],...., dir[n-1] with n dir pointers

  i=0;
  ndir=0;
 while(env[i])
  {
    printf("env:%d  %s\n",i,env[i]);
    char* name1="PATH=";
    char* name2="HOME=";
   if (strncmp(env[i], name1, 5)==0)
    {

     printf("Path: %s\n",env[i]);
      strcpy(tempdir,&env[i][5]);
      char *temppointer=strtok(tempdir,":");
      while(temppointer!=NULL)
      {
        dir[ndir]=temppointer;
        ndir++;
        temppointer=strtok(NULL,":");
      }
      break;


    }
    if (strncmp(env[i], name2, 5) == 0)
    {
     printf("Home: %s\n",env[i]);
      strcpy(dhome,&env[i][5]);
      
    }
  
i++;
  }

  // ASSUME: dir[0] = /bin; ndir=1

  dir[0] = "/bin";
  
   while(1){
     // get a command line of the form     cmd arg1 arg2, ... argn;
     printf("enter command Line : ");
     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;
     printf("line=%s\n", line);
     
     // tokenize line into token strings   cmd, arg1, arg2, ....
     printf("divide gpath[] into tokens\n");
     strcpy(gpath, line);
     n = 0;
     s = strtok(gpath, " ");
     while(s){
       token[n++] = s;
       s = strtok(NULL, " ");
     }
     token[n] = 0;
     //token[0] should be the cmd
     printf("tokens = ");
     for (i=0; i<n; i++){
       printf("%s   ", token[i]);
     }
     printf("\n");
    //end of tokenize block 
     
    //cd or exit
    //if (strcmp())
    if(strcmp(token[0],"cd")==0)
    {
      if(token[1]!=NULL)//chdir(arg1)
      {
        chdir(token[0]);
      }
      else//chdir($HOME)
      {
        chdir(dhome);
      }
      continue;
    }
    if(strcmp(token[0],"exit")==0)
    {
      exit(1);
    }

   

     // ASSUME token[0] not cd, exit
     pid = fork();
       if (pid){ // parent sh
           pid = wait(&status);
           printf("ZOMBIE child=%d status=%x\n", pid, status);
	   continue;
      }
      //do this block if fork() returns zero meaning we are in newly created child process
      //if fork() returns anything else but a zero, put parent to sleep and wait() for child process to die
      else
      {      // child sh
        printf("inside child\n");
    	  //do cmd processing;
  
        ///will need some sort of if check or soemthing here to check if simple command with I/O or pipes command
       //checking for pipe
       int boolcheck;
       boolcheck=pipecheck();
       if(boolcheck==1)//pipe stuff
       {
           pipe(pd);
           pid=fork();
           if(pid)//parent pipe
           {
               close(pd[0]);
               close(1);
               dup(pd[1]);
               close(pd[1]);
            exe(ndir, cmd, dir, env,head, 0);

           }
           else//child pipe
           {
               close(pd[1]); 
               close(0);
               dup(pd[0]); 
               close(pd[0]); 
               exe(ndir, cmd, dir, env,tail, 0);

           }




       }
       else if(boolcheck==0)//no pipe
       {
        exe(ndir, cmd, dir, env,token, 0);


       }

	   

	      printf("%s not found\n", cmd);
	      exit(1);
      }//else
    }//while(1)
}
int pipecheck()
{
for(int j=0;j<n;j++)  
{
    //if(strcmp(token[j],"|")==0)
    if(token[j]=='|')
	 {
	token[j]=0;
	tail =token +j + 1;
	head=token;
     return 1;
	 }

}
return 0;

}

void exe(int ndir, char *cmd, char* dir[], char* env[], char* tokenn[], int flag)
{
         //I/O redirection
         //code comes from book sections 3.9.3 & 3.9.4
        for(int j=0;j<n;j++)
          {
            //printf("\n inside i/o \n");
            if(strcmp(tokenn[j],">")==0)//send to outfile
            {
              printf("inside >\n");
              close(1);
              int fd = open(tokenn[j+1], O_WRONLY|O_CREAT, 0644);
              tokenn[j] = NULL;      
              break;
              //might need call to du() here
            }
           if(strcmp(token[j],">>")==0)//append to out
            {
              printf("inside >>\n");
             close(1);
              int fd = open(tokenn[j+1], O_WRONLY|O_CREAT|O_APPEND, 0644);
              tokenn[j] = NULL;
              //might need call to du() here

            }
           if(strcmp(tokenn[j],"<")==0)//take from infile
            {
              printf("inside <\n");
             close(0);
              int fd = open(tokenn[j+1], O_RDONLY);
              tokenn[j]=NULL;
              //might need call to du() or du2() here
            }
           // printf("end I/O reached\n");
            continue;
          } 
          //printf("ndir: %d\n",ndir);
          //for(int x=0;dir[x]!=NULL;x++)
          for(int x=0;x<ndir;x++)
          {
            //printf("\n test \n");
            strcpy(cmd, dir[x]);
            strcat(cmd, "/");
            strcat(cmd, tokenn[0]);
            printf("cmd=%s\n", cmd);
            int r = execve(cmd, tokenn, env);//replaces current child process with new process creating by running program
                  printf("execve %s failed r=%d\n", cmd, r);
            //dup2(oldFile,1);//fix later
          }
}