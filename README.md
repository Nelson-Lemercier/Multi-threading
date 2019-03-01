# Multi-threading
Multi-threading course project

# Single thread CPU
## Documentation

```transform_grey(path image input, path image output)```
For every pixel of the image, change the RGB values to 0.2126 x R + 0.7152 x G + 0.0722 x B.

```resize(path image input, path image output)```
Create an image four times smaller by taking every fourth pixel on width and height.

```ZNCC_right(path image left, path image right, path output image, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the right image.

```ZNCC_left(path image left, path image right, path output image, window size)```
Apply the ZNCC algorithm on the images, the disparity is calculate on the left image.

```cross_checking(path image left, path image right, path output image)```
Make a cross checking validation and gives one result image.

```occlusion_filling(path image input, path image output)```
Apply an occlusion filling algorithm. First, we define a window size of 3. If no pixel different from 0 is found, we add 2 to the window size and compute again.


## TODO

- [x] Resize function
- [x] Grey function
- [x] ZNCC algorithm 
- [x] Compute the disparity images for both input images
- [x] Implement the post-processing including cross-check and occlusion filling
- [x] Measure the total execution time of the implementation
- [x] Comment the code and make a documentation

**Impoved occlusion, but still some holes on the picture**