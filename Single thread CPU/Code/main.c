#include <lodePNG.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <math.h>

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

			grey_value = (r + g + b) / 3;

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

int main(int argc, char* argv[]){

	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im0.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png");
	transform_grey("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/im1.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_left.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/left.png");
	resize("C:/Users/Nelson/Documents/Etudes/Multi threading/Images/grey_right.png", "C:/Users/Nelson/Documents/Etudes/Multi threading/Images/right.png");

	return 0;

}
