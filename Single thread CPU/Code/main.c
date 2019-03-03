#include <lodePNG.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <math.h>
#include <Windows.h>

int show(const char* filename) { // Display the image using SDL library. Exemple from LodePNG library.
  unsigned error;
  unsigned char* image;
  unsigned w, h, x, y;
  SDL_Surface* scr;
  SDL_Event event;
  int done;
  size_t jump = 1;

  printf("showing %s\n", filename);

  error = lodepng_decode32_file(&image, &w, &h, filename);

  if(error) {
    printf("decoder error %u: %s\n", error, lodepng_error_text(error));
    return 0;
  }

  if(w / 1024 >= jump) jump = w / 1024 + 1;
  if(h / 1024 >= jump) jump = h / 1024 + 1;

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Error, SDL video init failed\n");
    return 0;
  }
  scr = SDL_SetVideoMode(w / jump, h / jump, 32, SDL_HWSURFACE);
  if(!scr) {
    printf("Error, no SDL screen\n");
    return 0;
  }
  SDL_WM_SetCaption(filename, NULL);

  for(y = 0; y + jump - 1 < h; y += jump)
  for(x = 0; x + jump - 1 < w; x += jump) {
    int checkerColor;
    Uint32* bufp;
    Uint32 r, g, b, a;

    r = image[4 * y * w + 4 * x + 0];
    g = image[4 * y * w + 4 * x + 1];
    b = image[4 * y * w + 4 * x + 2];
    a = image[4 * y * w + 4 * x + 3];

    checkerColor = 191 + 64 * (((x / 16) % 2) == ((y / 16) % 2));
    r = (a * r + (255 - a) * checkerColor) / 255;
    g = (a * g + (255 - a) * checkerColor) / 255;
    b = (a * b + (255 - a) * checkerColor) / 255;

    bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) / jump + (x / jump);
    *bufp = 65536 * r + 256 * g + b;
  }

  done = 0;
  while(done == 0) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) done = 2;
      else if(SDL_GetKeyState(NULL)[SDLK_ESCAPE]) done = 2;
      else if(event.type == SDL_KEYDOWN) done = 1;
    }
    SDL_UpdateRect(scr, 0, 0, 0, 0);
    SDL_Delay(5);
  }

  free(image);
  SDL_Quit();
  return done == 2 ? 1 : 0;

}

void transform_grey(const char* input, const char* output){ //Transform a RGBA image to a grey image (still in RGBA format)

	// Initialisation of the variables and decoding of the input image

	unsigned width;
	unsigned height;

	unsigned char* image;
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* grey_image = malloc(width * height * 4); // Declaration of the new image, and allocate memory.
	unsigned x, y;

	Uint32 r, g, b, a, grey_value;

	// For each pixel of the image ...

	for(y = 0; y < height; y++){

		for(x = 0; x < width; x++) {

			r = image[4 * width * y + 4 * x + 0];
			g = image[4 * width * y + 4 * x + 1];
			b = image[4 * width * y + 4 * x + 2];
			a = image[4 * width * y + 4 * x + 3];

			grey_value = (0.2126 * r + 0.7152 * g + 0.0722 * b); // The grey value is a combination of the RGB values (We ake mostly the green component).

			// Save the same value for R, G and B in the new image

			grey_image[4 * width * y + 4 * x + 0] = grey_value;
			grey_image[4 * width * y + 4 * x + 1] = grey_value;
			grey_image[4 * width * y + 4 * x + 2] = grey_value;
			grey_image[4 * width * y + 4 * x + 3] = a;

		}

	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, grey_image, width, height);

	  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	  free(grey_image);

}

