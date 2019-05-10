kernel void transform_grey(read_only image2d_t image, write_only image2d_t grey_image){

    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint r, g, b, a, grey_value = 0;
    uint4 values = read_imageui(image, smp, coord);

    r = values.x;
    g = values.y;
    b = values.z;
    a = values.w;

    grey_value = (0.2126 * r + 0.7152 * g + 0.0722 * b);

    values.x = grey_value;
    values.y = grey_value;
    values.z = grey_value;
    values.w = 255;

    write_imageui(grey_image, coord, values);

}


kernel void resize(read_only image2d_t input_image, write_only image2d_t resize_image){

    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    if(coord.x % 4 == 0 && coord.y % 4 == 0 ){
        uint4 values = read_imageui(input_image, smp, coord);
        coord.x /= 4;
        coord.y /= 4;;
        write_imageui(resize_image, coord, values);
    }
}

kernel void ZNCC_left(read_only image2d_t image_left, read_only image2d_t image_right, write_only image2d_t output, int sizeWindow, int halfWindow, unsigned int max_disp){
    
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 values = (uint4)(0, 0, 0, 0);
    uint4 value_left, value_right;
    double znccValue, maxSum, meanValueLeft, meanValueRight, upperSum, lowerSum_0, lowerSum_1;
    unsigned int sum_left, sum_right, bestDisp;

    znccValue = 0; maxSum = 0;

    for(int d = 0; d < max_disp; d++){

        sum_left = 0; sum_right = 0;

        for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){
            for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){

                int2 coordLeft = (int2)(i + d, j);
                int2 coordRight = (int2)(i, j);
                value_left = read_imageui(image_left, smp, coordLeft);
                value_right = read_imageui(image_right, smp, coordRight);
                sum_left += value_left.x;
                sum_right += value_right.x;

            }
        }

        meanValueLeft = 0; meanValueRight = 0;

        meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);
        meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);

        upperSum = 0; lowerSum_0 = 0; lowerSum_1 = 0;

        for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){
            for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){

                int2 coordLeft = (int2)(i + d, j);
                int2 coordRight = (int2)(i, j);
                value_left = read_imageui(image_left, smp, coordLeft);
                value_right = read_imageui(image_right, smp, coordRight);
                upperSum += (value_left.x - meanValueLeft) * (value_right.x - meanValueRight);
                lowerSum_0 += (value_left.x - meanValueLeft) * (value_left.x - meanValueLeft);
                lowerSum_1 += (value_right.x - meanValueRight) * (value_right.x - meanValueRight);

            }
        }

        znccValue = upperSum * rsqrt(lowerSum_0 * lowerSum_1); 

        if(znccValue > maxSum){
            maxSum = znccValue;
            bestDisp = d;
        }
    }

    unsigned int depthValue = ceil(((double)255 / max_disp) * bestDisp);

    values.x = depthValue;
    values.y = depthValue;
    values.z = depthValue;
    values.w = 255;
    write_imageui(output, coord, values);
}

kernel void ZNCC_right(read_only image2d_t image_left, read_only image2d_t image_right, write_only image2d_t output, int sizeWindow, int halfWindow, unsigned int max_disp){
    
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 values = (uint4)(0, 0, 0, 0);
    uint4 value_left, value_right;
    double znccValue, maxSum, meanValueLeft, meanValueRight, upperSum, lowerSum_0, lowerSum_1;
    unsigned int sum_left, sum_right, bestDisp;

    znccValue = 0; maxSum = 0;

    for(int d = 0; d < max_disp; d++){

        sum_left = 0; sum_right = 0;

        for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){
            for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){

                int2 coordLeft = (int2)(i, j);
                int2 coordRight = (int2)(i - d, j);
                value_left = read_imageui(image_left, smp, coordLeft);
                value_right = read_imageui(image_right, smp, coordRight);
                sum_left += value_left.x;
                sum_right += value_right.x;

            }
        }

        meanValueLeft = 0; meanValueRight = 0;

        meanValueLeft = (double)sum_left / (sizeWindow * sizeWindow);
        meanValueRight = (double)sum_right / (sizeWindow * sizeWindow);

        upperSum = 0; lowerSum_0 = 0; lowerSum_1 = 0;

        for(int j = coord.y - halfWindow; j <= coord.y + halfWindow; j++){
            for(int i = coord.x - halfWindow; i <= coord.x + halfWindow; i++){

                int2 coordLeft = (int2)(i, j);
                int2 coordRight = (int2)(i - d, j);
                value_left = read_imageui(image_left, smp, coordLeft);
                value_right = read_imageui(image_right, smp, coordRight);
                upperSum += (value_left.x - meanValueLeft) * (value_right.x - meanValueRight);
                lowerSum_0 += (value_left.x - meanValueLeft) * (value_left.x - meanValueLeft);
                lowerSum_1 += (value_right.x - meanValueRight) * (value_right.x - meanValueRight);

            }
        }

        znccValue = upperSum * rsqrt(lowerSum_0 * lowerSum_1);

        if(znccValue > maxSum){
            maxSum = znccValue;
            bestDisp = d;
        }
    }

    unsigned int depthValue = ceil(((double)255 / max_disp) * bestDisp);

    values.x = depthValue;
    values.y = depthValue;
    values.z = depthValue;
    values.w = 255;
    write_imageui(output, coord, values);
}

kernel void cross_checking(read_only image2d_t depth_left, read_only image2d_t depth_right, write_only image2d_t cross_checking){
    
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 valueLeft, valueRight, valueOutput;
    unsigned int dValue1, dValue2;
    
    valueLeft = read_imageui(depth_left, smp, coord);
    valueRight = read_imageui(depth_right, smp, coord);
    
    dValue1 = (double)64 / 255 * valueLeft.x;
    dValue2 = (double)64 / 255 * valueRight.x;
 
    if( abs_diff( dValue1, dValue2 ) < 8 ){

        valueOutput.x = valueLeft.x;
        valueOutput.y = valueLeft.x;
        valueOutput.z = valueLeft.x;
        valueOutput.w = 255;
    }
    
    else{

        valueOutput.x = 0;
        valueOutput.y = 0;
        valueOutput.z = 0;
        valueOutput.w = 255;
    }
    
    write_imageui(cross_checking, coord, valueOutput);

}

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