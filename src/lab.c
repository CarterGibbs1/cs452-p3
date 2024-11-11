#include <stdlib.h>
#include <sys/time.h> /* for gettimeofday system call */
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include "lab.h"

// Debug methods
void printArr(int A[], int n);

typedef struct mergesort_s_args {
    void *A;
    int p;
    int r;
} mergesort_s_args;

typedef struct merge_arguments {
    int n;  // size of array
    int num_arrays; // equal to num threads
    int* array;
} merge_arguments;

void* mergesort_mt_to_s(void *);

/**
 * @brief Standard insertion sort that is faster than merge sort for small array's
 *
 * @param A The array to sort
 * @param p The starting index
 * @param r The ending index
 */
static void insertion_sort(int A[], int p, int r)
{
  int j;

  for (j = p + 1; j <= r; j++)
    {
      int key = A[j];
      int i = j - 1;
      while ((i > p - 1) && (A[i] > key))
        {
	  A[i + 1] = A[i];
	  i--;
        }
      A[i + 1] = key;
    }
}

void merge_sorted_arrays(merge_arguments *args) {
    // Have array of integers, related to pointer of index
    // Check which index's value is smallest, append to new array
    // Have method to check if index is out of bound

    // Allocate return array
    int curr = 0;
    int *retVal = (int *) malloc(args->n * sizeof(int));

    // Allocate index array
    int *indexes = (int *) malloc(args->num_arrays * sizeof(int));
    int chunk_size = args->n / args->num_arrays;
    for (int i = 0; i < args->num_arrays; i++) {
        indexes[i] = floor(i * chunk_size);
    }

    // Loop through each index to find which element is smallest
    for (int i = 0; i < args->n; i++) {
        int smallestElement = INT_MAX;
        int smallestIndex = -1;
        for (int j = 0; j < args->num_arrays; j++) {
            int idx = indexes[j];

            // If index is out of bounds // Array already merged
            if (idx >= args->n || ((j < args->num_arrays - 1) && (idx >= floor((j + 1) * chunk_size)))) {
                continue;
            }

            int element = args->array[idx];
            if (element < smallestElement) {
                smallestElement = element;
                smallestIndex = j;
            }
        }

        /**
        printArr(retVal, args->n);
        printf("curr: %i\n", curr);
        printf("smallest element: %i\n", smallestElement);
        printf("smallest index: %i\n", smallestIndex);
        */

        retVal[curr] = smallestElement;
        curr++;
        indexes[smallestIndex]++;
    }


    // copy merged output from retVal back to array
    for (int i = 0; i < args->n; i++) {
        args->array[i] = retVal[i];
    }

    // Free memory
    free(retVal);
    free(indexes);
}

void mergesort_mt(int A[], int n, int num_thread) {
    // Create threads
    pthread_t threads[num_thread];
    mergesort_s_args **arguments = (mergesort_s_args**) malloc(num_thread * sizeof(mergesort_s_args*));
    int chunk_size = n / num_thread;
    for (int i = 0; i < num_thread; i++) {
        arguments[i] = (mergesort_s_args*) malloc(sizeof(mergesort_s_args));
        arguments[i]->A = A;
        arguments[i]->p = floor(i * chunk_size);
        arguments[i]->r = (i != num_thread - 1) ? floor((i+1) * chunk_size - 1) : n - 1;
        //printf("Arguments: %d, %d\n", arguments->p, arguments->r);
        pthread_create(&threads[i], NULL, mergesort_mt_to_s, (void *) arguments[i]);
    }

    // Join threads
    //int p, q, r;
    for (int i = 0; i < num_thread; i++) {
        pthread_join(threads[i], NULL);
        free(arguments[i]);

        /**
        if (i == 0) continue;
        p = 0;
        q = i * chunk_size;
        r = (i != num_thread - 1) ? floor((i+1) * chunk_size - 1) : n - 1;

        merge_s(A, p, q, r);
         */
    }

    merge_arguments *merge_args = (merge_arguments *) malloc(sizeof(merge_arguments));
    merge_args->array = A;
    merge_args->n = n;
    merge_args->num_arrays = num_thread;

    merge_sorted_arrays(merge_args);

    free(arguments);
    free(merge_args);
}

void* mergesort_mt_to_s(void *arguments) {
    mergesort_s_args *mergesort_args = (mergesort_s_args*) arguments;
    //printf("%d, %d\n", mergesort_args->p, mergesort_args->r);
    mergesort_s(mergesort_args->A, mergesort_args->p, mergesort_args->r);
    return NULL;
}

void mergesort_s(int A[], int p, int r)
{
  if (r - p + 1 <=  INSERTION_SORT_THRESHOLD)
    {
      insertion_sort(A, p, r);
    }
  else
    {
      int q = (p + r) / 2;
      mergesort_s(A, p, q);
      mergesort_s(A, q + 1, r);
      merge_s(A, p, q, r);
    }

}

void merge_s(int A[], int p, int q, int r)
{
  int *B = (int *)malloc(sizeof(int) * (r - p + 1));

  int i = p;
  int j = q + 1;
  int k = 0;
  int l;

  /* as long as both lists have unexamined elements */
  /*  this loop keeps executing. */
  while ((i <= q) && (j <= r))
    {
      if (A[i] < A[j])
        {
	  B[k] = A[i];
	  i++;
        }
      else
        {
	  B[k] = A[j];
	  j++;
        }
      k++;
    }

  /* now only at most one list has unprocessed elements. */
  if (i <= q)
    {
      /* copy remaining elements from the first list */
      for (l = i; l <= q; l++)
        {
	  B[k] = A[l];
	  k++;
        }
    }
  else
    {
      /* copy remaining elements from the second list */
      for (l = j; l <= r; l++)
        {
	  B[k] = A[l];
	  k++;
        }
    }

  /* copy merged output from array B back to array A */
  k = 0;
  for (l = p; l <= r; l++)
    {
      A[l] = B[k];
      k++;
    }

  free(B);
}

double getMilliSeconds()
{
  struct timeval now;
  gettimeofday(&now, (struct timezone *)0);
  return (double)now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}

// Debug Methods

void printArr(int A[], int n) {
    for (int i = 0; i < n; i++) printf("%d, ", A[i]);
    printf("\n\n");
}