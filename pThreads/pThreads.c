#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 100
#define ROUND(x) (int)(x + 0.5)

typedef struct
{
    int id;
    unsigned int *pArray;
    int size;
} thread_args_T;
struct timespec timeStart, timeStop;

int iBlockSize = ARRAY_SIZE / NUM_THREADS;

unsigned int arr[ARRAY_SIZE];
unsigned int *pArray = arr;

pthread_t thread[NUM_THREADS];
pthread_barrier_t barrier;
thread_args_T thread_args[NUM_THREADS];

void *(thread_func)(void *arg) {}
void *(*thread_func_ptr)(void *arg) = thread_func;

void print_array(unsigned int *pArray)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("%d ", pArray[i]);
    }
    printf("\n");
}

void compare_and_switch(unsigned int *a, unsigned int *b)
{
    unsigned int temp;
    if (*a > *b)
    {
        temp = *a;
        *a = *b;
        *b = temp;
    }
}

void even_phase(int iEnd, unsigned int *pArray)
{
    for (int i = 0; i < iEnd; i += 2)
    {
        compare_and_switch(&pArray[i], &pArray[i + 1]);
    }
}

void odd_phase(int iEnd, unsigned int *pArray)
{
    for (int i = 1; i < iEnd; i += 2)
    {
        compare_and_switch(&pArray[i], &pArray[i + 1]);
    }
}

void *thread_func(void *args)
{
    thread_args_T *pThreadArgs = (thread_args_T *)args;

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        even_phase(pThreadArgs->size, pThreadArgs->pArray);
        pthread_barrier_wait(&barrier);
        odd_phase(pThreadArgs->size, pThreadArgs->pArray);
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    // Initialize array
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        pArray[i] = rand() % 100;
    }

    printf("Unsorted array: ");
    print_array(pArray);

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    clock_gettime(CLOCK_REALTIME, &timeStart);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i].id = i;
        thread_args[i].pArray = pArray + (i * iBlockSize);
        if (thread_args[i].id < NUM_THREADS - 1)
        {
            thread_args[i].size = iBlockSize;
        }
        else
        {
            thread_args[i].size = iBlockSize + (ARRAY_SIZE % NUM_THREADS) - 1;
        }
        pthread_create(&thread[i], NULL, thread_func_ptr, (void *)&thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(thread[i], NULL);
    }
    clock_gettime(CLOCK_REALTIME, &timeStop);
    double elapsedTime = (timeStop.tv_sec - timeStart.tv_sec) / 1e9;
    pthread_barrier_destroy(&barrier);

    printf("Sorted array: ");
    print_array(pArray);
    printf("Elapsed time: %f\n", elapsedTime);

    return 0;
}

/*
Parallelization:
- Every thread
*/
