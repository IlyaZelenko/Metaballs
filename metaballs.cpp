#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace sf;


#define SQ_SIZE 8				// square size
#define R 80					// radius of metaball
#define N 1024					// width and height of the screen (512x512 pixels)
#define INDEX(x, y) (x+y*N)		// just because I wanna use 1-dimentional array and macros for it
#define N_OF_SQ N/SQ_SIZE		// number of squares
#define M_BALLS 8
#define CIRC_BOUND (N/2 - 100)

float RandPos() {	//random position
	return rand()%501 + N/2 - 250;
}

float RandVel() { //random velocity
	return rand()%2 + rand()%1000*.001 - .5;
}

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

float Q_rsqrt(float number)		// quake III quick reverse square root, not necessary but beautiful
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
	float r;			//radius
};

float InvDist(Metaball m, float x, float y){	//Inverse Distance
	float dist_sqared = (m.x - x) * (m.x - x) + (m.y - y) * (m.y - y);
	float inv_sqrt = Q_rsqrt(dist_sqared);
	return m.r * inv_sqrt;
}

float Linterp(int x0, int x1, float v0, float v1) {		//Linear Interpolation
	return (x1 - x0)*(1 - v0)/(v1 - v0) + x0;
}

void BounceTheBoundary(Metaball* mball) {
	float x = mball->x - N/2;						//centered the origin
	float y = -(mball->y - N/2);						
	Vector2f v0 = { mball->vx, -mball->vy };		//inverting y so it pointing upwards

	//std::cout << "centered position: (" << x << ", " << y << ")\n";
	//std::cout << "inverted velocity: (" << v0.x << ", " << v0.y << ")\n";
	float dot_prod = (x * v0.x + y * v0.y);
	float cross_prod = (x * v0.y - y * v0.x);
	//float cos_a = (x*v0.x + y*v0.y)/(sqrt(x*x + y*y) * sqrt(v0.x*v0.x + v0.y*v0.y));			//a * b = |a||b|cos(alpha)
	//float sin_a = (x*v0.y - y*v0.x)/(sqrt(x * x + y * y) * sqrt(v0.x * v0.x + v0.y * v0.y));	//a x b = |a||b|sin(alpha)
	float dot_prod2 = dot_prod * dot_prod;			//dot product squared
	float cross_prod2 = cross_prod * cross_prod;	//cross product squared
	float cos2_a = dot_prod2 / (dot_prod2 + cross_prod2);		//cos^2(alpha)  got from: a * b = |a||b|cos(alpha)
	float sin2_a = cross_prod2 / (dot_prod2 + cross_prod2);		//sin^2(alpha)  got from: a x b = |a||b|sin(alpha)

	float cos_2a = cos2_a - sin2_a;				//cos(2*alpha)
	float sin_2a = 2 * sqrt(sin2_a*cos2_a);		//sin(2*alpha)
	//std::cout << "cos2a: " << cos_2a << "\n";
	//std::cout << "sin2a: " << sin_2a << "\n";
	v0.x = -v0.x;
	v0.y = -v0.y;
	//std::cout <<"new velocity: (" << v0.x * cos_2a + v0.y * sin_2a << ", " << v0.x * -sin_2a + v0.y * cos_2a << ")\n";

	mball->vx = v0.x * cos_2a + v0.y * sin_2a*sign(cross_prod);
	mball->vy = -(v0.x * -sin_2a*sign(cross_prod) + v0.y * cos_2a);

	float d = x * x + y * y;
	float r = CIRC_BOUND - mball->r;

	if (d > r * r) {
		mball->x = r * x / sqrt(d) + N / 2;
		mball->y = -r * y / sqrt(d) + N / 2;
	}
}

