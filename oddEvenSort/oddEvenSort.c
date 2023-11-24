/**
 * @file oddEvenSort.c
 * @author krisp3t (github.com/krisp3t)
 * @brief Parallel odd-even sort algorithm implemented using pthreads in C
 * @date 2023-11-24
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 10000

/// Thread arguments
typedef struct
{
    int id;                // Thread ID
    unsigned int *p_array; // Pointer to the portion of the array of the thread
    int size;              // Size of the portion of the array of the thread
} thread_args_T;

struct timespec time_start, time_stop; // For measuring execution time

const int BLOCK_SIZE = ARRAY_SIZE / NUM_THREADS;

unsigned int arr[ARRAY_SIZE];
unsigned int *p_array = arr;

pthread_t thread[NUM_THREADS];
pthread_barrier_t barrier;
thread_args_T thread_args[NUM_THREADS];

void *(thread_func)(void *arg);
void *(*thread_func_ptr)(void *arg) = thread_func;

void print_array(unsigned int *p_array)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("%d ", p_array[i]);
    }
    printf("\n");
}

/**
 * @brief If not in order, switch the values of a and b
 *
 * @param a
 * @param b
 */
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

/**
 * @brief Every even indexed element is compared with the next odd indexed element and switched if not in order
 *
 * @param i_end index of the last element of the array to be sorted
 * @param p_array
 */
void even_phase(int i_end, unsigned int *p_array)
{
    for (int i = 0; i < i_end; i += 2)
    {
        compare_and_switch(&p_array[i], &p_array[i + 1]);
    }
}

/**
 * @brief Every odd indexed element is compared with the next even indexed element and switched if not in order
 *
 * @param i_end index of the last element of the array to be sorted
 * @param p_array
 */
void odd_phase(int i_end, unsigned int *p_array)
{
    for (int i = 1; i < i_end; i += 2)
    {
        compare_and_switch(&p_array[i], &p_array[i + 1]);
    }
}

void *thread_func(void *args)
{
    thread_args_T *pthread_args = (thread_args_T *)args;

    /// Every thread sorts its portion of the array
    /// Between phases threads must wait at the barrier to ensure that all threads have finished their current pass
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        even_phase(pthread_args->size, pthread_args->p_array);
        pthread_barrier_wait(&barrier);
        odd_phase(pthread_args->size, pthread_args->p_array);
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    // Initialize array
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        p_array[i] = rand() % 100;
    }

    printf("Unsorted array: ");
    print_array(p_array);

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    clock_gettime(CLOCK_REALTIME, &time_start);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i].id = i;
        thread_args[i].p_array = p_array + (i * BLOCK_SIZE);
        if (thread_args[i].id < NUM_THREADS - 1)
        {
            thread_args[i].size = BLOCK_SIZE;
        }
        else
        {
            // The last thread gets the remaining elements
            thread_args[i].size = BLOCK_SIZE + (ARRAY_SIZE % NUM_THREADS) - 1;
        }
        pthread_create(&thread[i], NULL, thread_func_ptr, (void *)&thread_args[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(thread[i], NULL);
    }
    clock_gettime(CLOCK_REALTIME, &time_stop);
    double elapsedTime = (time_stop.tv_sec - time_start.tv_sec) + (time_stop.tv_nsec - time_start.tv_nsec) / 1e9;
    pthread_barrier_destroy(&barrier);

    printf("Sorted array: ");
    print_array(p_array);
    printf("Elapsed time: %f\n", elapsedTime);

    return 0;
}