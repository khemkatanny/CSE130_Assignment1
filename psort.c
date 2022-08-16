/************************************************************************
 * 
 * CSE130 Assignment 1
 *  
 * POSIX Shared Memory Multi-Process Merge Sort
 * 
 * Copyright (C) 2020-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ************************************************************************/

// Sources:
// https://linux.die.net/man/2/ftruncate
// https://man7.org/linux/man-pages/man3/shm_open.3.html
// https://pubs.opengroup.org/onlinepubs/009604499/functions/mmap.html

#include "merge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>

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
  if (left < right)
  {
    int sizeOfArray = 16384;

    //creating shared memory
    const char *name = "tkhemka";
    int new_shm = shm_open(name, O_RDWR | O_CREAT, 0666);
    
    //attaching to shared memory
    ftruncate(new_shm, sizeOfArray);  //setting size of shared memory
    int *shm_mem = mmap(0, sizeOfArray, PROT_WRITE, MAP_SHARED, new_shm, 0);
    
    //copying right side of local memory into shared memory
    memcpy(shm_mem, arr, sizeof(int)*(right+1));

    //fork
    switch(fork())
    {
      //if error
      case -1:
        printf("fork failed/n");
        exit(-1);

      //if child
      case 0:
        //sorting one side of shared memory
        singleProcessMergeSort(shm_mem, ((left+right)/2)+1, right);
        exit(0);

      //if parent
      default:
        //sorting the left side of shared memory
        singleProcessMergeSort(arr, left, (left+right)/2);
        wait(NULL);

        //copying shared memory to right side of local memory
        int merge_mem = right - ((left+right)/2);
        memcpy(&(arr[((left+right)/2) + 1]), &(shm_mem[((left+right)/2) + 1]), sizeof(int)*merge_mem);

        //detaching from shared memory
        shm_unlink(name);

        //merging local memory
        merge(arr, left, (left+right)/2, right);
    }
  }
}
