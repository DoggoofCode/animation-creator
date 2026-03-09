#include <math.h>
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
} ScreenPosition;

typedef struct {
	float x;
	float y;
	float z;
} Point;

typedef struct {
	Point* point_ptr;
	int point_count;
} PointCloud;

void create_ppm(Pixel*, int);
Pixel* create_pixel_buf(PointCloud*, int);
int pixel_offset(int x, int y);
ScreenPosition virtual_to_real_screen(float x, float y);
Point rotate_x(Point p, float theta);
Point rotate_y(Point p, float theta);
Point rotate_z(Point p, float theta);


int main(int argc, char** argv)
{
	PointCloud points;
	points.point_count = 2;
	points.point_ptr = calloc(points.point_count, sizeof(Point));
	
	*points.point_ptr = (Point){-2., 0., 5.};
	*(points.point_ptr+1) = (Point){2., 2., 10.};

	for (int frame_count = 0; frame_count < 50; frame_count++)
	{
		printf("Refactoring point cloud for frame %d...\n", frame_count);
		// TODO: Active refactoring for point cloud
		printf("Creating pixel buff for frame %d...\n", frame_count);
		Pixel* buf = create_pixel_buf(&points, frame_count);
		printf("Creating file for frame %d...\n", frame_count);
		create_ppm(buf, frame_count);
	}
	free(points.point_ptr);
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

Pixel* create_pixel_buf(PointCloud* point_cloud, int frame_index)
{

	Pixel* pixel_buffer = calloc(VIDEO_WIDTH * VIDEO_HEIGHT, sizeof(Pixel));
	
	// TODO: Move to its own function
	for (int point_index = 0; point_index < point_cloud->point_count; point_index++)
	{
		// Rotates all points around the z axis by 1 degree
		point_cloud->point_ptr[point_index] = rotate_z(point_cloud->point_ptr[point_index], 1 * (3.14159 / 180));
	}

	for (int point_index = 0; point_index < point_cloud->point_count; point_index++)
	{
		Point current_point = point_cloud->point_ptr[point_index];
		float screen_x = current_point.x / current_point.z;
		float screen_y = current_point.y / current_point.z;
		ScreenPosition screen = virtual_to_real_screen(screen_x, screen_y);
		int display_size = 30 * (10 / current_point.z);
		for (int x_off = -display_size / 2; x_off < display_size / 2; x_off++)
		{
			for (int y_off = -display_size / 2; y_off < display_size / 2; y_off++)
			{
				if (x_off*x_off + y_off*y_off <= 225 * (10 / current_point.z))
				{
					*(pixel_buffer + pixel_offset(screen.x+x_off, screen.y+y_off)) = (Pixel){255, 255, 255};
				}
			}
		}
	}
	return pixel_buffer;
}

int pixel_offset(int x, int y)
{
	return x + (y * VIDEO_HEIGHT);
}

ScreenPosition virtual_to_real_screen(float x, float y)
{
	return (ScreenPosition){(x+1) * VIDEO_WIDTH / 2, ((-y)+1)* VIDEO_HEIGHT / 2};
}

Point rotate_x(Point p, float theta) {
    Point r;

    r.x = p.x;
    r.y = p.y * cosf(theta) - p.z * sinf(theta);
    r.z = p.y * sinf(theta) + p.z * cosf(theta);

    return r;
}

Point rotate_y(Point p, float theta) {
    Point r;

    r.x = p.x * cosf(theta) + p.z * sinf(theta);
    r.y = p.y;
    r.z = -p.x * sinf(theta) + p.z * cosf(theta);

    return r;
}

Point rotate_z(Point p, float theta) {
    Point r;

    r.x = p.x * cosf(theta) - p.y * sinf(theta);
    r.y = p.x * sinf(theta) + p.y * cosf(theta);
    r.z = p.z;

    return r;
}

