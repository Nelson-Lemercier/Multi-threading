__kernel void occlusion_filling(__global unsigned char* image, __global unsigned char* result, __global int halfWindowSize, __global unsigned width,
__private int newHalfWindowSize, __private int sizeWindow, __private int found, __private unsigned value){

	int index = get_global_id(0);

	if( image[4 * index] == 0 ){ // If a pixel value is 0

	while(!found){ // While no values other than 0 as been found

		// Search for a new value other than 0 in the research window

		for(int j = index - newHalfWindowSize; j < index + newHalfWindowSize; j++){

			for(int i = index - newHalfWindowSize; i < index + newHalfWindowSize; i++){

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

		if( index - newHalfWindowSize < 0 || index + newHalfWindowSize > width || index - newHalfWindowSize < 0 || index + newHalfWindowSize > height ){ // Check if increasing the size of the window do not overflow

			found = 1; // Otherwise, stop the loop

		}

	}

	// Initialize the variables for the next pixel

	found = 0;
	sizeWindow = 3;
	newHalfWindowSize = floor(sizeWindow / 2);

	}
	
	else{ // If the value of the pixel is not 0, save the value on the ouput image
	
		result[4 * index + 0] = image[4 * index + 0];
		result[4 * index + 1] = image[4 * index + 1];
		result[4 * index + 2] = image[4 * index + 2];
		result[4 * index + 3] = 255;
	
	}

}