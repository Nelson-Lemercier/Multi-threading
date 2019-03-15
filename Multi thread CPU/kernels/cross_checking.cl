__kernel void cross_checking(__global unsigned char* image1, __global unsigned char* image2, __global unsigned char* checkedImage, 
__private unsigned dValue1, __private unsigned dValue2, __private unsigned value){

	int index = get_global_id(0);

	// We get back to the "disparity scale" before the normalization

	dValue1 = ceil(((double)64 / 255) * image1[4 * index]);
	dValue2 = ceil(((double)64 / 255) * image2[4 * index]);

	if( abs( dValue1 - dValue2 ) < 8 ){ // If the difference between the value of image 1 and 2 is inferior to the threshold

		value = image1[4 * index]; // We take the value of the first image

		// And put it in the ouput image

		checkedImage[4 * index + 0] = value;
		checkedImage[4 * index + 1] = value;
		checkedImage[4 * index + 2] = value;
		checkedImage[4 * index + 3] = 255;

	}

	else{

		// Else, we put 0

		checkedImage[4 * index + 0] = 0;
		checkedImage[4 * index + 1] = 0;
		checkedImage[4 * index + 2] = 0;
		checkedImage[4 * index + 3] = 255;

	}
	

}