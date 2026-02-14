#include <stdio.h>
#include <cuda_runtime.h>

__global__ void hello_kernel() {
    printf("Hello from GPU! Block %d, Thread %d\n",
           blockIdx.x, threadIdx.x);
}

int main() {
    // Launch 1 block with 8 threads
    hello_kernel<<<1, 8>>>();

    // Wait for GPU to finish
    cudaDeviceSynchronize();

    printf("Hello from CPU!\n");
    return 0;
}