void resize(const char* input, const char* output){ // Resize the image by taking every fourth pixel

	// Initialisation of the variables and decoding of the input image

	unsigned width;
	unsigned height;

	unsigned char* image;
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	// The output image will be 4 times smaller

	unsigned width_redux = width / 4;
	unsigned height_redux = height / 4;

	unsigned char* image_redux = malloc(width_redux * height_redux * 4); // Declaration of the new image, and allocate memory.
	unsigned x, y, k, l;

	// Indexes to save the values in the output image

	k = 0;
	l = 0;

	Uint32 r, g, b, a;

	// For each fourth pixel, save the values in the output image at indexes k and l

	for(y = 0; y < height; y+=4){

		for(x = 0; x < width; x+=4) {

			r = image[4 * width * y + 4 * x + 0];
			g = image[4 * width * y + 4 * x + 1];
			b = image[4 * width * y + 4 * x + 2];
			a = image[4 * width * y + 4 * x + 3];

			image_redux[4 * width_redux * l + 4 * k + 0] = r;
			image_redux[4 * width_redux * l + 4 * k + 1] = g;
			image_redux[4 * width_redux * l + 4 * k + 2] = b;
			image_redux[4 * width_redux * l + 4 * k + 3] = a;

			k++;

		}

		k = 0;
		l++;

	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, image_redux, width_redux, height_redux);

	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	 free(image_redux);

}

void ZNCC_right(const char* input_left, const char* input_right, const char* output, int sizeWindow){ // Apply the ZNCC algorithm on the right image (the disparity will be applied on the right image)

	// Initialisation of the variables and decoding of the input images

	unsigned width;
	unsigned height;

	unsigned char* image_left;
	unsigned char* image_right;
	unsigned error;

	error = lodepng_decode32_file(&image_left, &width, &height, input_left);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&image_right, &width, &height, input_right);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	Uint32 value_left, value_right, sum_left, sum_right, bestDisp; // Declaration of the integers used in the algorithm

	double znccValue, maxSum, upperSum, lowerSum_0, lowerSum_1, meanValueLeft, meanValueRight; // Declaration of the floats used in the algorithm

	unsigned max_disp = 65; // The suggested maximum disparity is 260, the image is 4 times smaller

	int halfWindowSize = floor(sizeWindow / 2);

	unsigned char* depthImage = malloc(width * height * 4); // Declaration of the output image and memory allocation

	// For each pixel of the image, considering borders for the window, apply the ZNCC algorithm

	for(int h = halfWindowSize; h < height - halfWindowSize; h++){

		for(int w = halfWindowSize + max_disp; w < width - halfWindowSize; w++){

			maxSum = 0; bestDisp = 0;

			for(int d = 0; d < max_disp; d++){

				sum_left = 0; sum_right = 0; meanValueLeft = 0; meanValueRight = 0;

				for(int j = h - halfWindowSize; j < h + halfWindowSize; j++){

					for(int i = w - halfWindowSize; i < w + halfWindowSize; i++){

						value_left = image_left[4 * width * j + 4 * i ];
						value_right = image_right[4 * width * j + 4 * ( i - d )];

						sum_left += value_left;
						sum_right += value_right;

					}

				}

				meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);
				meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);

				upperSum = 0; lowerSum_0 = 0; lowerSum_1 = 0;

				for(int j = h - halfWindowSize; j < h + halfWindowSize; j++){

					for(int i = w - halfWindowSize; i < w + halfWindowSize; i++){

						value_left = image_left[4 * width * j + 4 * i ];
						value_right = image_right[4 * width * j + 4 * ( i - d )];

						upperSum += (value_left - meanValueLeft) * (value_right - meanValueRight);
						lowerSum_0 += (value_left - meanValueLeft) * (value_left - meanValueLeft);
						lowerSum_1 += (value_right - meanValueRight) * (value_right - meanValueRight);

					}

				}

				znccValue = upperSum / (sqrt(lowerSum_0) * sqrt(lowerSum_1));

				if(znccValue > maxSum){

					maxSum = znccValue;
					bestDisp = d;

				}

			}


			int depthValue = ceil(((double)255 / max_disp) * bestDisp); // Normalization of the result

			// Saving the result in the ouput image

			 depthImage[4 * width * h + 4 * w + 0] = depthValue;
			 depthImage[4 * width * h + 4 * w + 1] = depthValue;
			 depthImage[4 * width * h + 4 * w + 2] = depthValue;
			 depthImage[4 * width * h + 4 * w + 3] = 255;

		}

	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, depthImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(depthImage);

}

