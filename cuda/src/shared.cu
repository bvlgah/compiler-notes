#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

__global__ void sharedMemoryReadKernel(
    std::int32_t * __restrict__ output, const std::size_t stride) {
  extern __shared__ std::uint32_t shared[];
  const std::size_t tid = threadIdx.x + blockIdx.x * blockDim.x;
  const std::size_t idx = tid * stride;

  auto startClock = clock();
  shared[idx]++;
  auto endClock = clock();
  output[tid] = endClock - startClock;
}

static std::ostream
&operator<<(std::ostream &output,
    const thrust::host_vector<std::int32_t> &vec) {
  auto it = vec.begin();
  output << '[';

  if (it != vec.end()) {
    output << *it;
    while (++it != vec.end())
      output << ", " << *it;
  }

  output << ']';
  return output;
}

void sharedMemoryThroughput(std::size_t stride, std::size_t threadSize) {
  const std::size_t sharedSize = threadSize * stride * sizeof(std::uint32_t);
  thrust::device_vector<std::int32_t> output(threadSize);
  sharedMemoryReadKernel<<<1, threadSize, sharedSize>>>(
      thrust::raw_pointer_cast(output.data()), stride);

  thrust::host_vector<std::int32_t> timingVec = output;
  std::int32_t maxTiming = *std::max_element(
      timingVec.begin(), timingVec.end());
  std::int32_t minTiming = *std::min_element(
      timingVec.begin(), timingVec.end());
  std::cout << "Stride is " << stride
            << ", ThreadSize is " << threadSize << std::endl;
  std::cout << "Timing vector is " << timingVec << std::endl;
  std::cout << "Max Timing is " << maxTiming << " clocks" << std::endl;
  std::cout << "Min Timing is " << minTiming << " clocks" << std::endl;
}

int main(void) {
  std::array<std::size_t, 8> strides {1, 2, 4, 8, 16, 28, 31, 32};
  for (std::size_t stride : strides)
    sharedMemoryThroughput(stride, 32);
  return EXIT_SUCCESS;
}
