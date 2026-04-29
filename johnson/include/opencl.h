#ifndef OPENCL_H
#define OPENCL_H

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#define CL_CHECK(error) opencl_check_error(error, __FILE__, __LINE__)

typedef struct OpenCLContext {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
} OpenCLContext;

void opencl_check_error(cl_int error, const char* file, int line);
OpenCLContext initialize_opencl();
void destroy_context(OpenCLContext* context);
char* read_kernel(const char* file_path);
cl_kernel build_kernel(OpenCLContext* context, const char* file_name, const char* kernel_name);
void start_kernel(OpenCLContext* context, cl_kernel kernel, size_t global_work_size, size_t local_work_size);
void set_kernel_argument(cl_kernel kernel, cl_uint index, size_t size, const void* value);
cl_mem create_buffer(OpenCLContext* context, char* mode, size_t size);
void write_to_device(OpenCLContext* context, cl_mem device_buffer, size_t size, const void* host_buffer);
void read_from_device(OpenCLContext* context, cl_mem device_buffer, size_t size, void* host_buffer);

#endif // OPENCL_H
