/************************************************************************
 * 
 * CSE130 Assignment 1
 * 
 * UNIX Shared Memory Multi-Process Merge Sort
 * 
* Copyright (C) 2020-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ************************************************************************/

// Sources:
// https://stackoverflow.com/questions/44174911/what-will-be-length-of-array-in-shared-memory

#include "merge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <unistd.h>

/* 
 * Merge Sort in the current process a sub-array of ARR[] defined by the 
 * LEFT and RIGHT indexes.
 */
void singleProcessMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleProcessMergeSort(arr, left, middle); 
    singleProcessMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

/* 
 * Merge Sort in the current process and at least one child process a 
 * sub-array of ARR[] defined by the LEFT and RIGHT indexes.
 */
void multiProcessMergeSort(int arr[], int left, int right) 
{
  // Delete this line, it's only here to fail the code quality check
  //int i = 0;

  // Your code goes here 
  if(left < right)
  {
    // creating shared memory
    int shmid = shmget(IPC_PRIVATE, (right + 1) * sizeof(int), 0666|IPC_CREAT);
    // attaching to shared memory
    int *shm_array = ((int*) shmat(shmid, 0, 0));
    // copying local memory into shared memory
    memcpy(shm_array, arr, (right + 1) * sizeof(int));

    switch(fork())
    {
      // if error
      case -1:    
        printf("fork failed/n");
        exit(-1);

      // if child
      case 0:     
        // sort all shared memory
        singleProcessMergeSort(shm_array, ((left+right)/2) + 1, right);
        exit(0);
      
      // if parent
      default:    
        // sort left side of local memory
        singleProcessMergeSort(arr, left, ((left+right)/2));
        wait(NULL);
        // copy shared memory to right side of local memory
        memcpy(&(arr[((left+right)/2) + 1]), &(shm_array[((left+right)/2) + 1]), sizeof(int) * (right - ((left+right)/2)));
        // detach from shared memory
        shmdt(shm_array);
        // destroy shared memory
        shmctl(shmid, IPC_RMID, NULL);
        
        // merging local memory
        merge(arr, left, ((left+right)/2), right);
    }
  }
}
