#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <pthread.h>
#include "etime.h"
//compile with gcc -g -Wall -O2 -o kmeans kmeans.c etime.c -lm -lpthread -std=c99
pthread_mutex_t mutex; //initializes mutex

void* CheckAndCount(void* rank); // Thread function

//Declare our global variables;
//For arrays pointers are used (i.e. dynamic memory allocation)
int K;          //Number of clusters
int d;          //Number of dimensions or "features"
int N;          //Number of samples
int P;          //Number of threads to use
int *C;         //Array which maps each sample to its nearest cluster
float **A;      //Array which stores values for clusters
float **B;      //Array which stores values for samples
int *E;         //Counter array which counts how many samples are associated with a given cluster
float **D;      //Array which adds values of points nearest each cluster; used for reassigning clusters

//Function to seed random values with current time 
void seed()
{
        time_t timeValue;
        srand((unsigned)time(&timeValue));
}                             

//Function to generate random sequence of integers;
//necessary for generating initial cluster locations
//This generates an array of "N" integers, shuffles it,
//and chooses the first "K" integers of the array
void generateSetOfNumbers(int arr[], int n, int k)
{
        seed(); //Seed function defined earlier
        int p[n];

        for (int i=0; i<n; ++i)
        {
        p[i] = i;
        }
  
        //Shuffle p
        for (int i=n; i>0; --i)
        {
                //Get swap index
                int j = rand()%i;
                //Swap p[i] with p[j]
                int temp = p[i];
                p[i] = p[j];
                p[j] = temp;
        }
        //Copy first k elements from p to arr
        for (int i=0; i<k; ++i)
        {
                arr[i] = p[i];
        }
}

        