ConvexShape CalculateTheShape(int x0, int y0, Metaball metaballs[]) {  //	~//~	MARCHING SQUARES	~//~
	int x1 = x0 + SQ_SIZE, y1 = y0,									   //	(x0, y0)--------a-------(x1, y1)
		x2 = x0			 , y2 = y0 + SQ_SIZE,						   //		|						|
		x3 = x1  	     , y3 = y2;									   //		|						|
																	   //		|						|
	float f0 = 0, f1 = 0, f2 = 0, f3 = 0;							   //		b						c
	for (int k = 0; k < M_BALLS; k++) {								   //		|						|
		f0 += InvDist(metaballs[k], x0, y0);						   //		|						|
		f1 += InvDist(metaballs[k], x1, y1);						   //		|						|
		f2 += InvDist(metaballs[k], x2, y2);						   //	(x2, y2)--------d-------(x3, y3)
		f3 += InvDist(metaballs[k], x3, y3);
	}
	
	Vector2f a, b, c, d;
	a = Vector2f(Linterp(x0, x1, f0, f1), y0);		//Linterp(x0, x1, f(x0, y0), f(x1 , y1))
	b = Vector2f(x0, Linterp(y0, y2, f0, f2));
	c = Vector2f(x1, Linterp(y1, y3, f1, f3));
	d = Vector2f(Linterp(x2, x3, f2, f3), y3);

	f0 = f0 >= 1;
	f1 = f1 >= 1;
	f2 = f2 >= 1;
	f3 = f3 >= 1;
	int mapped = f0 + 2 * f1 + 4 * f2 + 8 * f3;

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
		convex.setPoint(1, c);
		convex.setPoint(2, a);
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
		convex.setPoint(2, Vector2f(x3, y3));
		convex.setPoint(3, d);
		convex.setPoint(4, b);
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

	srand(time(0));
	Metaball mballs[M_BALLS] = {{RandPos(), RandPos(),  RandVel(),  RandVel(),	2*R/3.f},
								{RandPos(), RandPos(),  RandVel(),	RandVel(),	  R/2.f},
								{RandPos(), RandPos(),  RandVel(),  RandVel(),	  R/3.f},
								{RandPos(), RandPos(),  RandVel(),	RandVel(),	  R/4.f},
								{RandPos(), RandPos(),  RandVel(),  RandVel(),	  R/5.f},
								{RandPos(), RandPos(),  RandVel(),	RandVel(),	  R/6.f},
								{RandPos(), RandPos(),  RandVel(),  RandVel(),	  R/7.f},
								{RandPos(), RandPos(),  RandVel(),	RandVel(),	  R/8.f}};
									

	bool debug_info = 0;
	RenderWindow window(VideoMode(N, N), "Metaballs");

	while (window.isOpen()) {
		Event event;
		
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == Keyboard::D)
				{
					debug_info = debug_info ^ bool(1);	//toggle the debug for boundary
				}
			}
		}
		window.clear();



		

	
	
		for (int k = 0; k < M_BALLS; k++) {		//Movement
			mballs[k].x += mballs[k].vx;
			mballs[k].y += mballs[k].vy;
		}
	
	
		for (int i = 0; i < N_OF_SQ; i++) {
			for (int j = 0; j < N_OF_SQ; j++){

				int x0 = i* SQ_SIZE;
				int y0 = j * SQ_SIZE;

				ConvexShape convex = CalculateTheShape(x0, y0, mballs);
				window.draw(convex);
			}
		}

		for (int k = 0; k < M_BALLS; k++) {			//circular boundary
			float x = mballs[k].x - (N/2);
			float y = mballs[k].y - (N/2);
			float r = CIRC_BOUND - mballs[k].r;
			if (x*x + y*y > r*r) {
				BounceTheBoundary(&mballs[k]);
			}
		}

		if (debug_info) {				//	Debug information
			for (int i = 0; i < M_BALLS; i++) {
				CircleShape mb_crcls(mballs[i].r, 100);
				mb_crcls.setOrigin(mballs[i].r, mballs[i].r);
				mb_crcls.setFillColor(Color(0, 0, 0, 0));
				mb_crcls.setOutlineThickness(1);
				mb_crcls.setOutlineColor(Color(255, 255, 255, 255));
				mb_crcls.setPosition(mballs[i].x, mballs[i].y);
				window.draw(mb_crcls);
			}
			CircleShape crcl(CIRC_BOUND, 500);
			crcl.setOrigin(CIRC_BOUND, CIRC_BOUND);
			crcl.setFillColor(Color(0, 0, 0, 0));
			crcl.setOutlineThickness(1);
			crcl.setOutlineColor(Color(255, 255, 255, 255));
			crcl.setPosition((float)(N / 2), (float)(N / 2));
			window.draw(crcl);
		}
		


		window.display();
	}

	return 0;
}