void ZNCC_left(const char* input_left, const char* input_right, const char* output, int sizeWindow){ // Apply the ZNCC algorithm on the left image (the disparity will be applied on the left image)

	// Initialisation of the variables and decoding of the input images

	unsigned width;
	unsigned height;

	unsigned char* image_left;
	unsigned char* image_right;
	unsigned error;

	error = lodepng_decode32_file(&image_left, &width, &height, input_left);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&image_right, &width, &height, input_right);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	Uint32 value_left, value_right, sum_left, sum_right, bestDisp; // Declaration of the integers used in the algorithm

	double znccValue, maxSum, upperSum, lowerSum_0, lowerSum_1, meanValueLeft, meanValueRight; // Declaration of the floats used in the algorithm

	unsigned max_disp = 65; // The suggested maximum disparity is 260, the image is 4 times smaller

	int halfWindowSize = floor(sizeWindow / 2);

	unsigned char* depthImage = malloc(width * height * 4); // Declaration of the output image and memory allocation

	// For each pixel of the image, considering borders for the window, apply the ZNCC algorithm

	for(int h = halfWindowSize; h < height - halfWindowSize; h++){

		for(int w = halfWindowSize; w < width - halfWindowSize - max_disp; w++){

			maxSum = 0; bestDisp = 0;

			for(int d = 0; d < max_disp; d++){

				sum_left = 0; sum_right = 0; meanValueLeft = 0; meanValueRight = 0;

				for(int j = h - halfWindowSize; j < h + halfWindowSize; j++){

					for(int i = w - halfWindowSize; i < w + halfWindowSize; i++){

						value_left = image_left[4 * width * j + 4 * ( i + d ) ];
						value_right = image_right[4 * width * j + 4 * i];

						sum_left += value_left;
						sum_right += value_right;

					}

				}

				meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);
				meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);

				upperSum = 0; lowerSum_0 = 0; lowerSum_1 = 0;

				for(int j = h - halfWindowSize; j < h + halfWindowSize; j++){

					for(int i = w - halfWindowSize; i < w + halfWindowSize; i++){

						value_left = image_left[4 * width * j + 4 * (i + d) ];
						value_right = image_right[4 * width * j + 4 * i ];

						upperSum += (value_left - meanValueLeft) * (value_right - meanValueRight);
						lowerSum_0 += (value_left - meanValueLeft) * (value_left - meanValueLeft);
						lowerSum_1 += (value_right - meanValueRight) * (value_right - meanValueRight);

					}

				}

				znccValue = upperSum / (sqrt(lowerSum_0) * sqrt(lowerSum_1));

				if(znccValue > maxSum){

					maxSum = znccValue;
					bestDisp = d;

				}

			}

			int depthValue = ceil(((double)255 / max_disp) * bestDisp); // Normalization of the result

			// Saving the result in the ouput image

			 depthImage[4 * width * h + 4 * w + 0] = depthValue;
			 depthImage[4 * width * h + 4 * w + 1] = depthValue;
			 depthImage[4 * width * h + 4 * w + 2] = depthValue;
			 depthImage[4 * width * h + 4 * w + 3] = 255;

		}
	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, depthImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(depthImage);

}

void cross_checking(const char* input1, const char* input2, const char* output){ // Take the two depth map as input and gives and cross checked image as output

	// Initialisation of the variables and decoding of the input images

	unsigned width;
	unsigned height;

	unsigned char* image1;
	unsigned char* image2;
	unsigned error;
	unsigned value;

	error = lodepng_decode32_file(&image1, &width, &height, input1);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&image2, &width, &height, input2);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned dValue1 = 0;
	unsigned dValue2 = 0;

	unsigned char* checkedImage = malloc(width * height * 4); // Declaration of the output image and memory allocation

	// For each pixel of the two input images

	for(int h = 0; h < height; h++){

		for(int w = 0; w < width; w++){

			// We get back to the "disparity scale" before the normalization

			dValue1 = ceil(((double)64 / 255) * image1[4 * width * h + 4 * w]);
			dValue2 = ceil(((double)64 / 255) * image2[4 * width * h + 4 * w]);

			if( abs( dValue1 - dValue2 ) < 8 ){ // If the difference between the value of image 1 and 2 is inferior to the threshold

				value = image1[4 * width * h + 4 * w]; // We take the value of the first image

				// And put it in the ouput image

				checkedImage[4 * width * h + 4 * w + 0] = value;
				checkedImage[4 * width * h + 4 * w + 1] = value;
				checkedImage[4 * width * h + 4 * w + 2] = value;
				checkedImage[4 * width * h + 4 * w + 3] = 255;

			}

			else{

				// Else, we put 0

				checkedImage[4 * width * h + 4 * w + 0] = 0;
				checkedImage[4 * width * h + 4 * w + 1] = 0;
				checkedImage[4 * width * h + 4 * w + 2] = 0;
				checkedImage[4 * width * h + 4 * w + 3] = 255;

			}

		}

	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, checkedImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(checkedImage);

}

