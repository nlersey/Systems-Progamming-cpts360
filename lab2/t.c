/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

//(Q1). Write C code to print values of argc and argv[] entries
    printf("Value of argc: %d\n",argc);
    for(int i=0;i<argc;i++)//iterate through equal number to argc
    {
        printf("Value of argv: %s",argv[i]);//print values of array argv

    }
    printf("\n");

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");

  // (Q2). Write C code to PRINT ADDRESS OF d, e, f
    printf("d: %p e: %p f: %p\n",&d, &e,&f);//print values

  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");

  // (Q3). Write C code to PRINT ADDRESS OF g,h,i
    printf("g: %p h: %p i: %p\n",&g, &h,&i);//print values

  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");

  // (Q4). Write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
    printf("u: %p v: %p w: %p i: %p p: %p\n",&u, &v,&w, &i, &p);

  u=10; v=11; w=12; i=13;

  FP = (int *)getebp();  // FP = stack frame pointer of the C() function


  // (Q5). Write C code to print the stack frame link list.

  p=FP;//set p to FP to be abe to parse link lsit
  int j=0;
  while(p!=NULL)//parse till end of list
  {
    printf("node: %d:  %x\n",j, p);
    p=*p;//go to next
    j++;
  }


// (Q6). Print the stack contents from p to the frame of main(). Include one stack entry per line when printing stack contents. Also, for each line include both address and contents (two columns)
//     YOU MAY JUST PRINT 128 entries of the stack contents.
  printf("Q6:\n");
  p = (int *) &p;//given by example code from hassan but idk what this does, i think it sets it to start of FP
  j=0;
  while(j<128)//prints 128 entries
  {
  printf("%x  %x\n",p,*p);
  p++;//iterate 
  j++;
  }




}