int main(int argc, char* argv[])
{
        K = strtol(argv[1], NULL, 10); //Read in number of clusters as first argument
        P = strtol(argv[2], NULL, 10); //Read in number of threads as second argument

        FILE *myFile;                  
        myFile = fopen(argv[3], "r");  //Define file to open in third argument; set to read
        fscanf(myFile, "%d", &N);      //Scan in first value as number of samples "N"
        fscanf(myFile, "%d", &d);      //Scan in second value as number of dimensions "d"
       
        //Dynamically allocate memory for all of our global arrays; according to their dimensions and type     
        C = malloc(N*sizeof(int));

        //Multidimensional arrays need a loop as seen here
        A = malloc(K*sizeof(float*));
        for (int i = 0; i < K; ++i)
        {
                A[i] = malloc(d*sizeof(float));
        }        

        D = malloc(K*sizeof(float*));
        for (int i = 0; i < K; ++i)
        {
                D[i] = malloc(d*sizeof(float));
        }        

        E = malloc(K*sizeof(int));

        B = malloc(N*sizeof(float*));
        for (int i = 0; i < N; ++i)
        {
                B[i] = malloc(d*sizeof(float));
        }        

        float F[K][d]; //Array to hold the previous iteration's cluster locations
                       //Used for checking if they have changed since last loop; if so, break the loop

        //This loop scans in values for samples from input file
        //Note that this assumes samples begin after sample # and dimensions (which is true in this lab)
        for (int i=0; i < N; ++i)
        {
                for (int j=0; j< d; ++j)
                {
                        fscanf(myFile, "%f", &B[i][j]);
                }
        }

        //This defines a K-dimensional array and stores K random integers chosen from N values, using function defined above
        int randoms[K];
        generateSetOfNumbers(randoms,N,K);

        //Assigns each cluster to a unique random sample using the array of random integers we just generated
        for (int i=0; i < K; ++i)
        {
                for (int j=0; j< d; ++j)
                {
                        A[i][j] = B[randoms[i]][j];
                }
        }

        //"Checker" array F is assigned values from cluster positions
        for (int i=0; i < K; ++i)
        {
                for (int j=0; j< d; ++j)
                {
                        F[i][j] = A[i][j];
                }
        }

tic(); //Since multiple thread creations/joinings, we'll start timing at beginning of this loop
        for (int a=0; a < 100; ++a) //If loop isn't broken, set to run for 100 iterations 
        {
                //"D" holds values for reassigning cluster positions based on nearest samples
                //Needs to be zeroed each iteration, so done at beginning of loop
                for (int i=0; i < K; ++i)
                {
                        for (int j=0; j < d; ++j)
                        {
                                D[i][j]=0;
                        }
                }

                //"E" is a counter for how many samples are associated with each cluster
                //Used when reassigning cluster positions ("D" will be divided by "E")
                //Needs to be zeroed each iteration, same reason as "D"                
                for (int i=0; i < K; ++i)
                {
                        E[i]=0;
                }

                 
                long thread;               //Variable to loop thread creation and joining over
                pthread_t* thread_handles; //Object uniquely identifiable to each of the threads; Used for creation and joining
                
                thread_handles = malloc(P*sizeof(pthread_t)); //Allocting memory for P thread handles

                pthread_mutex_init(&mutex,NULL);   //Preparing mutex

                 //Create threads and run CheckAndCount Function             
                 //As we'll see later, thread function calculates distance from clusters for all samples, 
                 //finds the closest cluster to each sample, and then adds that samples values to matrix D
                 //for reassigning cluster values later
                for (thread = 0; thread < P; thread++)
                {
                        pthread_create(&thread_handles[thread], NULL, CheckAndCount, (void*)thread);
                }

                //Join threads; waits until thread is done
                for (thread = 0; thread < P; thread++)
                {
                        pthread_join(thread_handles[thread], NULL);
                }
       
                pthread_mutex_destroy(&mutex); //Mutex locks no longer needed
                free(thread_handles);   //Free allocated memory to thread pointers to avoid memory leak

                //Reassign new values to clusters. D is sum of all samples associated with given cluster
                //and E is counter for how many samples for each cluster
                for (int i=0; i < K; ++i)
                {
                        if (E[i] > 0)  //Only reassign if at least one sample is closest to a cluster; avoids dividing by zero i.e. segfault
                        {
                                for (int j=0; j < d; ++j)
                                {
                                        A[i][j] = D[i][j]/(float)E[i] ; //Need to typecast int array E as float for this
                                }
                        }
                }

                //Now check if clusters have moved by adding differences between last clusters
                //and current ones; actually adding distances squared so negative numbers aren't a problem
                float brcounter = 0.0; //initialize brcounter (break counter) to zero; need a variable outside the for loop for break to work
                for (int i=0; i < K; ++i)
                {
                        for (int j=0; j < d; ++j)
                        {
                                brcounter += (A[i][j] - F[i][j]) * (A[i][j] - F[i][j]); //If cluster didn't move, this should be zero
                                                                                        //If no cluster moved, entire sum should be zero
                        }
                }
                if (brcounter < 0.00001) break; //if change is very small, break the loop; need this in the main for loop so the correct loop is broken

                //If there are differences between F and A, so that loop isn't broken,
                //here we assign new values of A to F
                for (int i=0; i < K; ++i)
                {
                        for (int j=0; j < d; ++j)
                        {
                                F[i][j]=A[i][j]; 
                        }
                }
        //end of iteration loop; will repeat until either break is achieved earlier or 100 iterations completed
        }       
        toc();
        printf("total time = %g\n", etime()); //Record time once loop is exited

        //Write array containing sample to cluster mapping to output file
        FILE* outputfile; 
        outputfile = fopen(argv[4],"w"); //Take name of output from 4th argument, then write to that file
        fprintf(outputfile, "%d\n", N);
        for (int b=0; b < N; ++b)
        {
                fprintf(outputfile, "%d\n",C[b]) ;
        } 
        fclose(outputfile); 
        
        

        //We need to free allocated memory to avoid memory leaks
        for (int i = 0; i < K; ++i)
        {
                free(A[i]);
        }
        free(A);

        for (int i = 0; i < K; ++i)
        {
                free(D[i]);
        }
        free(D);
        
        for (int i = 0; i < N; ++i)
        {
                free(B[i]);
        }
        free(B);
        
        free(C);
        free(E);

        return 0;
}

