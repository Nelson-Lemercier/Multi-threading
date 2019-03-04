# Multi-threading
Multi-threading course project

# Single thread CPU
## Documentation

```createPath(const char* folder, const char* nameImage)```
Return the full path to the image folder with the image name and a PNG extension to read or write images in the folder.

```transform_grey(input image name, output image name)```
For every pixel of the image, change the RGB values to 0.2126 x R + 0.7152 x G + 0.0722 x B.

```transform_grey(input image name, output image name)```
For every pixel of the image, change the RGB values to 0.2126 x R + 0.7152 x G + 0.0722 x B.

```resize(input image name, output image name)```
Create an image four times smaller by taking every fourth pixel on width and height.

```ZNCC_right(left image name, right image name, output image name, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the right image.

```ZNCC_left(left image name, right image name, output image name, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the left image.

```cross_checking(left image name, right image name, output image name)```
Make a cross checking validation and gives one result image.

```occlusion_filling(input image name, output image name)```
Apply an occlusion filling algorithm. First, we define a window size of 3. If no pixel different from 0 is found, we add 2 to the window size and compute again.


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