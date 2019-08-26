#include "tgaimage.h"
#include "gmt.h"
#include "model.h"
#include <vector>
#include <iostream>
#include <array>
#include <algorithm>
#include <math.h>
#include "profiler.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model* model = NULL;
const int width = 1000;
const int height = 1000;

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

		float w1 = static_cast<float>(P.y * C.x - P.x * C.y) / (B.y * C.x - B.x * C.y);
		if (w1 > 1 && w1 < 0) return false;
		float w2 = static_cast<float>(P.y - w1 * B.y) / C.y;

		return w1 >= 0 && w2 >= 0 && (w1 + w2) <= 1;
	}

	bool IsInTriangle(Vec2i p) {
		int aSide = (points[0].y - points[1].y) * p.x + (points[1].x - points[0].x) * p.y + (points[0].x * points[1].y - points[1].x * points[0].y);
		int bSide = (points[1].y - points[2].y) * p.x + (points[2].x - points[1].x) * p.y + (points[1].x * points[2].y - points[2].x * points[1].y);
		int cSide = (points[2].y - points[0].y) * p.x + (points[0].x - points[2].x) * p.y + (points[2].x * points[0].y - points[0].x * points[2].y);

		return (aSide >= 0 && bSide >= 0 && cSide >= 0) || (aSide < 0 && bSide < 0 && cSide < 0);
	}

	void BetterDraw(TGAImage& image, const TGAColor& color) {
		if (points[0].y == points[1].y && points[0].y == points[1].y) return;
		int xMax = std::max<int>({ points[0].x, points[1].x, points[2].x });
		int xMin = std::min<int>({ points[0].x, points[1].x, points[2].x });
		for (int x = xMin; x <= xMax; x++) {
			for (int y = points[0].y; y <= points[2].y; y++) {
				if (InTriangle({ x, y })) {
					image.set(x, y, color);
				}
			}
		}

		//auto t0 = points[0];
		//auto t1 = points[1];
		//auto t2 = points[2];

		//if (t0.y == t1.y && t0.y == t2.y) return;
		//if (t0.y > t1.y) std::swap(t0, t1);
		//if (t0.y > t2.y) std::swap(t0, t2);
		//if (t1.y > t2.y) std::swap(t1, t2);
		//int total_height = t2.y - t0.y;
		//for (int i = 0; i < total_height; i++) {
		//	bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		//	int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		//	float alpha = (float)i / total_height;
		//	float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
		//	Vec2i A = t0 + (t2 - t0) * alpha;
		//	Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		//	if (A.x > B.x) std::swap(A, B);
		//	for (int j = A.x; j <= B.x; j++) {
		//		image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
		//	}
		//}
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
	{
		LogDuration lg;
		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<int> face = model->face(i);
			Vec2i screen_coords[3];
			for (int j = 0; j < 3; j++) {
				Vec3f world_coords = model->vert(face[j]);
				screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
			}
			Triangle(screen_coords[0], screen_coords[1], screen_coords[2]).BetterDraw(image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

