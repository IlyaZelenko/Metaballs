#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace sf;


#define SQ_SIZE 4				// square size
#define R 50					// radius of metaball
#define N 512					// width and height of the screen (512x512 pixels)
#define INDEX(x, y) (x+y*N)		// just because I wanna use 1-dimentional array and macros for it
#define N_OF_SQ N/SQ_SIZE		// number of squares

float Q_rsqrt(float number)		// quake III quick reverse square root
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;							 // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck? 
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y));     // 1st iteration of Newton`s method
	//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

struct Metaball{
	float x, y;		//coordinates
	float vx, vy;	//velocity
	int r;			//radius
};

float InvDist(Metaball m, float x, float y){
	float dist_sqared = (m.x - x) * (m.x - x) + (m.y - y) * (m.y - y);
	float inv_sqrt = Q_rsqrt(dist_sqared);
	return m.r * inv_sqrt;
}



ConvexShape CalculateTheShape(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, Metaball metaballs[]) {
	float f0 = 0, f1 = 0, f2 = 0, f3 = 0;
	for (int k = 0; k < 2; k++) {
		f0 += InvDist(metaballs[k], x0, y0);
		f1 += InvDist(metaballs[k], x1, y1);
		f2 += InvDist(metaballs[k], x2, y2);
		f3 += InvDist(metaballs[k], x3, y3);
	}
	f0 = f0 >= 1 ? 1 : 0;
	f1 = f1 >= 1 ? 1 : 0;
	f2 = f2 >= 1 ? 1 : 0;
	f3 = f3 >= 1 ? 1 : 0;
	int mapped = f0 + 2 * f1 + 4 * f2 + 8 * f3;
	
	Vector2f a, b, c, d;
	a = Vector2f((x0 + x1) / 2, y0);
	b = Vector2f(x0, (y0 + y2) / 2);
	c = Vector2f(x1, (y1 + y3) / 2);
	d = Vector2f((x2 + x3)/2, y3);

	ConvexShape convex;
	switch (mapped) {
	case 0:
		convex.setPointCount(4);
		convex.setFillColor(Color::Black);

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, Vector2f(x1, y1));
		convex.setPoint(2, Vector2f(x3, y3));
		convex.setPoint(3, Vector2f(x2, y2));

		return convex;
	case 1:
		convex.setPointCount(3);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, a);
		convex.setPoint(2, b);
		return convex;
	case 2:
		convex.setPointCount(3);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x1, y1));
		convex.setPoint(1, a);
		convex.setPoint(2, c);
		return convex;
	case 3:
		convex.setPointCount(4);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, b);
		convex.setPoint(2, c);
		convex.setPoint(3, Vector2f(x1, y1));
		return convex;
	case 4:
		convex.setPointCount(3);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x2, y2));
		convex.setPoint(1, b);
		convex.setPoint(2, d);
		return convex;
	case 5:
		convex.setPointCount(4);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, a);
		convex.setPoint(2, d);
		convex.setPoint(3, Vector2f(x2, y2));
		return convex;
	case 6:
		convex.setPointCount(6);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, a);
		convex.setPoint(1, Vector2f(x1, y1));
		convex.setPoint(2, c);
		convex.setPoint(3, d);
		convex.setPoint(4, Vector2f(x2, y2));
		convex.setPoint(5, b);
		return convex;
	case 7:
		convex.setPointCount(5);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, Vector2f(x1, y1));
		convex.setPoint(2, c);
		convex.setPoint(3, d);
		convex.setPoint(4, Vector2f(x2, y2));
		return convex;
	case 8:
		convex.setPointCount(3);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x3, y3));
		convex.setPoint(1, c);
		convex.setPoint(2, d);
		return convex;
	case 9:
		convex.setPointCount(6);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, a);
		convex.setPoint(1, Vector2f(x0, y0));
		convex.setPoint(2, b);
		convex.setPoint(3, d);
		convex.setPoint(4, Vector2f(x3, y3));
		convex.setPoint(5, c);
		return convex;
	case 10:
		convex.setPointCount(4);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x1, y1));
		convex.setPoint(1, a);
		convex.setPoint(2, d);
		convex.setPoint(3, Vector2f(x3, y3));
		return convex;
	case 11:
		convex.setPointCount(5);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, Vector2f(x1, y1));
		convex.setPoint(2, Vector2f(x2, y2));
		convex.setPoint(3, d);
		convex.setPoint(4, c);
		return convex;
	case 12:
		convex.setPointCount(4);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x2, y2));
		convex.setPoint(1, b);
		convex.setPoint(2, c);
		convex.setPoint(3, Vector2f(x3, y3));
		return convex;
	case 13:
		convex.setPointCount(5);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, Vector2f(x2, y2));
		convex.setPoint(2, Vector2f(x3, y3));
		convex.setPoint(3, c);
		convex.setPoint(4, a);
		return convex;
	case 14:
		convex.setPointCount(5);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x1, y1));
		convex.setPoint(1, Vector2f(x3, y3));
		convex.setPoint(2, Vector2f(x2, y2));
		convex.setPoint(3, b);
		convex.setPoint(4, a);
		return convex;
	case 15:
		convex.setPointCount(4);
		convex.setFillColor(Color(250, 150, 100));

		convex.setPoint(0, Vector2f(x0, y0));
		convex.setPoint(1, Vector2f(x1, y1));
		convex.setPoint(2, Vector2f(x3, y3));
		convex.setPoint(3, Vector2f(x2, y2));
		return convex;
	}
}

int main() {





	Metaball mballs[2] = { {120.f, 256.f, 2.f, 0.f, R} , {400.f, 256.f, -2.f, 0.f, R} };

	RenderWindow window(VideoMode(N, N), "Metaballs");



	while (window.isOpen()) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed)
				window.close();
		}

		window.clear();



	
	
		for (int k = 0; k < 2; k++) {
			mballs[k].x += mballs[k].vx;
			mballs[k].y += mballs[k].vy;
		}
	
	
		for (int i = 0; i < N_OF_SQ; i++) {
			for (int j = 0; j < N_OF_SQ; j++){

				int x0, x1, x2, x3;
				int y0, y1, y2, y3;

				x0 = i * SQ_SIZE;
				y0 = j * SQ_SIZE;

				x1 = (i + 1) * SQ_SIZE;
				y1 = j * SQ_SIZE;

				x2 = i * SQ_SIZE;
				y2 = (j + 1) * SQ_SIZE;

				x3 = (i + 1) * SQ_SIZE;
				y3 = (j + 1) * SQ_SIZE;


				ConvexShape convex = CalculateTheShape(x0, y0, x1, y1, x2, y2, x3, y3, mballs);
				window.draw(convex);
			}
		}
		

		window.display();
	}

	return 0;
}