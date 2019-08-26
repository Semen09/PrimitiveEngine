#include "tgaimage.h"
#include "gmt.h"
#include "model.h"
#include <vector>
#include <iostream>
#include <array>
#include <algorithm>
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model* model = NULL;
const int width = 100;
const int height = 100;

struct Point {
	int x;
	int y;

	Point() : x(0), y(0)
	{};
	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}
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

class Triangle {
public:
	Triangle(const Vec2i& first, const Vec2i& second, const Vec2i& third) {
		points[0] = first;
		points[1] = second;
		points[2] = third;

		if (points[0].y > points[1].y) points[0].Swap(points[1]);
		if (points[0].y > points[2].y) points[0].Swap(points[2]);
		if (points[1].y > points[2].y) points[1].Swap(points[2]);
	}

	void Draw(TGAImage& image, const TGAColor& color) {
		DrawLine(points[0].x, points[0].y, points[1].x, points[1].y, image, color);
		DrawLine(points[2].x, points[2].y, points[1].x, points[1].y, image, color);
		DrawLine(points[2].x, points[2].y, points[0].x, points[0].y, image, white);
	}

	bool InTriangle(Vec2i P)
	{
		P = P - points[0];
		Vec2i B = points[1] - points[0];
		Vec2i C = points[2] - points[0];

		// Zero division if
		if (C.y == 0) {
			C.Swap(B);
		}

		float w1 = (float)(P.y * C.x - P.x * C.y) / (B.y * C.x - B.x * C.y);
		float w2 = ((float)P.y - w1 * B.y) / C.y;

		return w1 >= 0 && w2 >= 0 && (w1 + w2) <= 1;
	}

	void BetterDraw(TGAImage& image, const TGAColor& color) {
		int xMax, xMin;
		if (points[0].x > points[1].x) {
			xMax = (points[0].x > points[2].x) ? points[0].x : points[2].x;
			xMin = (points[1].x > points[2].x) ? points[2].x : points[1].x;
		}
		else {
			xMax = (points[1].x > points[2].x) ? points[1].x : points[2].x;
			xMin = (points[0].x > points[2].x) ? points[2].x : points[0].x;
		}
		for (int x = xMin; x <= xMax; x++) {
			for (int y = points[0].y; y <= points[2].y; y++) {
				if (InTriangle({ x, y })) {
					image.set(x, y, red);
				}
			}
		}
	}
private:
	std::array<Vec2i, 3> points;
};

int main(int argc, char** argv) {
	model = new Model("obj/african_head/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	/*for (int i = 0; i < model->nfaces(); i++) {
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
	}*/
	Triangle a({ 0, 0 }, { 30, 30 }, { 50, 20 });
	Triangle b({90, 90}, {50, 80}, {70, 50});

	a.BetterDraw(image, red);
	b.BetterDraw(image, red);
	//a.Draw(image, red);
	//b.Draw(image, red);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

