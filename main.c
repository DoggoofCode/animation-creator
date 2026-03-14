#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VIDEO_WIDTH 1000
#define VIDEO_HEIGHT 1000

typedef struct {
	int r;
	int g;
	int b;
	float z_index;
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
void create_pixel_buf(PointCloud*, int, Point*, Pixel*);
int pixel_offset(int x, int y);
ScreenPosition virtual_to_real_screen(float x, float y);
Point rotate_x(Point p, float theta);
Point rotate_y(Point p, float theta);
Point rotate_z(Point p, float theta);


int main(int argc, char** argv)
{

	Point CameraPosition = {0., 0., -5};

	PointCloud points;
	points.point_count = 8;
	points.point_ptr = calloc(points.point_count, sizeof(Point));

	*points.point_ptr = (Point){2., 2., 2.};
	*(points.point_ptr+1) = (Point){2., 2., -2.};
	*(points.point_ptr+2) = (Point){-2., 2., -2.};
	*(points.point_ptr+3) = (Point){-2., 2., 2.};
	*(points.point_ptr+4) = (Point){2., -2., 2.};
	*(points.point_ptr+5) = (Point){2., -2., -2.};
	*(points.point_ptr+6) = (Point){-2., -2., -2.};
	*(points.point_ptr+7) = (Point){-2., -2., 2.};

	Pixel* pixel_buffer = calloc(VIDEO_WIDTH * VIDEO_HEIGHT, sizeof(Pixel));

	for (int frame_count = 0; frame_count < 30*10; frame_count++)
	{
		printf("Refactoring point cloud for frame %d...\n", frame_count);
		// TODO: Active refactoring for point cloud
		printf("Creating pixel buff for frame %d...\n", frame_count);
		create_pixel_buf(&points, frame_count, &CameraPosition, pixel_buffer);
		printf("Creating file for frame %d...\n", frame_count);
		create_ppm(pixel_buffer, frame_count);
		memset(pixel_buffer, 0, VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(Pixel));
	}
	free(pixel_buffer);
	free(points.point_ptr);
	return 0;
}

void create_ppm(Pixel* image_buffer, int frame_index)
{
	Pixel* pixel_ptr = image_buffer;
	char filename[100];
	snprintf(filename, sizeof(filename), "frames/frame-%02d.ppm", frame_index);

	FILE *fptr = fopen(filename, "w");

	fprintf(fptr, "P3\n%d %d\n255\n", VIDEO_WIDTH, VIDEO_HEIGHT);

	// Buffer for the entire row's string
	char* frame_char_buffer = calloc(VIDEO_HEIGHT*VIDEO_WIDTH*12+1, sizeof(char));

    char single_pixel_character_buffer[13];
    size_t single_char_strlen;

   	char* frame_char_buffer_pointer = frame_char_buffer;
	for (int y_pixel = 0; y_pixel < VIDEO_HEIGHT; y_pixel++)
	{
		for (int x_pixel = 0; x_pixel < VIDEO_WIDTH; x_pixel++)
		{
		    // Resets and adds a single pixels string into single_pixel_character_buffer
    		memset(single_pixel_character_buffer, 0, sizeof(single_pixel_character_buffer));
            snprintf(single_pixel_character_buffer, sizeof(single_pixel_character_buffer), "%d %d %d ", pixel_ptr->r, pixel_ptr->g, pixel_ptr->b);
            single_char_strlen = strlen(single_pixel_character_buffer);

            // Updates the entire rows position buffer then copies single_pixel into it
    		strncpy(frame_char_buffer_pointer, single_pixel_character_buffer, single_char_strlen);

            frame_char_buffer_pointer += single_char_strlen;
			pixel_ptr++;
		}
		*(frame_char_buffer_pointer++) = '\n';
		// frame_char_buffer_pointer++;
	}
	fprintf(fptr, "%s", frame_char_buffer);

	free(frame_char_buffer);
	fclose(fptr);

}

void create_pixel_buf(PointCloud* point_cloud, int frame_index, Point* camera_position, Pixel* pixel_buffer)
{

	// TODO: Move to its own function
	for (int point_index = 0; point_index < point_cloud->point_count; point_index++)
	{
		// Rotates all points around the z axis by 1 degree
		point_cloud->point_ptr[point_index] = rotate_z(point_cloud->point_ptr[point_index], 1 * (3.14159 / 180));
		point_cloud->point_ptr[point_index] = rotate_y(point_cloud->point_ptr[point_index], 1 * (3.14159 / 180));
	}

	for (int point_index = 0; point_index < point_cloud->point_count; point_index++)
	{
		Point current_point = point_cloud->point_ptr[point_index];
		current_point.z -= camera_position->z;
		float screen_x = current_point.x / current_point.z;
		float screen_y = current_point.y / current_point.z;
		ScreenPosition screen = virtual_to_real_screen(screen_x, screen_y);
		int display_size = 30 * (10 / sqrt(current_point.z));
		int buffer_offset, greyscale_size;
		float length_from_outside, circle_squared, alias_percent;
		printf("Frame: %d, Point#: %d, Screen Pos: (%f\x1b[2m[x: %f z: %f]\x1b[0m %f\x1b[2m[y: %f z: %f]\x1b[0m)\n", frame_index, point_index, screen_x, current_point.x, current_point.z, screen_y, current_point.y, current_point.z);
		for (int x_off = -display_size / 2; x_off < display_size / 2; x_off++)
		{
			for (int y_off = -display_size / 2; y_off < display_size / 2; y_off++)
			{
				buffer_offset = pixel_offset(screen.x+x_off, screen.y+y_off);
				circle_squared = (16*16) * pow(10 / current_point.z, 2);
				alias_percent = (x_off*x_off + y_off*y_off) / circle_squared;

				if (buffer_offset == -1)
					continue;

				if ((pixel_buffer + buffer_offset)->z_index != 0 && (pixel_buffer + buffer_offset)->z_index < current_point.z)
					continue;

				if (alias_percent < 0.9)
				{
					*(pixel_buffer + buffer_offset) = (Pixel){255, 255, 255, current_point.z};
				} else if (alias_percent < 1.)
				{
					greyscale_size = 250 - 2500*(alias_percent - .9);
					*(pixel_buffer + buffer_offset) = (Pixel){greyscale_size, greyscale_size, greyscale_size, current_point.z};
				}
			}
		}
	}
}

int pixel_offset(int x, int y)
{
	if (0 <= x && x <= VIDEO_WIDTH && 0 <= y && y <= VIDEO_HEIGHT)
		return x + (y * VIDEO_HEIGHT);
	return -1;
}

ScreenPosition virtual_to_real_screen(float x, float y)
{
	return (ScreenPosition){(x+1) * VIDEO_WIDTH / 2, (1-y)* VIDEO_HEIGHT / 2};
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
