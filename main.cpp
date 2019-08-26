#include "tgaimage.h"
#include "gmt.h"
#include "model.h"
#include <vector>
#include <iostream>
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model* model = NULL;
const int width = 2000;
const int height = 2000;

struct Point {
	int x;
	int y;
};

template <typename T>
T abs(const T& value) {
	return value > 0 ? value : -value;
}

void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
	bool steep = false;
	// if the line is steep, we transpose the image
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	// make it left-to-right
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int deltax = std::abs(x1 - x0);
	int deltay = std::abs(y1 - y0);
	int error = 0;
	int y = y0;
	int diry = y1 > y0 ? 1 : -1;

	for (int x = x0; x <= x1; x++) {
		if (steep) {
			// if transposed, de-transpose
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += deltay;
		if (2 * error >= deltax) {
			y += diry;
			error -= deltax;
		}
	}
}

int main(int argc, char** argv) {
	model = new Model("obj/african_head/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			DrawLine(x0, y0, x1, y1, image, white);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

