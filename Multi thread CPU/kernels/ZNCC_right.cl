__kernel void ZNCC_right(__global unsigned char* image_left, __global unsigned char* image_right, __global unsigned char* depthImage, __global int halfWindowSize, __global unsigned width,
__private Uint32 value_left, __private Uint32 value_right, __private Uint32 sum_left, __private Uint32 sum_right, __private Uint32 bestDisp, __private double ZNCCValue, 
__private double maxSum, __private double upperSum, __private double lowerSum_0, __private double lowerSum_1, __private double meanValueLeft, __private double meanValueRight){
	
	int index = get_global_id(0);
	
	maxSum = 0; bestDisp = 0;

	for(int d = 0; d < max_disp; d++){

		sum_left = 0; sum_right = 0; meanValueLeft = 0; meanValueRight = 0;

		for(int j = index - halfWindowSize; j < index + halfWindowSize; j++){

			for(int i = index - halfWindowSize; i < index + halfWindowSize; i++){

				value_left = image_left[4 * width * j + 4 * i ];
				value_right = image_right[4 * width * j + 4 * ( i - d )];

				sum_left += value_left;
				sum_right += value_right;

			}

		}

		meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);
		meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);

		upperSum = 0; lowerSum_0 = 0; lowerSum_1 = 0;

		for(int j = index - halfWindowSize; j < index + halfWindowSize; j++){

			for(int i = index - halfWindowSize; i < index + halfWindowSize; i++){

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

	 depthImage[4 * index + 0] = depthValue;
	 depthImage[4 * index + 1] = depthValue;
	 depthImage[4 * index + 2] = depthValue;
	 depthImage[4 * index + 3] = 255;
	
}