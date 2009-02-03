/*
 * Driver program for a CUDA-based A5/1 rainbow table generator.
 *
 * Copyright (C) 2009: Ingo Albrecht <prom@berlin.ccc.de>
 */

#ifndef TEST_INTERMEDIATES
/* values below are for normal runs */

/*
 * These values are appropriate for a Quadro FX 570M.
 *
 * Before running this on different hardware, you
 * should decrease OPERATIONS_PER_RUN and then
 * increase it incrementally until you get
 * run lengths approaching 5 seconds.
 *
 * Thread and block count should be selected
 * so that they almost hit the register bound.
 *
 * If you want to tune the code for your card,
 * you should do it incrementally, keeping
 * the run length below 5 seconds, or your
 * graphics subsystem might go wonky.
 */

// number of threads per block
#define NUM_THREADS 32

// number of blocks to schedule
#define NUM_BLOCKS  32

// how long each run should be in cycles.
// must be a power of two for now.
#define OPERATIONS_PER_RUN  32768

#else
// values below are for intermediate testing

#define NUM_THREADS 10
#define NUM_BLOCKS  1

#define OPERATIONS_PER_RUN 32768

#endif

// total operations per chain (2^21)
#define OPERATIONS_PER_CHAIN 2097152

// number of chains to be computed
#define NUM_CHAINS NUM_THREADS * NUM_BLOCKS


#include <stdio.h>
#include <unistd.h>

#include <cutil.h>

#include "calculate_chain_kernel.cu"

int
main(int argc, char **argv) {
  CUT_DEVICE_INIT(argc, argv);

  uint32 i;

  uint64 start = 0; // XXX put your start vector here
  
  printf("Computing %d chains divided into %d blocks of %d threads, starting at 0x%16.16llx\n",
         NUM_CHAINS, NUM_BLOCKS, NUM_THREADS, start);

  uint32 num_runs = OPERATIONS_PER_CHAIN / OPERATIONS_PER_RUN;
  
  printf("Will execute %d runs of %d steps each.\n", num_runs, OPERATIONS_PER_RUN);

  // create a timer for the whole run
  unsigned int total_timer = 0;
  CUT_SAFE_CALL(cutCreateTimer(&total_timer));
  
  // compute size of state
  uint32  s_results = NUM_CHAINS * sizeof(uint64);
  
  // allocate and initialize host memory
  uint64* h_results = (uint64*) calloc(1, s_results);
  for(i = 0; i < NUM_CHAINS; i++) {
    h_results[i] = start + i;
  }
    
  // allocate and initialize device memory
  uint64* d_results;
  CUDA_SAFE_CALL(cudaMalloc((void**)&d_results, s_results));

  CUT_SAFE_CALL(cutStartTimer(total_timer));
  
  CUDA_SAFE_CALL(cudaMemcpy(d_results, h_results, s_results, cudaMemcpyHostToDevice));

  double total_run_time = 0.0;

  uint32 run;
  for(run = 0; run < num_runs; run++) {
    unsigned int run_timer = 0;
    CUT_SAFE_CALL(cutCreateTimer(&run_timer));
    
    uint32 index = OPERATIONS_PER_CHAIN - 1 - run * OPERATIONS_PER_RUN;

#ifdef TEST_INTERMEDIATES
    // print intermediates (for testing against calculate_chains_dump)
    for(i = 0; i < NUM_CHAINS; i++) {
      printf("results[%d] = 0x%16.16llx\n", i, h_results[i]);
    }
#endif
    
    printf("Run %3.3d/%3.3d, starting at index 0x%6.6x... ", run+1, num_runs, index);

    fflush(stdout);
    usleep(500*1000);
    
    CUT_SAFE_CALL(cutStartTimer(run_timer));

#ifdef TEST_INTERMEDIATES    
    CUDA_SAFE_CALL(cudaMemcpy(d_results, h_results, s_results, cudaMemcpyHostToDevice));
#endif
    
    dim3 gridDims(NUM_BLOCKS, 1, 1);
    dim3 blockDims(NUM_THREADS, 1, 1);
    crunch<<<gridDims, blockDims>>>(d_results, index);
    
    CUDA_SAFE_CALL(cudaThreadSynchronize());

#ifdef TEST_INTERMEDIATES
    CUDA_SAFE_CALL(cudaMemcpy(h_results, d_results, s_results, cudaMemcpyDeviceToHost));
#endif
    
    CUT_SAFE_CALL(cutStopTimer(run_timer));
    
    float run_time = cutGetTimerValue(run_timer);
    printf("%f ms.\n", run_time);
    total_run_time += run_time;
    fflush(stdout);

    
    CUT_SAFE_CALL(cutDeleteTimer(run_timer));
  }

  CUDA_SAFE_CALL(cudaMemcpy(h_results, d_results, s_results, cudaMemcpyDeviceToHost));

  CUT_SAFE_CALL(cutStopTimer(total_timer));

  // free device memory
  CUDA_SAFE_CALL(cudaFree((void**)d_results));

  // print results
  for(i = 0; i < NUM_CHAINS; i++) {
    printf("results[%d] = 0x%16.16llx\n", i, h_results[i]);
  }

  // free host memory  
  free(h_results);

  // report total time
  printf("Total time: %f ms, %f spent crunching\n", cutGetTimerValue(total_timer), total_run_time);

  // delete the whole-run timer  
  CUT_SAFE_CALL(cutDeleteTimer(total_timer));
  
  return 0;
}
