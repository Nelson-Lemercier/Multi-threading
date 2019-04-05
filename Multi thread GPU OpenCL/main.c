#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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

    char* resize_source = {

		"kernel void resize(read_only image2d_t input_image, write_only image2d_t resize_image){\n"
		"const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\n"
		"int2 coord = (int2)(get_global_id(0), get_global_id(1));\n"
			"if( coord.x % 4 == 0 && coord.y % 4 == 0){\n"
			"uint4 values = read_imageui(input_image, smp, coord);\n"
			"coord.x /= 4;\n"
			"coord.y /= 4;\n;"
			"write_imageui(resize_image, coord, values);\n"
			"}\n"
    	"}\n"
    };

    char* ZNCC_left = {

		"kernel void ZNCC_left(read_only image2d_t image_left, read_only image2d_t image_right, write_only image2d_t output, unsigned int sizeWindow, unsigned int halfWindow, unsigned int max_disp){\n"
			"const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\n"
			"int2 coord = (int2)(get_global_id(0), get_global_id(1));\n"
    		"uint4 values = (uint4)(0, 0, 0, 0);\n"
    		"uint4 value_left, value_right;\n"
			"double maxSum = 0; unsigned int bestDisp = 0;\n"
			"for(int d = 0; d < max_disp; d++){\n"
				"unsigned int sum_left, sum_right = 0;\n"
				"for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){\n"
					"for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){\n"
						"int2 coordLeft = (int2)(i + d, j);\n"
						"int2 coordRight = (int2)(i, j);\n"
						"value_left = read_imageui(image_left, smp, coordLeft);\n"
						"value_right = read_imageui(image_right, smp, coordRight);\n"
						"sum_left += value_left.x;\n"
						"sum_right += value_right.x;\n"
					"}\n"
				"}\n"
				"double znccValue, upperSum, lowerSum_0, lowerSum_1, meanValueLeft, meanValueRight = 0;\n"
				"meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);\n"
				"meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);\n"
				"for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){\n"
					"for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){\n"
						"int2 coordLeft = (int2)(i + d, j);\n"
						"int2 coordRight = (int2)(i, j);\n"
						"value_left = read_imageui(image_left, smp, coordLeft);\n"
						"value_right = read_imageui(image_right, smp, coordRight);\n"
						"upperSum += (value_left.x - meanValueLeft) * (value_right.x - meanValueRight);\n"
						"lowerSum_0 += (value_left.x - meanValueLeft) * (value_left.x - meanValueLeft);\n"
						"lowerSum_1 += (value_right.x - meanValueRight) * (value_right.x - meanValueRight);\n"
					"}\n"
				"}\n"

				"znccValue = upperSum / (sqrt(lowerSum_0) * sqrt(lowerSum_1));\n"
				"if(znccValue > maxSum){\n"
					"maxSum = znccValue;\n"
					"bestDisp = d;\n"
				"}\n"
			"}\n"
    		"unsigned int depthValue = ceil(((double)255 / max_disp) * bestDisp);\n"
    		"values.x = depthValue;\n"
    		"values.y = depthValue;\n"
    		"values.z = depthValue;\n"
    		"values.w = 255;\n"
    		"write_imageui(output, coord, values);\n"
		"}\n"

    };

    // Get the program and compile it

	program = clCreateProgramWithSource(context, 1, (const char**)&ZNCC_left, NULL, &err);

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

	kernel = clCreateKernel(program, "ZNCC_left", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	// Read the input images

	unsigned width;
	unsigned height;
	unsigned char* leftImage;
	unsigned char* rightImage;
	unsigned error;

	const char* leftImagePath = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png";
	const char* rightImagePath = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png";

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

	unsigned int sizeWindow = 9;
	unsigned int max_disp = 65;
	unsigned int halfWindowSize = floor(sizeWindow / 2);

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image_left);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_image_right);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_image);
	err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &sizeWindow);
	err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &halfWindowSize);
	err |= clSetKernelArg(kernel, 5, sizeof(unsigned int), &max_disp);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	size_t global_work_size[2] = { width - 8 - max_disp, height - 8 };
	size_t global_work_offset[2] = { 4, 4 };
	size_t local_work_size[2] = { 1, 1 };

	err = clEnqueueNDRangeKernel(cmd_queue, kernel, 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {width - 8, height - 8, 1};

	err = clEnqueueReadImage(cmd_queue, output_image, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	unsigned width2 = width - 8;
	unsigned height2 = height - 8;

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/test_depth.png", output, width2, height2);
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
