#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <CL/cl.h>
#include <lodePNG.h>

char* read_kernel(char* pathFile, char* source_str){

		FILE *fp;
		size_t program_size;

		fp = fopen(pathFile, "r");
		if (!fp) {
			printf("Failed to load kernel\n");
		}

		fseek(fp, 0, SEEK_END);
		program_size = ftell(fp);
		rewind(fp);
		source_str = (char*)malloc(program_size + 1);
		source_str[program_size] = '\0';
		fread(source_str, sizeof(char), program_size, fp);
		fclose(fp);

		return source_str;

}

int main (int argc, const char * argv[]) {

	cl_int err;
	cl_uint numPlatforms;
	cl_device_id device;
	cl_device_id GPU;
	cl_context context;
	char name_plat[1024] = {0};
	char name_vendor[1024] = {0};
	char name_GPU[1024] = {0};
	size_t ret;
    cl_platform_id* platforms;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue cmd_queue;


	//*******************************************************************************************************
	// Get a NVIDIA GPU

    err = clGetPlatformIDs(0, NULL, &numPlatforms);

    if (CL_SUCCESS == err)
    		 printf("Detected OpenCL platforms: %d\n\n", numPlatforms);
    	else
    		 printf("\nError calling clGetPlatformIDs. Error code: %d", err);

    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms, NULL);

    for(int i = 0; i < numPlatforms; i++){

    	err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(name_plat), name_plat, &ret);

    	if (CL_SUCCESS == err)
    		printf("Platform name: %s\n", name_plat);
		else
			printf("\nError calling clGetPlatformInfo. Error code: %d", err);

    }

    for(int i = 0; i < numPlatforms; i++){

      	err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device, NULL);

      	if (CL_SUCCESS == err){

      		err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(name_vendor), name_vendor, NULL);

			if (CL_SUCCESS == err && !strcmp(name_vendor, "NVIDIA Corporation")){

				err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &GPU, NULL);
				err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name_GPU), name_GPU, NULL);

				if (CL_SUCCESS == err){

					printf("Name of the selected GPU: %s\n", name_GPU);

				}

			}

      	}

    }

    //*************************************************************************************************************

    // Creating the context

    context = clCreateContext(NULL, 1, &GPU, NULL, NULL, &err);

    if(err != CL_SUCCESS){

    	printf("\nError during the creation of the context: %d\n", err);

    }

    // Creating the command queue

    cmd_queue = clCreateCommandQueue(context, GPU, 0, &err);

    if(err != CL_SUCCESS){

    	printf("\nError during the creation of the command queue: %d\n", err);

    }

    char* src = NULL;

    src = read_kernel("C:/Users/Nelson/Documents/Etudes/Multi threading/Code/test_OpenCL/Kernels/crossCheckingSource.cl", src);

    // Get the program and compile it

	program = clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nError during the loading of the program: %d\n", err);

	}

	err = clBuildProgram(program, 1, &GPU, NULL, NULL, NULL);

	if(err != CL_SUCCESS){

		printf("\nUnable to build the program: %d\n", err);

	}

	cl_char log[3600];

	err = clGetProgramBuildInfo(program, GPU, CL_PROGRAM_BUILD_LOG, sizeof(log), &log, NULL);

	printf("log: %s\n", log);

	// Create the kernel

	kernel = clCreateKernel(program, "cross_checking", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	// Read the input images

	unsigned width;
	unsigned height;
	unsigned char* leftImage;
	unsigned char* rightImage;
	unsigned error;

	const char* leftImagePath = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth1.png";
	const char* rightImagePath = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth2.png";

	error = lodepng_decode32_file(&leftImage, &width, &height, leftImagePath);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&rightImage, &width, &height, rightImagePath);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem input_image_left, input_image_right, output_image;

	cl_image_format format;

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNSIGNED_INT8;

	input_image_left = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)leftImage, &err);
	input_image_right = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)rightImage, &err);
	output_image = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

//	int sizeWindow = 9;
//	int halfWindowSize = 4;
//	unsigned max_disp = 65;

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image_left);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_image_right);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_image);
//	err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &sizeWindow);
//	err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &halfWindowSize);
//	err |= clSetKernelArg(kernel, 5, sizeof(unsigned int), &max_disp);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	size_t global_work_size[2] = { width, height };
	size_t global_work_offset[2] = { 0, 0 };
	size_t local_work_size[2] = { 1, 1 };

	err = clEnqueueNDRangeKernel(cmd_queue, kernel, 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {width, height, 1};

	err = clEnqueueReadImage(cmd_queue, output_image, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/test_cross_checking.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	clReleaseMemObject(input_image_left);
	clReleaseMemObject(input_image_right);
	clReleaseMemObject(output_image);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);

	free(leftImage);
	free(rightImage);
	free(output);

    return 0;

}
