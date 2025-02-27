/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/
#include "xcl2.hpp"
#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>

using std::default_random_engine;
using std::generate;
using std::uniform_int_distribution;
using std::vector;

// row major
void matmul(vector<int, aligned_allocator<int> >& C,
            vector<int, aligned_allocator<int> >& A,
            vector<int, aligned_allocator<int> >& B,
            int M) {
    for (int k = 0; k < M; k++) {
        for (int j = 0; j < M; j++) {
            for (int i = 0; i < M; i++) {
                C[k * M + j] += A[k * M + i] * B[i * M + j];
            }
        }
    }
}

int gen_random() {
    static default_random_engine e;
    static uniform_int_distribution<int> dist(0, 10);
    return dist(e);
}

void print(vector<int, aligned_allocator<int> >& data, int dims) {
    vector<int> out(dims * dims);
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 10; c++) {
            printf("%4d ", data[r * dims + c]);
        }
        printf("…\n");
    }
    for (int r = 0; r < 10; r++) {
        printf("   %s ", "…");
    }
    printf("⋱\n\n");
}

void verify(vector<int, aligned_allocator<int> >& gold, vector<int, aligned_allocator<int> >& output) {
    for (int i = 0; i < (int)output.size(); i++) {
        if (output[i] != gold[i]) {
            printf("Mismatch %d: gold: %d device: %d\n", i, gold[i], output[i]);
            exit(EXIT_FAILURE);
        }
    }
}

// This example illustrates how to use array partitioning attributes in OpenCL
// kernels for FPGA devices using matmul.
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];
    static const int dims = 16;
    cl_int err;
    cl::Program program;
    cl::CommandQueue q;
    cl::Context context;

    /* less iteration for emulation mode */
    int iteration = xcl::is_emulation() ? 2 : 100;

    vector<int, aligned_allocator<int> > A(dims * dims);
    vector<int, aligned_allocator<int> > B(dims * dims);
    vector<int, aligned_allocator<int> > gold(dims * dims, 0);
    vector<int, aligned_allocator<int> > C(dims * dims, 0);
    generate(begin(A), end(A), gen_random);
    generate(begin(B), end(B), gen_random);

    printf("A:\n");
    print(A, dims);
    printf("B:\n");
    print(B, dims);
    matmul(gold, A, B, dims);

    printf("Gold:\n");
    print(gold, dims);
    auto devices = xcl::get_xil_devices();

    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        program = cl::Program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    // compute the size of array in bytes
    size_t array_size_bytes = dims * dims * sizeof(int);
    OCL_CHECK(err,
              cl::Buffer buffer_a(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, array_size_bytes, A.data(), &err));
    OCL_CHECK(err,
              cl::Buffer buffer_b(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, array_size_bytes, B.data(), &err));
    OCL_CHECK(err,
              cl::Buffer buffer_c(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, array_size_bytes, C.data(), &err));

    OCL_CHECK(err, cl::Kernel matmul_kernel(program, "matmul_naive", &err));

    OCL_CHECK(err, err = matmul_kernel.setArg(0, buffer_a));
    OCL_CHECK(err, err = matmul_kernel.setArg(1, buffer_b));
    OCL_CHECK(err, err = matmul_kernel.setArg(2, buffer_c));
    OCL_CHECK(err, err = matmul_kernel.setArg(3, dims));

    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_a, buffer_b}, 0 /* 0 means from host*/));

    cl::Event event;
    uint64_t nstimestart, nstimeend;
    uint64_t matmul_time = 0;
    for (int i = 0; i < iteration; i++) {
        OCL_CHECK(err, err = q.enqueueTask(matmul_kernel, nullptr, &event));
        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_c}, CL_MIGRATE_MEM_OBJECT_HOST));
        q.finish();
        OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart));
        OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend));
        matmul_time += nstimeend - nstimestart;
        verify(gold, C);
    }

    OCL_CHECK(err, cl::Kernel matmul_partition_kernel(program, "matmul_partition", &err));

    OCL_CHECK(err, err = matmul_partition_kernel.setArg(0, buffer_a));
    OCL_CHECK(err, err = matmul_partition_kernel.setArg(1, buffer_b));
    OCL_CHECK(err, err = matmul_partition_kernel.setArg(2, buffer_c));
    OCL_CHECK(err, err = matmul_partition_kernel.setArg(3, dims));

    uint64_t matmul_partition_time = 0;
    for (int i = 0; i < iteration; i++) {
        OCL_CHECK(err, err = q.enqueueTask(matmul_partition_kernel, nullptr, &event));
        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_c}, CL_MIGRATE_MEM_OBJECT_HOST));
        q.finish();
        OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart));
        OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend));
        matmul_partition_time += nstimeend - nstimestart;
        verify(gold, C);
    }

    printf(
        "|-------------------------+-------------------------|\n"
        "| Kernel(%3d iterations)  |    Wall-Clock Time (ns) |\n"
        "|-------------------------+-------------------------|\n",
        iteration);
    printf("| %-23s | %23lu |\n", "matmul: naive", matmul_time);
    printf("| %-23s | %23lu |\n", "matmul: partition", matmul_partition_time);
    printf("|-------------------------+-------------------------|\n");
    printf(
        "Note: Wall Clock Time is meaningful for real hardware execution "
        "only, not for emulation.\n");
    printf(
        "Please refer to profile summary for kernel execution time for "
        "hardware emulation.\n");
    printf("TEST PASSED\n\n");

    return EXIT_SUCCESS;
}
