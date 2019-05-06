#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <CL/cl.h>
#include <lodePNG.h>
#include <Windows.h>

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

	LARGE_INTEGER clockFrequency;
	QueryPerformanceFrequency(&clockFrequency);

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;

	LARGE_INTEGER startGPU;
	LARGE_INTEGER endGPU;

	LARGE_INTEGER startOverhead;
	LARGE_INTEGER endOverhead;

	LARGE_INTEGER startCleanUp;
	LARGE_INTEGER endCleanUp;

	LARGE_INTEGER startWrite;
	LARGE_INTEGER endWrite;
	LARGE_INTEGER startKernels;
	LARGE_INTEGER endKernels;
	LARGE_INTEGER startRead;
	LARGE_INTEGER endRead;

	LARGE_INTEGER startKernel0;
	LARGE_INTEGER endKernel0;
	LARGE_INTEGER startKernel1;
	LARGE_INTEGER endKernel1;
	LARGE_INTEGER startKernel2;
	LARGE_INTEGER endKernel2;
	LARGE_INTEGER startKernel3;
	LARGE_INTEGER endKernel3;
	LARGE_INTEGER startKernel4;
	LARGE_INTEGER endKernel4;
	LARGE_INTEGER startKernel5;
	LARGE_INTEGER endKernel5;

	QueryPerformanceCounter(&startTime);

	QueryPerformanceCounter(&startOverhead);

	/********************************************/
	/*                                          */
	/*            SELECT THE GPU                */
	/*                                          */
	/********************************************/

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

	/********************************************/
	/*                                          */
	/* CREATE THE CONTEXT AND THE COMMAND QUEUE */
	/*                                          */
	/********************************************/

    context = clCreateContext(NULL, 1, &GPU, NULL, NULL, &err);

    if(err != CL_SUCCESS){

    	printf("\nError during the creation of the context: %d\n", err);

    }

    cmd_queue = clCreateCommandQueue(context, GPU, 0, &err);

    if(err != CL_SUCCESS){

    	printf("\nError during the creation of the command queue: %d\n", err);

    }

	/********************************************/
	/*                                          */
	/*   GET THE SOURCE CODE AND COMPILE IT     */
	/*                                          */
	/********************************************/

    char* sourceCode = NULL;

    sourceCode = read_kernel("C:/Users/Nelson/Documents/Etudes/Multi threading/Code/test_OpenCL/Kernels/kernels.cl", sourceCode);

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


	/********************************************/
	/*                                          */
	/*            CREATE THE KERNELS            */
	/*                                          */
	/********************************************/


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

	/********************************************/
	/*                                          */
	/*     CREATE OUTPUT IMAGES POINTERS        */
	/*                                          */
	/********************************************/

	unsigned width;
	unsigned height;
	unsigned char* originalImage0;
	unsigned char* originalImage1;
	unsigned error;

	const char* path = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png";

	error = lodepng_decode32_file(&originalImage0, &width, &height, path);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* greyImage0 = malloc(sizeof(unsigned char) * width * height * 4);

	path = "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im1.png";

	error = lodepng_decode32_file(&originalImage1, &width, &height, path);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* greyImage1 = malloc(sizeof(unsigned char) * width * height * 4);

	unsigned widthResize = width / 4;
	unsigned heightResize = height /4;

	unsigned char* resizeImage0 = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);
	unsigned char* resizeImage1 = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);

	unsigned char* znccImage0 = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);
	unsigned char* znccImage1 = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);

	unsigned char* crossCheckedImage = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);
	unsigned char* occlusionImage = malloc(sizeof(unsigned char) * widthResize * heightResize * 4);

	QueryPerformanceCounter(&endOverhead);

	/********************************************/
	/*                                          */
	/*        CREATE THE IMAGE OBJECTS          */
	/*                                          */
	/********************************************/

	QueryPerformanceCounter(&startGPU);
	QueryPerformanceCounter(&startWrite);

	cl_mem original0, grey0, original1, grey1, resize0, resize1, zncc0 , zncc1, crossChecked, occlusion;

	cl_image_format format;

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNSIGNED_INT8;

	original0 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)originalImage0, &err);
	original1 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format, (size_t)width, (size_t)height, 0, (void*)originalImage1, &err);
	grey0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);
	grey1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)width, (size_t)height, 0, NULL, &err);
	resize0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);
	resize1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);
	zncc0 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);
	zncc1 = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);
	crossChecked = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);
	occlusion = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, (size_t)widthResize, (size_t)heightResize, 0, NULL, &err);

	if(err != CL_SUCCESS){

		printf("\nError clCreateImage2D: %d\n", err); // Effective only on the last image, have to find a fix

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endWrite);

	/********************************************/
	/*                                          */
	/*            EXECUTE THE KERNELS           */
	/*                                          */
	/********************************************/

	QueryPerformanceCounter(&startKernels);

	QueryPerformanceCounter(&startKernel0);

	//******************************************************************************************************
	// Transform grey 0

	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &original0);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &grey0);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	size_t global_work_size[2] = { width, height };
	size_t global_work_offset[2] = { 0, 0 };
	size_t local_work_size[2] = { 1, 1 };

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel0);

	//******************************************************************************************************
	// Transform grey 1

	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &original1);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &grey1);


	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&startKernel1);

	//******************************************************************************************************
	// Resize 0

	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &grey0);
	err |= clSetKernelArg(kernel[1], 1, sizeof(cl_mem), &resize0);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[1], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel1);

	//******************************************************************************************************
	// Resize 1

	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &grey1);
	err |= clSetKernelArg(kernel[1], 1, sizeof(cl_mem), &resize1);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = width; global_work_size[1] = height;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[1], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&startKernel2);

	//******************************************************************************************************
	// ZNCC 0

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

	global_work_size[0] = widthResize - 8 - maxDisp - 3; global_work_size[1] = heightResize - 8;
	global_work_offset[0] = 4; global_work_offset[1] = 4;
	local_work_size[0] = 33; local_work_size[1] = 31;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[2], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel2);

	QueryPerformanceCounter(&startKernel3);

	//******************************************************************************************************
	// ZNCC 1

	err = clSetKernelArg(kernel[3], 0, sizeof(cl_mem), &resize0);
	err |= clSetKernelArg(kernel[3], 1, sizeof(cl_mem), &resize1);
	err |= clSetKernelArg(kernel[3], 2, sizeof(cl_mem), &zncc1);
	err |= clSetKernelArg(kernel[3], 3, sizeof(cl_mem), &windowSize);
	err |= clSetKernelArg(kernel[3], 4, sizeof(cl_mem), &halfWindow);
	err |= clSetKernelArg(kernel[3], 5, sizeof(cl_mem), &maxDisp);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = widthResize - 8 - maxDisp - 3; global_work_size[1] = heightResize - 8;
	global_work_offset[0] = 4 + maxDisp; global_work_offset[1] = 4;
	local_work_size[0] = 33; local_work_size[1] = 31;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[3], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel3);

	QueryPerformanceCounter(&startKernel4);

	//******************************************************************************************************
	// Cross checking

	err = clSetKernelArg(kernel[4], 0, sizeof(cl_mem), &zncc0);
	err |= clSetKernelArg(kernel[4], 1, sizeof(cl_mem), &zncc1);
	err |= clSetKernelArg(kernel[4], 2, sizeof(cl_mem), &crossChecked);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = widthResize; global_work_size[1] = heightResize;
	global_work_offset[0] = 0; global_work_offset[1] = 0;
	local_work_size[0] = 1; local_work_size[1] = 1;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[4], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel4);

	QueryPerformanceCounter(&startKernel5);

	//******************************************************************************************************
	// Occlusion

	err = clSetKernelArg(kernel[5], 0, sizeof(cl_mem), &crossChecked);
	err |= clSetKernelArg(kernel[5], 1, sizeof(cl_mem), &occlusion);
	err |= clSetKernelArg(kernel[5], 2, sizeof(cl_mem), &width);
	err |= clSetKernelArg(kernel[5], 3, sizeof(cl_mem), &height);

	if(err != CL_SUCCESS){

		printf("\nError clSetKernelArg: %d\n", err);

	}

	global_work_size[0] = widthResize - 1 - 6; global_work_size[1] = heightResize - 1 - 3;
	global_work_offset[0] = 1; global_work_offset[1] = 1;
	local_work_size[0] = 28; local_work_size[1] = 25;

	err = clEnqueueNDRangeKernel(cmd_queue, kernel[5], 2, global_work_offset, global_work_size, local_work_size, 0, 0, 0);

	if(err != CL_SUCCESS){

		printf("\nError clEnqueueNDRangeKernel: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endKernel5);

	QueryPerformanceCounter(&endKernels);

	/********************************************/
	/*                                          */
	/*         RETRIEVE THE RESULTS             */
	/*                                          */
	/********************************************/

	QueryPerformanceCounter(&startRead);

	//******************************************************************************************************
	// Transform grey 0

	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {width, height, 1};

	err = clEnqueueReadImage(cmd_queue, grey0, CL_FALSE, origin, region, 0, 0, greyImage0, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// transform grey 1

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, grey1, CL_FALSE, origin, region, 0, 0, greyImage1, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// Resize 0

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, resize0, CL_FALSE, origin, region, 0, 0, resizeImage0, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// Resize1

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, resize1, CL_FALSE, origin, region, 0, 0, resizeImage1, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// ZNCC0

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize - 8; region[1] = heightResize - 8; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, zncc0, CL_FALSE, origin, region, 0, 0, znccImage0, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// ZNCC1

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize - 8; region[1] = heightResize - 8; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, zncc1, CL_FALSE, origin, region, 0, 0, znccImage1, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	//Cross checking

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, crossChecked, CL_FALSE, origin, region, 0, 0, crossCheckedImage, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	//******************************************************************************************************
	// Occlusion

	origin[0] = origin[1] = origin[2] = 0;
	region[0] = widthResize; region[1] = heightResize; region[2] = 1;

	err = clEnqueueReadImage(cmd_queue, occlusion, CL_FALSE, origin, region, 0, 0, occlusionImage, 0, 0, 0);
	if(err != CL_SUCCESS){

		printf("\nError clEnqueueReadImage: %d\n", err);

	}

	clFinish(cmd_queue);

	QueryPerformanceCounter(&endRead);

	QueryPerformanceCounter(&endGPU);


	/********************************************/
	/*                                          */
	/*            ENCODE THE RESULTS            */
	/*                                          */
	/********************************************/


	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey0.png", greyImage0, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey1.png", greyImage1, width, height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/resize0.png", resizeImage0, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/resize1.png", resizeImage1, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/zncc0.png", znccImage0, widthResize - 8, heightResize - 8);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/zncc1.png", znccImage1, widthResize - 8, heightResize - 8);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/crossChecked.png", crossCheckedImage, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_encode32_file("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/occlusion.png", occlusionImage, widthResize, heightResize);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	/********************************************/
	/*                                          */
	/*  FREE THE POINTERS AND THE IMAGE OBJECTS */
	/*                                          */
	/********************************************/

	QueryPerformanceCounter(&startCleanUp);

	free(greyImage0);
	free(greyImage1);
	free(resizeImage0);
	free(resizeImage1);
	free(znccImage0);
	free(znccImage1);
	free(crossCheckedImage);
	free(occlusionImage);

	clReleaseMemObject(original0);
	clReleaseMemObject(original1);
	clReleaseMemObject(grey0);
	clReleaseMemObject(grey1);
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

	QueryPerformanceCounter(&endCleanUp);

	QueryPerformanceCounter(&endTime);

	LARGE_INTEGER delta;
	LARGE_INTEGER deltaGPU;
	LARGE_INTEGER deltaOverhead;
	LARGE_INTEGER deltaCleanUp;

	LARGE_INTEGER deltaWrite;
	LARGE_INTEGER deltaKernels;
	LARGE_INTEGER deltaRead;

	LARGE_INTEGER deltaKernel0;
	LARGE_INTEGER deltaKernel1;
	LARGE_INTEGER deltaKernel2;
	LARGE_INTEGER deltaKernel3;
	LARGE_INTEGER deltaKernel4;
	LARGE_INTEGER deltaKernel5;

	delta.QuadPart = endTime.QuadPart - startTime.QuadPart;
	float deltaSeconds = (float)delta.QuadPart / clockFrequency.QuadPart;

	deltaGPU.QuadPart = endGPU.QuadPart - startGPU.QuadPart;
	float deltaGPUSeconds = (float)deltaGPU.QuadPart / clockFrequency.QuadPart;

	deltaOverhead.QuadPart = endOverhead.QuadPart - startOverhead.QuadPart;
	float deltaOverheadSeconds = (float)deltaOverhead.QuadPart / clockFrequency.QuadPart;

	deltaCleanUp.QuadPart = endCleanUp.QuadPart - startCleanUp.QuadPart;
	float deltaCleanUpSeconds = (float)deltaCleanUp.QuadPart / clockFrequency.QuadPart;

	deltaWrite.QuadPart = endWrite.QuadPart - startWrite.QuadPart;
	float deltaWriteSeconds = (float)deltaWrite.QuadPart / clockFrequency.QuadPart;

	deltaKernels.QuadPart = endKernels.QuadPart - startKernels.QuadPart;
	float deltaKernelsSeconds = (float)deltaKernels.QuadPart / clockFrequency.QuadPart;

	deltaRead.QuadPart = endRead.QuadPart - startRead.QuadPart;
	float deltaReadSeconds = (float)deltaRead.QuadPart / clockFrequency.QuadPart;

	deltaKernel0.QuadPart = endKernel0.QuadPart - startKernel0.QuadPart;
	float deltaKernel0Seconds = (float)deltaKernel0.QuadPart / clockFrequency.QuadPart;

	deltaKernel1.QuadPart = endKernel1.QuadPart - startKernel1.QuadPart;
	float deltaKernel1Seconds = (float)deltaKernel1.QuadPart / clockFrequency.QuadPart;

	deltaKernel2.QuadPart = endKernel2.QuadPart - startKernel2.QuadPart;
	float deltaKernel2Seconds = (float)deltaKernel2.QuadPart / clockFrequency.QuadPart;

	deltaKernel3.QuadPart = endKernel3.QuadPart - startKernel3.QuadPart;
	float deltaKernel3Seconds = (float)deltaKernel3.QuadPart / clockFrequency.QuadPart;

	deltaKernel4.QuadPart = endKernel4.QuadPart - startKernel4.QuadPart;
	float deltaKernel4Seconds = (float)deltaKernel4.QuadPart / clockFrequency.QuadPart;

	deltaKernel5.QuadPart = endKernel5.QuadPart - startKernel5.QuadPart;
	float deltaKernel5Seconds = (float)deltaKernel5.QuadPart / clockFrequency.QuadPart;

	printf("Execution time:\t%f s\nOverhead execution time:\t%f s\nGPU execution time:\t%f s\nWrite execution time:\t%f s\nKernels execution time:\t%f s\nRead execution time:\t%f s\nCleanUp execution time:\t%f s\n\n", deltaSeconds, deltaOverheadSeconds, deltaGPUSeconds, deltaWriteSeconds, deltaKernelsSeconds, deltaReadSeconds, deltaCleanUpSeconds);

	printf("Kernel0:\t%f s\nKernel1:\t%f s\nKernel2:\t%f s\nKernel3:\t%f s\nKernel4:\t%f s\nKernel5:\t%f s\n", deltaKernel0Seconds, deltaKernel1Seconds, deltaKernel2Seconds, deltaKernel3Seconds, deltaKernel4Seconds, deltaKernel5Seconds);

    return 0;

}
