kernel void occlusion(read_only image2d_t input, write_only image2d_t output, unsigned width, unsigned height){
    
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 outputValues = (uint4)(0, 0, 0, 0);

	int found = 0;
	int sizeWindow = 3;
    
	int halfWindowSize = floor((float)sizeWindow / 2);
	int newHalfWindowSize = halfWindowSize;

    uint4 values = read_imageui(input, smp, coord);

    if( values.x == 0 ){ // If a pixel value is 0

        while(!found){ // While no values other than 0 as been found

            // Search for a new value other than 0 in the research window

            for(int j = coord.y - newHalfWindowSize; j < coord.y + newHalfWindowSize; j++){

                for(int i = coord.x - newHalfWindowSize; i < coord.x + newHalfWindowSize; i++){

                    int2 coordSearch = (int2)(i, j);
                    values = read_imageui(input, smp, coordSearch);

                    if( values.x != 0 && !found){ // If a value other than 0 is found, and no other value was found in the window

                        // Put this value in the output image

                        outputValues.x = values.x;
                        outputValues.y = values.x;
                        outputValues.z = values.x;
                        outputValues.w = 255;

                        found = 1; // The value is found

                    }

                }

            }

            // Increase the size of the window, in case the value was not found

            sizeWindow += 2;
            newHalfWindowSize = floor((float)sizeWindow / 2);

            if( coord.x - newHalfWindowSize < 0 || coord.x + newHalfWindowSize > width || coord.y - newHalfWindowSize < 0 || coord.y + newHalfWindowSize > height ){ // Check if increasing the size of the window do not overflow

                found = 1; // Otherwise, stop the loop

            }

        }

        // Initialize the variables for the next pixel

        found = 0;
        sizeWindow = 3;
        newHalfWindowSize = floor((float)sizeWindow / 2);

    }

    else{ // If the value of the pixel is not 0, save the value on the ouput image

        outputValues.x = values.x;
        outputValues.y = values.x;
        outputValues.z = values.x;
        outputValues.w = 255;

    }

     write_imageui(output, coord, outputValues);

}