#include "tgaimage.h"
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

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
	TGAImage image(100, 100, TGAImage::RGB);
	DrawLine(10, 10, 15, 90, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

