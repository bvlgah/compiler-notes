#include <algorithm>
#include <cstddef>
#include <iostream>
#include <nvbench/nvbench.cuh>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

__global__ void sharedMemoryReadKernel(
    nvbench::int32_t * __restrict__ output, const std::size_t stride) {
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
    const thrust::host_vector<nvbench::int32_t> &vec) {
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

void sharedMemoryThroughput(nvbench::state &state) {
  const std::size_t stride =
    static_cast<std::size_t>(state.get_int64("Stride"));
  const std::size_t threadSize =
    static_cast<std::size_t>(state.get_int64("ThreadSize"));
  const std::size_t sharedSize = threadSize * stride * sizeof(std::uint32_t);

  thrust::device_vector<nvbench::int32_t> output(threadSize);

  state.add_element_count(threadSize, "Elements");
  state.add_global_memory_reads<nvbench::int32_t>(threadSize, "DataSize");
  state.add_global_memory_writes<nvbench::int32_t>(threadSize);

  state.exec([stride, threadSize, sharedSize, &output](
        nvbench::launch &launch){
    sharedMemoryReadKernel<<<1, threadSize, sharedSize, launch.get_stream()>>>(
        thrust::raw_pointer_cast(output.data()),
        stride);
  });

  thrust::host_vector<nvbench::int32_t> timingVec = output;
  nvbench::int32_t maxTiming = *std::max_element(
      timingVec.begin(), timingVec.end());
  nvbench::int32_t minTiming = *std::min_element(
      timingVec.begin(), timingVec.end());
  std::cout << "Timing vector is " << timingVec << std::endl;
  std::cout << "Max Timing is " << maxTiming << " clocks" << std::endl;
  std::cout << "Min Timing is " << minTiming << " clocks" << std::endl;
}

NVBENCH_BENCH(sharedMemoryThroughput)
    .add_int64_power_of_two_axis("Stride", nvbench::range(0, 6, 1))
    .add_int64_power_of_two_axis("ThreadSize", nvbench::range(5, 8, 1));