//This is the threads function; it computes the closest cluster to each sample, and then adds that sample's
//value to D while incrementing the counter array E, global arrays later used for reassigning the values of the clusters
void* CheckAndCount(void* rank)
{
        //First we need to divide up work
        long my_rank = (long) rank; //Use thread's rank as unique identifier in how to divide up work
        int chunk_size = N / P; //Divide up samples by thread count; Note that any remainder is ignored
        //Next 2 lines define unique start and end for each thread
        int my_start = my_rank * chunk_size; 
        int my_end = my_rank * chunk_size + chunk_size;
        if (my_rank == (P-1)) my_end = N;       //Any remainder of N/P will be done by last thread
                                                //Note that if this is a concern, it's possible to also divy up remainder
                                                //in a fair fashion, but in this case remainder is so small as to not be worth it

        //Since global array E[k] is going to be possibly accessed by multiple threads at once, we are going to have to mutex lock it
        //To avoid excessive numbers of locks, each thread gets its own local variable and waits until the very end before locking
        //and updating the global variable
        int my_E[K];
        for (int i=0; i < K; ++i)
        {
                my_E[i]=0;
        }

        //As with E and my_E, each thread also gets its own local D and waits until the very end to update it to avoid excessive locking/unlocking
        float my_D[K][d];
        for (int i=0; i < K; ++i)
        {
                for (int j=0; j < d; ++j)
                {
                        my_D[i][j]=0.0;
                }
        }
      
        
        for (int i=my_start; i < my_end; ++i) //Each thread loops over its own share of N/P samples
        {
                float min = 100; //Initialize min to some arbitrary large value at beginning of each loop
                for (int j=0; j < K; ++j)  //Each sample gets looped over all clusters
                {
                        float tmp=0;       //temporary value to store distance
                        //Loop over all dimensions and calculate differences squared between cluster and sample
                        for (int l = 0; l < d; ++l)
                        {
                                tmp += ( A[j][l] - B[i][l] ) * ( A[j][l] - B[i][l] ) ;
                        }
                        tmp = sqrt(tmp);  //take square root and you have distance from cluster and sample
                        if (tmp < min)    //If distance calculated is the smallest seen yet, store it as the new minimum
                                          //and associate sample with that given cluster
                        {
                                min = tmp;
                                C[i] = j; //Even though global variable, there's no need to lock this since each thread will be accessing different elements
                                          //This is an N-sized array and N is what is being divided among threads
                        }                
                }
                ++my_E[C[i]]; //Increment local counter for how many samples associated with cluster by one, by looking into last value stored into C[i] (which will be from 0 to K-1)

                //Add sample associated with given cluster to a temporary array which is used to reassign new cluster values (see code after thread function call in main)
                for (int m=0; m < d; ++m)
                {
                        my_D[C[i]][m] += B[i][m] ; 
                }
        }

        //E is a K-dimensional array which only counts how many times a cluster has had a sample associated with it
        //Because it is a global variable threads may access at same time, need to mutex lock it
        //Each thread adds its own local values to E
        pthread_mutex_lock(&mutex); //only one thread can access this at a time
        for (int i=0; i < K; ++i)
        {
                E[i] += my_E[i];
        }
        pthread_mutex_unlock(&mutex);//unlocks code so another thread can access it

        //Similar to above, each thread adds its own local values to the global variable summing all samples associated with each cluster       
        pthread_mutex_lock(&mutex);
        for (int i=0; i < K; ++i)
        {
                for (int j=0; j < d; ++j)
                {
                        D[i][j] += my_D[i][j] ; 
                }
        }
        pthread_mutex_unlock(&mutex);

        return NULL;
}


