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

        znccValue = upperSum / (sqrt(lowerSum_0) * sqrt(lowerSum_1));

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