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
#include<pthread.h>
#include<unistd.h>
#include<math.h>

#define MAX_NUMofTHREADS  256


typedef struct pthread_arguments
{
    int *vector;
    int peak,bottom,vector_len,thread_counter;
} p_args ;

int thread_limit ;
int thread_host = 1 ;

pthread_t thread[MAX_NUMofTHREADS];
pthread_mutex_t lock ;


/* local function declarations */
int  test( struct pthread_arguments *thread_args);
void init( int *a, int n);
void print(int *v,int len);
int partition(int *v,int n);
void swap(int *v,int i,int j  );
void qsort_seq(int *v, int n);

//My functions :
void *pthread_sortFunc( void *thread_args);
void fill_Matrix(int *wM,int* iM,int bottom,int wM_len);



int main(int argc, char **argv)
{

    /* parse input*/
    /**if (argc != 3)
    {
        printf("Usage: %s q  p \n  where n=2^q is problem size (power of two)\n And thread_limit = 2^p \n",argv[0]);
        exit(1);
    }**/

    struct timeval startwtime, endwtime;
    double total_time;


    /* initiate vector of random integerts */
    /**int n  = 1<<atoi(argv[1]);
    thread_limit = pow(2,atoi(argv[2]));**/

    int p = 1;
    while(p<9)
    {
        int thread_limit = pow(2,p);
        int q = 16;
                while(q<25)
        {
            int n = pow(2,q);

            if(thread_limit > MAX_NUMofTHREADS)
            {
                fprintf(stderr," [-][-] Usage: Number of threads can't be greater than 2^8 = 256. \n Exitting .. \n\n");
                exit(1);
            }

            int *a = (int *) malloc(n * sizeof(int));


            /* initialize vector */
            init(a, n);

            p_args *thread_args = malloc(sizeof(struct pthread_arguments)) ;

            thread_args->vector = malloc(n*sizeof(int));
            thread_args->vector = a;
            thread_args->vector_len = n ;
            thread_args->thread_counter = thread_host;
            thread_args->bottom = 0;
            thread_args->peak = n-1;

            /* sort elements in original order */

            gettimeofday(&startwtime, NULL);
            pthread_create(&thread[0],NULL,&pthread_sortFunc,(void*)thread_args);
            pthread_join(thread[0],NULL);
            gettimeofday(&endwtime, NULL);

            total_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
                                  + endwtime.tv_sec - startwtime.tv_sec);
            /* validate result */

            /**int pass = test(thread_args);
            printf(" TEST %s\n",(pass) ? "PASSed" : "FAILed");
            assert( pass != 0 );

            /* print sorted vector */
            //print(thread_args->vector,n);

            /* print execution time */

            printf("%f \n", total_time);
            q++;
        }
        p++;
    }
    /* exit */
    return 0;

}

/** -------------- SUB-PROCEDURES  ----------------- **/

