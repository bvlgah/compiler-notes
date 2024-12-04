#include <cstdint>
#include <cstdlib>
#include <hip/hip_runtime.h>
#include <hipblas/hipblas.h>
#include <iostream>
#include <nvbench/nvbench.cuh>
#include <random>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#define CHECK_HIP_ERROR(expr) do {                                        \
    hipError_t err = (expr);                                              \
    if (err != hipSuccess) {                                              \
        std::cerr << "HIP error: " << hipGetErrorString(err)              \
                  << " at line " << __LINE__ << std::endl;                \
        exit(EXIT_FAILURE);                                               \
    }                                                                     \
} while(0)

#define CHECK_HIPBLAS_ERROR(expr) do {                                    \
    hipblasStatus_t status = (expr);                                      \
    if (status != HIPBLAS_STATUS_SUCCESS) {                               \
        std::cerr << "hipBLAS error: " << status << " at line "           \
                  << __LINE__ << std::endl;                               \
        exit(EXIT_FAILURE);                                               \
    }                                                                     \
} while(0)

template <std::uint32_t BlockSize>
__global__ void matrixMulKernel(
    const float *matrixA, const float *matrixB, float *matrixC) {
  std::uint32_t matrixSize = blockDim.x * gridDim.x;

  std::uint32_t aStart = matrixSize * BlockSize * blockIdx.y;
  std::uint32_t aEnd = aStart + matrixSize;
  std::uint32_t aStep = BlockSize;
  std::uint32_t bStart = BlockSize * blockIdx.x;
  std::uint32_t bStep = BlockSize * matrixSize;

  float sum = 0.0f;
  for (std::uint32_t a = aStart, b = bStart;
       a < aEnd;
       a += aStep, b += bStep) {
    __shared__ float subMatA[BlockSize][BlockSize];
    __shared__ float subMatB[BlockSize][BlockSize];

    subMatA[threadIdx.y][threadIdx.x]
        = matrixA[a + threadIdx.y * matrixSize + threadIdx.x];
    subMatB[threadIdx.y][threadIdx.x]
        = matrixB[b + threadIdx.y * matrixSize + threadIdx.x];
    __syncthreads();

#pragma unroll
    for (std::uint32_t i = 0; i < BlockSize; i++) {
      sum += subMatA[threadIdx.y][i] * subMatB[i][threadIdx.x];
    }

    __syncthreads();
  }

  std::uint32_t start = matrixSize * BlockSize * blockIdx.y
      + BlockSize * blockIdx.x;
  matrixC[start + threadIdx.y * matrixSize + threadIdx.x] = sum;
}

__global__ void naiveMatrixMulKernel(
    const float *matrixA, const float *matrixB, float *matrixC) {
  std::uint32_t row = threadIdx.y + blockDim.y * blockIdx.y;
  std::uint32_t col = threadIdx.x + blockDim.x * blockIdx.x;
  std::uint32_t matrixSize = blockDim.x * gridDim.x;

  float sum = 0.0f;
  for (std::uint32_t i = 0; i < matrixSize; ++i)
    sum += matrixA[row * matrixSize + i] * matrixB[i * matrixSize + col];
  matrixC[row * matrixSize + col] = sum;
}

static void randomInitMatrix(thrust::host_vector<float> &matrix) {
  std::random_device rDevice;
  std::default_random_engine rEngine{rDevice()};
  std::uniform_real_distribution<float> dist{0.0f, 1000.0f};

  for (float &value : matrix)
    value = dist(rEngine);
}

static bool checkMatrixMulResult(
    const thrust::host_vector<float> &matrixA,
    const thrust::host_vector<float> &matrixB,
    const thrust::host_vector<float> &matrixC,
    std::uint32_t matrixSize) {
  for (std::uint32_t row = 0; row < matrixSize; ++row) {
    for (std::uint32_t col = 0; col < matrixSize; ++col) {
      float result = 0.0f;
      for (std::uint32_t i = 0; i < matrixSize; ++i) {
        result += matrixA[row * matrixSize + i] * matrixB[i * matrixSize + col];
      }

      float errAbs = std::abs(result - matrixC[row * matrixSize + col]);
      const float errRelThreshold = 10e-6f;

      if (errAbs > std::abs(result) * errRelThreshold) {
        std::cerr << "matrixC[" << row << ", " << col << "] = "
                  << matrixC[row * matrixSize + col]
                  << ", but " << result << " is expected" << std::endl;
        return false;
      }
    }
  }
  return true;
}

static void naiveMatrixMulThroughput(nvbench::state &state) {
  const std::uint32_t blockSize =
      static_cast<std::uint32_t>(state.get_int64("BlockSize"));
  const std::uint32_t matrixSize =
      static_cast<std::uint32_t>(state.get_int64("MatrixSize"));

  if (matrixSize % blockSize != 0) {
    std::cerr << "MatrixSize " << matrixSize
              << " cannot be evenly divided by BlockSize" << blockSize
              << std::endl;
    exit(EXIT_FAILURE);
  }

  state.add_element_count(matrixSize * matrixSize * matrixSize * 2,
      "Number of Operations");
  state.add_global_memory_reads(matrixSize * matrixSize * matrixSize * 2);
  state.add_global_memory_writes(matrixSize * matrixSize);

  thrust::host_vector<float> matrixA(matrixSize * matrixSize);
  thrust::host_vector<float> matrixB(matrixSize * matrixSize);
  thrust::host_vector<float> matrixC(matrixSize * matrixSize);

  randomInitMatrix(matrixA);
  randomInitMatrix(matrixB);

  thrust::device_vector<float> devMatrixA = matrixA;
  thrust::device_vector<float> devMatrixB = matrixB;
  thrust::device_vector<float> devMatrixC(matrixSize * matrixSize);

  state.exec([&](nvbench::launch &launch) {
    dim3 blockDim {blockSize, blockSize};
    dim3 gridDim {matrixSize / blockSize, matrixSize / blockSize};
    naiveMatrixMulKernel<<<gridDim, blockDim, 0, launch.get_stream()>>>(
        thrust::raw_pointer_cast(devMatrixA.data()),
        thrust::raw_pointer_cast(devMatrixB.data()),
        thrust::raw_pointer_cast(devMatrixC.data()));
  });

  matrixC = devMatrixC;
  if (!checkMatrixMulResult(matrixA, matrixB, matrixC, matrixSize)) {
    std::cerr << "Result of matrix multiplication is incorrect" << std::endl;
    exit(EXIT_FAILURE);
  }
}

