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