/** procedure test() : verify sort results **/
int test(struct pthread_arguments *t_args)
{

    // TODO: implement
    int pass = 0;
    for(int counter = 0; counter< t_args->vector_len; counter++)
    {
        //printf("%i \n",a[counter]);
        if(t_args->vector[counter]< t_args->vector[counter-1])
        {
            pass = 0;
            break;
        }
        if(counter== t_args->vector_len-1)
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
void print(int *v,int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%d ", v[i]);
    }
    printf("\n");
}



/** PTHREAD FUNCTION USING SWAP AND PARTITION parallel edition : **/


/* swap -- swap elements k and l of vector v */
void swap(int *v,int i,int j )
{
    int temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}


/* partition -- in-place update of elements */

int partition(int *v,int n )
{
    int pivot = v[n-1];
    int i = 0;
    for (int j = 0; j < n - 1  ; j++)
        if (v[j] < pivot)
        {
            swap(v,i++,j);
        }
    swap(v,i,n-1);
    return (i);
}


void qsort_seq(int *v, int n)
{

    if (n > 1)
    {
        int p = partition(v, n);
        qsort_seq(v,p);
        qsort_seq(&v[p+1],n-p-1);
    }
}


/* qsort parall. -- Entry point for QuickSort */

void* pthread_sortFunc(void *thread_args)
{
    if (  ((p_args*)thread_args)->vector_len > 1  )
    {



        pthread_mutex_lock(&lock);
        int p = partition( ((p_args*)thread_args)->vector, ((p_args*)thread_args)->vector_len);
        pthread_mutex_unlock(&lock);


        int hold_len = ((p_args*)thread_args)->vector_len;
        int hold_bottom = 0;
        int hold_peak = ((p_args*)thread_args)->peak ;



        /** WORKER1 & WORKER2 STRUCT :**/
        p_args *w1  = malloc(sizeof(p_args)),*w2  = malloc(sizeof(p_args));


        pthread_mutex_lock(&lock);
        thread_host ++ ;
        w1->thread_counter = thread_host;
        thread_host ++ ;
        w2->thread_counter = w1->thread_counter++;
        pthread_mutex_unlock(&lock);


        if(w1->thread_counter> thread_limit || w2->thread_counter > thread_limit)
        {
            //printf("[-][-][-] OUT OF THREAD BOUNDS : sequential execution  ===> \n");
            qsort_seq(((p_args*)thread_args)->vector,((p_args*)thread_args)->vector_len);

        }

        else
        {

            //WORKER 1 ATTRIBUTES INIT. :
            w1->bottom = hold_bottom  ;
            w1->peak = p - 1;
            w1->vector_len = w1->peak - w1->bottom + 1 ;
            w1->vector = malloc(w1->vector_len*sizeof(int));


            //WORKER 2 ATTRIBUTES INIT. :
            w2->bottom = p + 1 ;
            w2->peak =  ((p_args*)thread_args)->vector_len-1 ;
            w2->vector_len = w2->peak - w2->bottom + 1 ;
            w2->vector = malloc(w2->vector_len*sizeof(int));


            // FILL WORKER 1 :
            for(int counter1 = 0; counter1 < w1->vector_len; counter1++ )
            {
                w1->vector[counter1] = ((p_args*)thread_args)->vector[counter1 + w1->bottom];
            }

            //FIL WORKER 2 :
            for(int counter2 = 0; counter2 < w2->vector_len; counter2++)
            {
                w2->vector[counter2] =  ((p_args*)thread_args)->vector[counter2 + w2->bottom]; //
            }



            /** IF SOMETHING GOES WRONG IN CREATION OR JOINING :**/

            /**----------------------------------------------------**/
            if(pthread_create(&thread[w1->thread_counter],NULL,&pthread_sortFunc,(void*)w1))
            {
                fprintf(stderr,"ERROR CREATING THREAD \n");
                return NULL ;
            }
            if(pthread_join(thread[w1->thread_counter],NULL))
            {
                fprintf(stderr,"ERROR JOINING  THREAD \n");
                return NULL ;
            }


            if(pthread_create(&thread[w2->thread_counter],NULL,&pthread_sortFunc,(void*)w2))
            {
                fprintf(stderr,"ERROR CREATING THREAD");
                return NULL ;
            }
            if(pthread_join(thread[w2->thread_counter],NULL))
            {
                fprintf(stderr,"ERROR CREATING THREAD");
                return NULL ;
            }
            /**----------------------------------------------------**/


            //REFILL THE MATRIX :
            fill_Matrix(w1->vector,((p_args*)thread_args)->vector,w1->bottom,w1->vector_len);
            fill_Matrix(w2->vector,((p_args*)thread_args)->vector,w2->bottom,w2->vector_len);

        }
    }

}


// wM : worker Matrix
//iM : initial Matrix

void fill_Matrix(int *wM,int *iM,int bottom,int wM_len)
{
    for(int counter = 0; counter < wM_len; counter++)
    {
        iM[counter+bottom] = wM[counter];
    }
}



