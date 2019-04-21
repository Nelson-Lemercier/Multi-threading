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
	cl_kernel kernel[6];
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

    // Get the source code

    char* sourceCode = NULL;

    sourceCode = read_kernel("C:/Users/Nelson/Documents/Etudes/Multi threading/Code/test_OpenCL/Kernels/kernels.cl", sourceCode);

    // Get the program and compile it

	program = clCreateProgramWithSource(context, 1, (const char**)&sourceCode, NULL, &err);

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

	// Create the kernels

	kernel[0] = clCreateKernel(program, "transform_grey", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	kernel[1] = clCreateKernel(program, "resize", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	kernel[2] = clCreateKernel(program, "ZNCC_left", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	kernel[3] = clCreateKernel(program, "ZNCC_right", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	kernel[4] = clCreateKernel(program, "cross_checking", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	kernel[5] = clCreateKernel(program, "occlusion", &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the kernel: %d\n", err);

	}

	//******************************************************************************************************
	// Transform grey 0

	// Read the input images

	unsigned width;
	unsigned height;
	unsigned char* image;
	unsigned error;

	const char* path = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png";

	error = lodepng_decode32_file(&image, &width, &height, path);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem originalImage0, greyImage0;

	cl_image_format format;

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNSIGNED_INT8;

	originalImage0 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)image, &err);
	greyImage0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &originalImage0);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &greyImage0);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	size_t global_work_size[2] = { width, height };
	size_t global_work_offset[2] = { 0, 0 };
	size_t local_work_size[2] = { 1, 1 };

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {width, height, 1};

	err = clEnqueueReadImage(cmd_queue, greyImage0, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey0.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// transform grey 1

	path = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im1.png";

	error = lodepng_decode32_file(&image, &width, &height, path);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem originalImage1, greyImage1;

	originalImage1 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)image, &err);
	greyImage1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &originalImage1);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &greyImage1);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, greyImage1, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey1.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// Resize 0

	unsigned widthResize = width / 4;
	unsigned heightResize = height /4;

	output = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);

	// Create the input and output buffers

	cl_mem resize0;

	resize0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &greyImage0);
	err |= clSetKernelArg(kernel[1], 1, sizeof(cl_mem), &resize0);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[1], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, resize0, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/resize0.png", output, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// Resize1

	output = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);

	// Create the input and output buffers

	cl_mem resize1;

	resize1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &greyImage1);
	err |= clSetKernelArg(kernel[1], 1, sizeof(cl_mem), &resize1);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[1], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, resize1, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/resize1.png", output, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// ZNCC0

	width = widthResize;
	height = heightResize;

	output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem zncc0;

	zncc0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	int windowSize = 9;
	int halfWindow = floor(windowSize / 3);
	unsigned int maxDisp = 64;

	err = clSetKernelArg(kernel[2], 0, sizeof(cl_mem), &resize0);
	err |= clSetKernelArg(kernel[2], 1, sizeof(cl_mem), &resize1);
	err |= clSetKernelArg(kernel[2], 2, sizeof(cl_mem), &zncc0);
	err |= clSetKernelArg(kernel[2], 3, sizeof(cl_mem), &windowSize);
	err |= clSetKernelArg(kernel[2], 4, sizeof(cl_mem), &halfWindow);
	err |= clSetKernelArg(kernel[2], 5, sizeof(cl_mem), &maxDisp);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width - 8 - maxDisp; global_work_size[1] = height - 8;
	global_work_offset[0] = 4; global_work_offset[1] = 4;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[2], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width - 8; region[1] = height - 8; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, zncc0, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/zncc0.png", output, width - 8, height - 8);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// ZNCC1

	output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem zncc1;

	zncc1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[3], 0, sizeof(cl_mem), &resize0);
	err |= clSetKernelArg(kernel[3], 1, sizeof(cl_mem), &resize1);
	err |= clSetKernelArg(kernel[3], 2, sizeof(cl_mem), &zncc1);
	err |= clSetKernelArg(kernel[3], 3, sizeof(cl_mem), &windowSize);
	err |= clSetKernelArg(kernel[3], 4, sizeof(cl_mem), &halfWindow);
	err |= clSetKernelArg(kernel[3], 5, sizeof(cl_mem), &maxDisp);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width - 8 - maxDisp; global_work_size[1] = height - 8;
	global_work_offset[0] = 4 + maxDisp; global_work_offset[1] = 4;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[3], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width - 8; region[1] = height - 8; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, zncc1, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/zncc1.png", output, width - 8, height - 8);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	//Cross checking

	output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem crossChecked;

	crossChecked = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[4], 0, sizeof(cl_mem), &zncc0);
	err |= clSetKernelArg(kernel[4], 1, sizeof(cl_mem), &zncc1);
	err |= clSetKernelArg(kernel[4], 2, sizeof(cl_mem), &crossChecked);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[4], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, crossChecked, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/crossChecked.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************
	// Occlusion

	output = malloc(sizeof(unsigned char) * width * height * 4);

	// Create the input and output buffers

	cl_mem occlusion;

	occlusion = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, (size_t)width, (size_t)height, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nUnable to create the images objects: %d\n", err);

	}

	// Set the kernel arguments

	err = clSetKernelArg(kernel[5], 0, sizeof(cl_mem), &crossChecked);
	err |= clSetKernelArg(kernel[5], 1, sizeof(cl_mem), &occlusion);
	err |= clSetKernelArg(kernel[5], 2, sizeof(cl_mem), &width);
	err |= clSetKernelArg(kernel[5], 3, sizeof(cl_mem), &height);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	// Enqueue the kernel

	global_work_size[0] = width - 1; global_work_size[1] = height - 1;
	global_work_offset[0] = 1; global_work_offset[1] = 1;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[5], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	//Read the result

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, occlusion, CL_TRUE, origin, region, 0, 0, output, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	// Encode the result in a output image

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/occlusion.png", output, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(output);

	//******************************************************************************************************

	clReleaseMemObject(originalImage0);
	clReleaseMemObject(originalImage1);
	clReleaseMemObject(greyImage0);
	clReleaseMemObject(greyImage1);
	clReleaseMemObject(resize0);
	clReleaseMemObject(resize1);
	clReleaseMemObject(zncc0);
	clReleaseMemObject(zncc1);
	clReleaseMemObject(crossChecked);
	clReleaseMemObject(occlusion);
	clReleaseKernel(kernel[0]);
	clReleaseKernel(kernel[1]);
	clReleaseKernel(kernel[2]);
	clReleaseKernel(kernel[3]);
	clReleaseKernel(kernel[4]);
	clReleaseKernel(kernel[5]);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);

    return 0;

}