void occlusion_filling(const char* input, const char* output){ // Fill the holes done during the cross checking

	// Initialisation of the variables and decoding of the input images

	unsigned width;
	unsigned height;
	unsigned error;

	unsigned char* image;

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* result = malloc(width * height * 4); // Declaration of the output image and memory allocation

	// Declaration and initialization of the values used in the algorithm

	unsigned value;
	int found = 0;
	int sizeWindow = 3;
	int halfWindowSize = floor(sizeWindow / 2);
	int newHalfWindowSize = halfWindowSize;

	// For each pixel of the image, considering the borders for the research window

	for(int h = halfWindowSize; h < height - halfWindowSize; h++){

		for(int w = halfWindowSize; w < width - halfWindowSize; w++){

			if( image[4 * width * h + 4 * w] == 0 ){ // If a pixel value is 0

				while(!found){ // While no values other than 0 as been found

					// Search for a new value other than 0 in the research window

					for(int j = h - newHalfWindowSize; j < h + newHalfWindowSize; j++){

						for(int i = w - newHalfWindowSize; i < w + newHalfWindowSize; i++){

							if( image[4 * width * j + 4 * i] != 0 && !found){ // If a value other than 0 is found, and no other value was found in the window

								// Put this value in the output image

								value = image[4 * width * j + 4 * i];

								result[4 * width * h + 4 * w + 0] = value;
								result[4 * width * h + 4 * w + 1] = value;
								result[4 * width * h + 4 * w + 2] = value;
								result[4 * width * h + 4 * w + 3] = 255;

								found = 1; // The value is found

							}

						}

					}

					// Increase the size of the window, in case the value was not found

					sizeWindow += 2;
					newHalfWindowSize = floor(sizeWindow / 2);

					if( w - newHalfWindowSize < 0 || w + newHalfWindowSize > width || h - newHalfWindowSize < 0 || h + newHalfWindowSize > height ){ // Check if increasing the size of the window do not overflow

						found = 1; // Otherwise, stop the loop

					}

				}

				// Initialize the variables for the next pixel

				found = 0;
				sizeWindow = 3;
				newHalfWindowSize = floor(sizeWindow / 2);

			}

			else{ // If the value of the pixel is not 0, save the value on the ouput image

				result[4 * width * h + 4 * w + 0] = image[4 * width * h + 4 * w + 0];
				result[4 * width * h + 4 * w + 1] = image[4 * width * h + 4 * w + 1];
				result[4 * width * h + 4 * w + 2] = image[4 * width * h + 4 * w + 2];
				result[4 * width * h + 4 * w + 3] = 255;

			}

		}

	}

	// Encode the output image and save it on the computer

	 error = lodepng_encode32_file(output, result, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(result);

}

int main(int argc, char* argv[]){

	LARGE_INTEGER clockFrequency;
	QueryPerformanceFrequency(&clockFrequency);

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;

	QueryPerformanceCounter(&startTime);

	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png");
	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im1.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png");
	ZNCC_right("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth1.png", 9);
	ZNCC_left("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth2.png", 9);
	cross_checking("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth1.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth2.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/cross_check.png");
	occlusion_filling("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/cross_check.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/occlusion.png");

	QueryPerformanceCounter(&endTime);

	LARGE_INTEGER delta;

	delta.QuadPart = endTime.QuadPart - startTime.QuadPart;
	float deltaSeconds = (float)delta.QuadPart / clockFrequency.QuadPart;

	printf("Execution time:\t%f s", deltaSeconds);

	return 0;

}
