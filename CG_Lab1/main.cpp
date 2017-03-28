// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDL.h"
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
int t;
float v = 0.005;

SDL_Surface* screen;
vector<vec3> stars(100);
vec3 whitecolor(1, 1, 1);
// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Update();
void Draw();
void Interpolate(vec3 a, vec3 b, vector<vec3>& result);


// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
	
	for (size_t s = 0; s < stars.size(); ++s)
	{
		stars[s].x = ((float(rand()) / float(RAND_MAX))*2-1);
		stars[s].y = ((float(rand()) / float(RAND_MAX))*2-1);
		stars[s].z = float(rand()) / float(RAND_MAX);
	}
	t = SDL_GetTicks();

	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}
//task1

	/*vector<vec3> result(4);
	vec3 a(1, 4, 9.2);
	vec3 b(4, 1, 9.8);
	Interpolate(a, b, result);
	for (int i = 0; i<result.size(); ++i)
	{
		cout << "( "
			<< result[i].x << ", "
			<< result[i].y << ", "
			<< result[i].z << " ) ";
	}
	int g;
	cin >> g;*/

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

//task 2
/* 
void Draw()
{
	vec3 topLeft(1, 0, 0); // red
	vec3 topRight(0, 0, 1); // blue
	vec3 bottomLeft(0, 1, 0); // green
	vec3 bottomRight(1, 1, 0); // yellow

	vector<vec3> leftSide(SCREEN_HEIGHT);
	vector<vec3> rightSide(SCREEN_HEIGHT);
	vector<vec3> TopDown(SCREEN_WIDTH);

	Interpolate(topLeft, bottomLeft, leftSide);
	Interpolate(topRight, bottomRight, rightSide);
	
	for (unsigned int y = 0; y < SCREEN_HEIGHT; y++) 
	{
		Interpolate(leftSide[y], rightSide[y], TopDown);
		
		for (unsigned int x = 0; x < SCREEN_WIDTH; x++)
		{
			PutPixelSDL(screen, x, y, TopDown[x]);
		}
	}
	
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
*/

//task 3
void Draw()
{

	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	//vector<float> rxf;
	//vector<float> ryf;
	//vector<float> rzf;
	float x2d, y2d;
	float f = SCREEN_HEIGHT / 2;

	//float xAxis, yAxis, zAxis;
	for (size_t s = 0; s<stars.size(); ++s)
	{
		
		x2d = f*stars[s].x / stars[s].z + SCREEN_WIDTH / 2;
		y2d = f*stars[s].y / stars[s].z + SCREEN_HEIGHT / 2;
		PutPixelSDL(screen, x2d, y2d, whitecolor);
		 
		
	}
	
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void Update()
{

	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	for (int i = 0; i < stars.size(); ++i)
	{
		stars[i].z = stars[i].z - v*dt;
		if (stars[i].z <= 0)
			stars[i].z += 1;
		if (stars[i].z > 1)
			stars[i].z -= 1;
	}
}

void Interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
	vec3 c;
	c.x = (b.x - a.x) / (result.size() - 1);
	c.y = (b.y - a.y) / (result.size() - 1);
	c.z = (b.z - a.z) / (result.size() - 1);
	result[0] = a;
	for (unsigned int i = 0; i < result.size(); ++i)
	{
		result[i + 1] = result[i] + c;
	}
}