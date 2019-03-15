__kernel void transform_grey(__global unsigned char* image, __global unsigned char* grey_image, __private r, __private g, __private b, __private a, __private gery_value){

	int index = get_global_id(0);

	r = image[4 * index + 0];
	g = image[4 * index + 1];
	b = image[4 * index + 2];
	a = image[4 * index + 3];

	grey_value = (0.2126 * r + 0.7152 * g + 0.0722 * b);

	grey_image[4 * index + 0] = grey_value;
	grey_image[4 * index + 1] = grey_value;
	grey_image[4 * index + 2] = grey_value;
	grey_image[4 * index + 3] = a;

}
	