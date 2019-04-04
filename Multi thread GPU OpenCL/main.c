#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <CL/cl.h>
#include <lodePNG.h>

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

    // Source code

    char* transform_grey_source = {

		"kernel void transform_grey(read_only image2d_t image, write_only image2d_t grey_image){\n"
    	"const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\n"
		"int2 coord = (int2)(get_global_id(0), get_global_id(1));\n"
		"uint r, g, b, a, grey_value = 0;\n"
		"uint4 values = read_imageui(image, smp, coord);\n"
		"r = values.x;\n"
    	"g = values.y;\n"
		"b = values.z;\n"
		"a = values.w;\n"
		"grey_value = (0.2126 * r + 0.7152 * g + 0.0722 * b);\n"
    	"values.x = grey_value;\n"
		"values.y = grey_value;\n"
		"values.z = grey_value;\n"
		"values.w = 255;\n"
		"write_imageui(grey_image, coord, values);\n"
		"}\n"

    };

    // Get the program and compile it

	program = clCreateProgramWithSource(context, 1, (const char**)&transform_grey_source, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nError during the loading of the program: %d\n", err);

	}

	err = clBuildProgram(program, 1, &GPU, NULL, NULL, NULL);

	if(err != CL_SUCCESS){

		printf("\nUnable to build the program: %d\n", err);

	}

	// Create the kernel

	kernel = clCreateKernel(program, "transform_grey", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	// Read the input image

	unsigned width;
	unsigned height;
	unsigned char* image;
	unsigned error;

	const char* input = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png";

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem input_image, output_image;

	cl_image_format format;

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNSIGNED_INT8;

	input_image = clCreateImage2D(context, CL_MEM_READ_ONLY, &format, (size_t)width, (size_t)height, 0, NULL, &err);
	output_image = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, (size_t)width, (size_t)height, 0, NULL, &err);
	
	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}
	
	// Write the input image 

    size_t origin[3] = {0, 0, 0};
	size_t region[3] = {width, height, 1};
	
	err = clEnqueueWriteImage(cmd_queue, input_image, CL_TRUE, origin, region, 0, 0, (void*)image, 0, 0, 0);

	// Set the kernel arguments

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_image);

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

	err = clEnqueueReadImage(cmd_queue, output_image, CL_TRUE, origin, region, 11760, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/test.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	clReleaseMemObject(input_image);
	clReleaseMemObject(output_image);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);

	free(image);
	free(output);

    return 0;

}
