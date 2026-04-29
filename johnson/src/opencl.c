#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opencl.h"

void opencl_check_error(cl_int error, const char* file, int line) {

    if (error != CL_SUCCESS) {
        fprintf(stderr, "OpenCL error [%d] at '%s': line %d\n", error, file, line);
        exit(EXIT_FAILURE);
    }
}

OpenCLContext initialize_opencl() {

    OpenCLContext context;
    cl_int error;
    cl_uint n_platforms;
    cl_uint n_devices;

    error = clGetPlatformIDs(1, &context.platform, &n_platforms);
    CL_CHECK(error);

    error = clGetDeviceIDs(context.platform, CL_DEVICE_TYPE_GPU, 1, &context.device, &n_devices);
    CL_CHECK(error);

    context.context = clCreateContext(NULL, 1, &context.device, NULL, NULL, &error);
    CL_CHECK(error);

    context.queue = clCreateCommandQueue(context.context, context.device, 0, &error);
    CL_CHECK(error);

    return context;
}

void destroy_context(OpenCLContext* context) {

    if (context == NULL) {
        return;
    }

    clReleaseCommandQueue(context->queue);
    clReleaseContext(context->context);
}

char* read_kernel(const char* file_path) {

    FILE* file = fopen(file_path, "rb");

    if (!file) {
        perror("Could not open file for reading!");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long int file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);

    if (!buffer) {
        perror("Memory allocation failed!");
        fclose(file);
        
        return NULL;
    }

    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

cl_kernel build_kernel(OpenCLContext* context, const char* file_path, const char* kernel_name) {

    cl_int error;
    char* kernel_code = read_kernel(file_path);

    cl_program program = clCreateProgramWithSource(context->context, 1, (const char**)&kernel_code, NULL, &error);
    CL_CHECK(error);

    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    CL_CHECK(error);

    cl_kernel kernel = clCreateKernel(program, kernel_name, &error);
    CL_CHECK(error);

    clReleaseProgram(program);
    free(kernel_code);

    return kernel;
}

void set_kernel_argument(cl_kernel kernel, cl_uint index, size_t size, const void* value) {

    cl_int error = clSetKernelArg(kernel, index, size, value);
    CL_CHECK(error);
}

void start_kernel(OpenCLContext* context, cl_kernel kernel, size_t global_work_size, size_t local_work_size) {

    cl_int error = clEnqueueNDRangeKernel(
        context->queue,
        kernel,
        1,
        NULL,
        &global_work_size,
        &local_work_size,
        0,
        NULL,
        NULL
    );

    CL_CHECK(error);
}

cl_mem create_buffer(OpenCLContext* context, char* mode, size_t size) {

    cl_int error;
    cl_mem_flags flag;

    if (strcmp(mode, "read") == 0) {
        flag = CL_MEM_READ_ONLY;
    }
    else if (strcmp(mode, "write") == 0) {
        flag = CL_MEM_WRITE_ONLY;
    }
    else if (strcmp(mode, "read_write") == 0) {
        flag = CL_MEM_READ_WRITE;
    }
    else {
        fprintf(stderr, "OpenCL error when creating buffer: unknown memory mode %s\n", mode);
        exit(EXIT_FAILURE);
    }

    cl_mem buffer = clCreateBuffer(context->context, flag, size, NULL, &error);
    CL_CHECK(error);

    return buffer;
}

void write_to_device(OpenCLContext* context, cl_mem device_buffer, size_t size, const void* host_buffer) {

    cl_int error = clEnqueueWriteBuffer(
        context->queue,
        device_buffer,
        CL_FALSE,
        0,
        size,
        host_buffer,
        0,
        NULL,
        NULL
    );

    CL_CHECK(error);
}

void read_from_device(OpenCLContext* context, cl_mem device_buffer, size_t size, void* host_buffer) {

    cl_int error = clEnqueueReadBuffer(
        context->queue,
        device_buffer,
        CL_TRUE,
        0,
        size,
        host_buffer,
        0,
        NULL,
        NULL
    );
    
    CL_CHECK(error);
}