template <std::uint32_t BlockSize>
static void matrixMulThroughput(nvbench::state &state,
    nvbench::type_list<nvbench::enum_type<BlockSize>>) {
  const std::uint32_t matrixSize =
      static_cast<std::uint32_t>(state.get_int64("MatrixSize"));

  if (matrixSize % BlockSize != 0) {
    std::cerr << "MatrixSize " << matrixSize
              << " cannot be evenly divided by BlockSize" << BlockSize
              << std::endl;
    exit(EXIT_FAILURE);
  }

  state.add_element_count(matrixSize * matrixSize * matrixSize * 2,
      "Number of Operations");
  state.add_global_memory_reads(
      matrixSize * matrixSize * (matrixSize / BlockSize) * 2);
  state.add_global_memory_writes(matrixSize * matrixSize);

  thrust::host_vector<float> matrixA(matrixSize * matrixSize);
  thrust::host_vector<float> matrixB(matrixSize * matrixSize);
  thrust::host_vector<float> matrixC(matrixSize * matrixSize);

  randomInitMatrix(matrixA);
  randomInitMatrix(matrixB);

  thrust::device_vector<float> devMatrixA = matrixA;
  thrust::device_vector<float> devMatrixB = matrixB;
  thrust::device_vector<float> devMatrixC(matrixSize * matrixSize);

  state.exec([&](nvbench::launch &launch) {
    dim3 blockDim {BlockSize, BlockSize};
    dim3 gridDim {matrixSize / BlockSize, matrixSize / BlockSize};
    matrixMulKernel<BlockSize>
        <<<gridDim, blockDim, 0, launch.get_stream()>>>(
        thrust::raw_pointer_cast(devMatrixA.data()),
        thrust::raw_pointer_cast(devMatrixB.data()),
        thrust::raw_pointer_cast(devMatrixC.data()));
  });

  matrixC = devMatrixC;
  if (!checkMatrixMulResult(matrixA, matrixB, matrixC, matrixSize)) {
    std::cerr << "Result of matrix multiplication is incorrect" << std::endl;
    exit(EXIT_FAILURE);
  }
}

static void blasMatrixMulThroughput(nvbench::state &state) {
  const std::uint32_t matrixSize =
      static_cast<std::uint32_t>(state.get_int64("MatrixSize"));

  state.add_element_count(matrixSize * matrixSize * matrixSize * 2,
      "Number of Operations");

  thrust::host_vector<float> matrixA(matrixSize * matrixSize);
  thrust::host_vector<float> matrixB(matrixSize * matrixSize);
  thrust::host_vector<float> matrixC(matrixSize * matrixSize);

  randomInitMatrix(matrixA);
  randomInitMatrix(matrixB);

  thrust::device_vector<float> devMatrixA = matrixA;
  thrust::device_vector<float> devMatrixB = matrixB;
  thrust::device_vector<float> devMatrixC = matrixC;

  // Create hipBLAS handle
  hipblasHandle_t handle;
  CHECK_HIPBLAS_ERROR(hipblasCreate(&handle));

  state.exec([&](nvbench::launch &launch) {
    float alpha = 1.0f;
    float beta = 0.0f;
    CHECK_HIPBLAS_ERROR(hipblasSgemm(
        handle,
        HIPBLAS_OP_N,
        HIPBLAS_OP_N,
        matrixSize,
        matrixSize,
        matrixSize,
        &alpha,
        thrust::raw_pointer_cast(devMatrixA.data()),
        matrixSize,
        thrust::raw_pointer_cast(devMatrixB.data()),
        matrixSize,
        &beta,
        thrust::raw_pointer_cast(devMatrixC.data()),
        matrixSize));
  });

  matrixC = devMatrixC;
  hipblasDestroy(handle);

  if (!checkMatrixMulResult(matrixA, matrixB, matrixC, matrixSize)) {
    std::cerr << "Result of matrix multiplication is incorrect" << std::endl;
    exit(EXIT_FAILURE);
  }
}

NVBENCH_BENCH(naiveMatrixMulThroughput)
    .add_int64_power_of_two_axis("MatrixSize", nvbench::range(8, 10, 1))
    .add_int64_power_of_two_axis("BlockSize", nvbench::range(3, 5, 1));

using BlockSize = nvbench::enum_type_list<
    std::uint32_t(8),
    std::uint32_t(16),
    std::uint32_t(32)>;

NVBENCH_BENCH_TYPES(matrixMulThroughput, NVBENCH_TYPE_AXES(BlockSize))
    .add_int64_power_of_two_axis("MatrixSize", nvbench::range(8, 10, 1));

// NVBENCH_BENCH(blasMatrixMulThroughput)
//     .add_int64_power_of_two_axis("MatrixSize", nvbench::range(8, 10, 1));
