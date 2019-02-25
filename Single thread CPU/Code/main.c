#include <lodePNG.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <math.h>

int show(const char* filename) {
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

void encode(const char* filename, const unsigned char* image, unsigned width, unsigned height) {

  unsigned error = lodepng_encode32_file(filename, image, width, height);

  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

}

void transform_grey(const char* input, const char* output){

	unsigned width;
	unsigned height;

	unsigned char* image;
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned char* grey_image = malloc(width * height * 4);
	unsigned x, y;

	Uint32 r, g, b, a, grey_value;

	for(y = 0; y < height; y++){

		for(x = 0; x < width; x++) {

			r = image[4 * width * y + 4 * x + 0];
			g = image[4 * width * y + 4 * x + 1];
			b = image[4 * width * y + 4 * x + 2];
			a = image[4 * width * y + 4 * x + 3];

			grey_value = (0.2126 * r + 0.7152 * g + 0.0722 * b);

			grey_image[4 * width * y + 4 * x + 0] = grey_value;
			grey_image[4 * width * y + 4 * x + 1] = grey_value;
			grey_image[4 * width * y + 4 * x + 2] = grey_value;
			grey_image[4 * width * y + 4 * x + 3] = a;

		}

	}


	 error = lodepng_encode32_file(output, grey_image, width, height);

	  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	  free(image);
	  free(grey_image);

}

void resize(const char* input, const char* output){

	unsigned width;
	unsigned height;

	unsigned char* image;
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, input);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	unsigned width_redux = width / 4;
	unsigned height_redux = height / 4;

	unsigned char* image_redux = malloc(width_redux * height_redux * 4);
	unsigned x, y, k, l;

	k = 0;
	l = 0;

	Uint32 r, g, b, a;

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

	 error = lodepng_encode32_file(output, image_redux, width_redux, height_redux);

	  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	  free(image);
	  free(image_redux);

}

void ZNCC_1(const char* input_left, const char* input_right, const char* output, int sizeWindow){

	unsigned width;
	unsigned height;

	unsigned char* image_left;
	unsigned char* image_right;
	unsigned error;

	error = lodepng_decode32_file(&image_left, &width, &height, input_left);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&image_right, &width, &height, input_right);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	Uint32 value_left, value_right, sum_left, sum_right, meanValueLeft, meanValueRight, upperSum, lowerSum_0, lowerSum_1, bestDisp;

	float znccValue, maxSum;

	unsigned max_disp = 65; //260 / 4

	int halfWindowSize = floor(sizeWindow / 2);

	unsigned char* depthImage = malloc(width * height * 4);

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

				meanValueLeft = sum_left / (sizeWindow * sizeWindow);
				meanValueRight = sum_right / (sizeWindow * sizeWindow);

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


			int depthValue = ceil((255 / max_disp) * bestDisp);

			 depthImage[4 * width * h + 4 * w + 0] = depthValue;
			 depthImage[4 * width * h + 4 * w + 1] = depthValue;
			 depthImage[4 * width * h + 4 * w + 2] = depthValue;
			 depthImage[4 * width * h + 4 * w + 3] = 255;

		}

	}

	 error = lodepng_encode32_file(output, depthImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(depthImage);

}

void ZNCC_2(const char* input_left, const char* input_right, const char* output, int sizeWindow){

	unsigned width;
	unsigned height;

	unsigned char* image_left;
	unsigned char* image_right;
	unsigned error;

	error = lodepng_decode32_file(&image_left, &width, &height, input_left);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	error = lodepng_decode32_file(&image_right, &width, &height, input_right);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	Uint32 value_left, value_right, sum_left, sum_right, meanValueLeft, meanValueRight, upperSum, lowerSum_0, lowerSum_1, bestDisp;

	float znccValue, maxSum;

	unsigned max_disp = 65; //260 / 4

	int halfWindowSize = floor(sizeWindow / 2);

	unsigned char* depthImage = malloc(width * height * 4);

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

				meanValueLeft = sum_left / (sizeWindow * sizeWindow);
				meanValueRight = sum_right / (sizeWindow * sizeWindow);

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


			int depthValue = ceil((255 / max_disp) * bestDisp);

			 depthImage[4 * width * h + 4 * w + 0] = depthValue;
			 depthImage[4 * width * h + 4 * w + 1] = depthValue;
			 depthImage[4 * width * h + 4 * w + 2] = depthValue;
			 depthImage[4 * width * h + 4 * w + 3] = 255;

		}
	}

	 error = lodepng_encode32_file(output, depthImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(depthImage);

}

void cross_checking(const char* input1, const char* input2, const char* output){

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

	unsigned char* checkedImage = malloc(width * height * 4);

	for(int h = 0; h < height; h++){

		for(int w = 0; w < width; w++){

			if( abs( image1[4 * width * h + 4 * w] - image2[4 * width * h + 4 * w] ) < 8 ){

				value = image1[4 * width * h + 4 * w];

				checkedImage[4 * width * h + 4 * w + 0] = value;
				checkedImage[4 * width * h + 4 * w + 1] = value;
				checkedImage[4 * width * h + 4 * w + 2] = value;
				checkedImage[4 * width * h + 4 * w + 3] = 255;

			}

			else{

				checkedImage[4 * width * h + 4 * w + 0] = 0;
				checkedImage[4 * width * h + 4 * w + 1] = 0;
				checkedImage[4 * width * h + 4 * w + 2] = 0;
				checkedImage[4 * width * h + 4 * w + 3] = 255;

			}

		}

	}

	 error = lodepng_encode32_file(output, checkedImage, width, height);
	 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

	free(checkedImage);

}

int main(int argc, char* argv[]){

	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png");
	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im1.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png");
	ZNCC_1("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth1.png", 9);
	ZNCC_2("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth2.png", 9);
	cross_checking("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth1.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth2.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/cross_check.png");
	//show("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/depth.png");

	return 0;

}
