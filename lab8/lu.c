/** C4.5.c: Gauss Elimination with Partial Pivoting **/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define N 1024
double A[N][N];
double l[N][N];
double u[N][N];
int    p[N];   
double b[N]; 
double B[N];   
double y[N];   
double x[N];  

int print_matrix()
{
   int i, j;
   //printf("------------------------------------\n");
   for(i=0; i<N; i++){
       for(j=0;j<N+1;j++)
         // printf("%6.2f  ",  A[i][j]);
       printf("\n");
   }
}

void ludecomposition()
{
int i,j,k;
double max;
double temp1;

for(k=0;k<(N);k++)
{
    max=0;
    for(i=k;i<(N);i++)//partial pivot
    {
        if(max < fabs(A[i][k]))
        {
          max = fabs(A[i][k]);
          j = i; // pivot row number
        }
    }
    if(max==0)//exit because A matrix is singular
    {
        exit(1);
    }
    //swap p[k] and p[j]
    temp1=p[k];
    p[k]=p[j];
    p[j]=temp1;

    //row swap A
        //printf("swap row %d and row %d of A\n", k, j);

    for(int x=0;x<N;x++)
    {
        temp1=A[k][x];
        A[k][x]=A[j][x];
        A[j][x]=temp1;
    }
    
   // printf("\n A \n");
    for(int i=0;i<N;i++)
    {
        for(int j=0;j<N;j++)
        {
       // printf("%6.2f  ",  A[i][j]);
        }
                //printf("\n");

    }


    //swap l(k,0:k‐2) and l(j,0:k‐2)
    for (int x=0; x<k-2; x++)
    {
        temp1=l[k][x]; 
        l[k][x]=l[j][x]; 
        l[j][x]=temp1;
    }

    /*u(k,k) = a(k,k)                // update u matrix
        for i = k+1 to n‐1 do{
           l(i,k) = a(i,k)/u(k,k)
           u(k,i) = a(k,i)
        }*/
    
    u[k][k]=A[k][k];
    for(i=(k+1);i<(N);i++)
    {
        l[i][k]=A[i][k]/u[k][k];
        u[k][i]=A[k][i];

    }

    /*for i = k+1 to n‐1 do{         // row reduction
           for j = k+1 to n‐1 do{
             a(i,j) = a(i,j) ‐ l(i,k)*u(k,j)*/
       //printf("row reductions of A by row %d\n", k);
    for(i=(k+1);i<(N);i++)
    {
        for(j=k+1;j<N;j++)
        {
            A[i][j]=A[i][j]-l[i][k]*u[k][j];
        }
        

    }
    
   //printf("\n A:\n");
    for(int x=0;x<N;x++)
    {
        for(int j=0;j<N;j++){
       // printf("%6.2f  ",  A[x][j]);
        }
              //  printf("\n");

    }

    
   

  //  printf("\n L:\n");

    for(int x=0;x<N;x++)
    {
   
        for(int j=0;j<N;j++){
       // printf("%6.2f  ",  l[x][j]);
        }   
               // printf("\n");

    }
        //printf("\n U:\n");

    for(int x=0;x<N;x++)
    {
        for(int j=0;j<N;j++)
        {
       // printf("%6.2f  ",  u[x][j]);
        }
               // printf("\n");

    }
        //printf("\n P:\n");


    for(int x=0;x<N;x++)
    {
   // printf("%d  ",  p[x]);
    }



}


}

int main(int argc, char *argv[])
{
  int i, j;
  double sum;

    printf("main: initialize matrix A[N][N+1] as [A|B]\n"); 
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
        A[i][j] = 1.0;
  for (i=0; i<N; i++)
      A[i][N-i-1] = 1.0*N;
  for (i=0; i<N; i++)
  {
    //A[i][N] = (N*(N+1))/2 + (N-i)*(N - 1);///delete?
    B[i] = (N*(N+1))/2 + (N-i)*(N - 1);
  }
    print_matrix();  // show initial matrix [A|B]

     for (i=0; i<N; i++)
   {
       p[i]=i;
       for(j=0;j<N;j++)
       {
       u[i][j]=0.0;
       l[i][j]=0.0;

       } 
  }
  //make diagonal line for l
  for (i=0;i<N;i++)
  {
      for (j=0;j<N;j++)
      {
          if(j==i)
          {
             l[i][j]=1.0;
          }
      }
  }


  
    printf("A:\n");
    for(int x=0;x<N;x++)
    {
        for(int j=0;j<N;j++){
        printf("%6.2f  ",  A[x][j]);
        }
        printf("\n");
    }

   

    printf("\n L:\n");

    for(int x=0;x<N;x++)
    {
   
        for(int j=0;j<N;j++){
        printf("%6.2f  ",  l[x][j]);
        }   
                printf("\n");

    }
        printf("\n U:\n");

    for(int x=0;x<N;x++)
    {
        for(int j=0;j<N;j++)
        {
        printf("%6.2f  ",  u[x][j]);
        }
                printf("\n");

    }
        printf("\n P:\n");


    for(int x=0;x<N;x++)
    {
    printf("%d  ",  p[x]);
    }

//start clock
clock_t t; //got this code from geeksfor geeks
t = clock(); 

 ludecomposition();
 
//end clock
 t = clock() - t; 
double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 


for (i=0; i<N; i++){
    b[i] = B[ p[i] ];
  }
//code from KC's book
 for(i=0;i<N;i++)  //forward substitution
 {
     y[i]=b[i];
     for(j=0;j<i;j++){
     y[i]=y[i]-(l[i][j]*y[j]);
     }
    
 }


printf("\n Y \n");
 for(int i=0;i<N;i++)
    {
    printf("%6.2f  ",  y[i]);
    }
 
//backward sub //code from KC's book
for (i=(N-1);i>=0;i--)
{
    sum=0;
    for(j=i+1;j<N;j++)
    {
        sum+=u[i][j]*x[j];
    }
    x[i]=(y[i] - sum) / u[i][i];
}

 printf("\n x:\n");

    for(int i=0;i<N;i++)
    {
    printf("%6.2f  ",  x[i]);
    }

  //print clock time

 printf("\n ludecomposition() took %f seconds to execute \n", time_taken); 

  // print solution
  /*printf("\n The solution is :\n");
  for(i=0; i<N; i++){
	printf("%6.2f  ", A[i][N]);
  }
  printf("\n");*/



}