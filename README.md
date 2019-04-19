# Multi-threading
Multi-threading course project

# Documentation
## Functions

```createPath(const char* folder, const char* nameImage)```
Return the full path to the image folder with the image name and a PNG extension to read or write images in the folder.

```transform_grey(imageFolder, input image name, output image name)```
For every pixel of the image, change the RGB values to 0.2126 x R + 0.7152 x G + 0.0722 x B.

```transform_grey(imageFolder, input image name, output image name)```
For every pixel of the image, change the RGB values to 0.2126 x R + 0.7152 x G + 0.0722 x B.

```resize(imageFolder, input image name, output image name)```
Create an image four times smaller by taking every fourth pixel on width and height.

```ZNCC_right(imageFolder, left image name, right image name, output image name, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the right image.

```ZNCC_left(imageFolder, left image name, right image name, output image name, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the left image.

```cross_checking(imageFolder, left image name, right image name, output image name)```
Make a cross checking validation and gives one result image.

```occlusion_filling(imageFolder, input image name, output image name)```
Apply an occlusion filling algorithm. First, we define a window size of 3. If no pixel different from 0 is found, we add 2 to the window size and compute again.

## Kernels

```kernel void transform_grey(read_only image2d_t image, write_only image2d_t grey_image)```

```kernel void resize(read_only image2d_t input_image, write_only image2d_t resize_image)```

```kernel void ZNCC_left(read_only image2d_t image_left, read_only image2d_t image_right, write_only image2d_t output, unsigned int sizeWindow, unsigned int halfWindow, unsigned int max_disp)```

```kernel void ZNCC_right(read_only image2d_t image_left, read_only image2d_t image_right, write_only image2d_t output, unsigned int sizeWindow, unsigned int halfWindow, unsigned int max_disp)```

```kernel void cross_checking(read_only image2d_t depth_left, read_only image2d_t depth_right, write_only image2d_t cross_checking)```

# Configuration

- **CPU**:  Intel(R) Core(TM) i5-8250U CPU @1.60GHz 1.80GHz 
            4 physical cores / 8 logic cores

- **GPU**:  NVIDIA GeForce MX150

# Specification GPU

- local_mem_size: 49152
- max_compute_units: 3
- max_clock_frequency: 1531
- max_constant_buffer_size: 65536
- max_work_group_size: 1024
- max_work_item_size	
    - x: 1024 
    - y: 1024 
    - z: 64

# Single thread CPU
## TODO

- [x] Resize function
- [x] Grey function
- [x] ZNCC algorithm 
- [x] Compute the disparity images forinput  both images
- [x] Implement the post-processing including cross-check and occlusion filling
- [x] Measure the total execution time of the implementation
- [x] Comment the code and make a documentation

## Time of execution

Average on 5 trials: 193,485321 seconds

# Multi thread CPU

## TODO

- [x] CPU parallelization and multithreading
- [x] Implementation of the OpenCL kernels

## Time of execution

Average on 5 trials: 42.154733 seconds

# Multi thread GPU OpenCL

## TODO

**Kernels to modify**

- [x] Transform grey
- [x] Resize
- [x] ZNCC left
- [x] ZNCC right **Return a similar image but a too bright**
- [x] cross checking
- [ ] occlusion filling
________________________________________________________________________________________

**Secondary tasks**

- [ ] Comments and documentation
- [ ] Refactoring
- [x] Implement a function to read kernels' source code from files

________________________________________________________________________________________

**General tasks**

- [ ] Implement the algorithm using OpenCL and a GPU
- [ ] Start the optimization of the OpenCL kernels required in the next phase