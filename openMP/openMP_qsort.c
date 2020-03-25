/**	
	Author: Chares Moustakas
	AEM   : 8860
	Prof  : Nikolaos Pitsianis,Dimitrios Floros
	email : <charesmoustakas@gmail.com> <cmoustakas@ece.auth.gr>
**/


#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include<omp.h>
#include<math.h>

#define MAX_NUMofTHREADS 256

/* local function declarations */
int  test( int *a, int n);
void init( int *a, int n);
void print(int *a, int n);
void swap(int *v, int k, int l);
int partition(int *v, int n);
void qsort_OMP(int *v, int n);

/* --- Entry POINT --- */
int main(int argc, char **argv)
{

    /* parse input */
    if (argc != 3)
    {
        printf("Usage: %s q\n  where n=2^q is problem size (power of two)\n p: where th = 2^p is number of worker threads\n",
               argv[0]);
        exit(1);
    }


    /* initiate vector of random integerts */
    int n  = 1<<atoi(argv[1]);
    int *a = (int *) malloc(n * sizeof(int));
    int numberOfThreads = pow(2,atoi(argv[2]));

    if(numberOfThreads > MAX_NUMofTHREADS)
    {
        fprintf(stderr," [-][-] Usage: Number of threads can't be greater than 2^8 = 256. \n Exitting .. \n\n");
        exit(1);
    }
    /* initialize vector */
    init(a, n);

    omp_set_num_threads(numberOfThreads);


    /* sort elements in original order */

    struct timeval startwtime, endwtime;
    double total_time;

    gettimeofday(&startwtime, NULL);
    qsort_OMP(a, n);
    gettimeofday(&endwtime, NULL);

    total_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
                          + endwtime.tv_sec - startwtime.tv_sec);

    /* validate result */
    int pass = test(a, n);
    printf(" TEST %s\n",(pass) ? "PASSed" : "FAILed");
    assert( pass != 0 );

    /* print sorted vector */
    print(a, n);

    /* print execution time */
    printf("%f\n", total_time);




    /* exit */
    return 0;

}

/** -------------- SUB-PROCEDURES  ----------------- **/

/** procedure test() : verify sort results **/
int test(int *a, int n)
{

    // TODO: implement
    int pass = 0;
    for(int counter = 0; counter<n; counter++)
    {
        //printf("%i \n",a[counter]);
        if(a[counter]<a[counter-1])
        {
            pass = 0;
            break;
        }
        if(counter==n-1)
        {
            pass = 1;
        }
    }

    return pass;

}

/** procedure init() : initialize array "a" with data **/
void init(int *a, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        a[i] = rand() % n; // (N - i);
    }
}

/** procedure  print() : print array elements **/
void print(int *a, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");
}



/* swap -- swap elements k and l of vector v */
void swap(int *v, int k, int l)
{
    int temp = v[k];
    v[k] = v[l];
    v[l] = temp;
}


/* partition -- in-place update of elements */
int partition(int *v, int n)
{
    int pivot = v[n-1];
    int i = 0;


    for (int j = 0; j < n - 1; j++)
        if (v[j] < pivot)
            swap(v,i++,j);

    swap(v, i, n - 1);
    return (i);
}

/* qsortseq -- Entry point for QuickSort */
void qsort_OMP(int *v, int n)
{

    if (n > 1)
    {
        int p = partition(v, n);
        #pragma omp parallel sections
        {

            #pragma omp section
            {qsort_OMP(v,p);}
            #pragma omp section
            {qsort_OMP(&v[p+1],n-p-1);}

        }
    }
}

