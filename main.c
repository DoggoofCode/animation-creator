#include <stdio.h>
#include <stdlib.h>

#define VIDEO_WIDTH 1000
#define VIDEO_HEIGHT 1000

typedef struct {
	int r;
	int g;
	int b;
} Pixel;

typedef struct {
	int x;
	int y;
	int z;
} Point;

void create_ppm(Pixel*, int);
Pixel* create_pixel_buf(int);

int main(int argc, char** argv)
{
	for (int frame_count = 0; frame_count < 100; frame_count++)
	{
		printf("Creating pixel buff for frame %d...\n", frame_count);
		Pixel* buf = create_pixel_buf(frame_count);
		printf("Creating file for frame %d...\n", frame_count);
		create_ppm(buf, frame_count);
	}
	return 0;
}

void create_ppm(Pixel* image_buffer, int frame_index)
{
	Pixel* pixel_ptr = image_buffer; 
	char filename[100];
	snprintf(filename, sizeof(filename), "frames/frame-%02d.ppm", frame_index);

	FILE *fptr = fopen(filename, "w");

	int r, g, b;
	fprintf(fptr, "P3\n%d %d\n255\n", VIDEO_WIDTH, VIDEO_HEIGHT);
	for (int y_pixel = 0; y_pixel < VIDEO_HEIGHT; y_pixel++)
	{
		for (int x_pixel = 0; x_pixel < VIDEO_WIDTH; x_pixel++)
		{
			r = pixel_ptr->r;
			g = pixel_ptr->g;
			b = pixel_ptr->b;
			fprintf(fptr, "%d %d %d ", r, g, b);
			pixel_ptr++;
		}
		fprintf(fptr, "\n");
	}

	fclose(fptr);
	free(image_buffer);

}

Pixel* create_pixel_buf(int frame_index)
{
	Pixel* pixel_buffer = calloc(VIDEO_WIDTH * VIDEO_HEIGHT, sizeof(Pixel));
	Pixel* pixel_ptr = pixel_buffer; 
	for (int y_pixel = 0; y_pixel < VIDEO_HEIGHT; y_pixel++)
	{
		for (int x_pixel = 0; x_pixel < VIDEO_WIDTH; x_pixel++)
		{
			pixel_ptr->r = 255*((float)((x_pixel-10*frame_index)%1000) / VIDEO_WIDTH);
			pixel_ptr->g = 255*((float)((y_pixel-10*frame_index)%1000) / VIDEO_HEIGHT);
			pixel_ptr->b = 0;
			pixel_ptr++;
			
		}
	}
	return pixel_buffer;
}

