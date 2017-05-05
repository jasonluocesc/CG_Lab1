#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <math.h>
#include <algorithm>

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const float PI = 3.141592653589f;
const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
const int focalLength = SCREEN_HEIGHT; //camera focalLength
SDL_Surface* screen;
int t; //Update 
vector<Triangle> triangles;
vec3 cameraPos(0, 0, -3); //camera position
vec3 Black(0, 0, 0);
vec3 White(1, 1, 1);//color
float m = std::numeric_limits<float>::max();

float yaw = 0.0f;
mat3 R;

vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f*vec3(1, 1, 1);
vec3 indirectLight = 0.5f*vec3(1, 1, 1);
// ----------------------------------------------------------------------------
// STRUCTS

struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles,
	Intersection& closestIntersection);
void RotateY();
void RotateX();
vec3 DirectLight(const Intersection& i);

int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.

	LoadTestModel(triangles);

	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	vec3 right(R[0][0], R[0][1], R[0][2]);
	vec3 down(R[1][0], R[1][1], R[1][2]);
	vec3 forward(R[2][0], R[2][1], R[2][2]);

	Uint8* keystate = SDL_GetKeyState(0);

	if (keystate[SDLK_UP])
	{
		yaw += 0.05f;
		RotateX();
	}
	if (keystate[SDLK_DOWN])
	{
		yaw -= 0.05f;
		RotateX();
	}
	if (keystate[SDLK_LEFT])
	{
		yaw += 0.05f;
		RotateY();
	}
	if (keystate[SDLK_RIGHT])
	{
		yaw -= 0.05f;
		RotateY();
	}

	if (keystate[SDLK_w])
		lightPos += forward;

	if (keystate[SDLK_s])
		lightPos -= forward;
}


void Draw()
{
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	Intersection intersection_ray;
	vec3 currentlight;
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			vec3 dir(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			//cameraPos = R*cameraPos;
			dir = R*dir;
			bool happened = ClosestIntersection(cameraPos, dir, triangles, intersection_ray);
			if (happened)
			{
				currentlight = DirectLight(intersection_ray)+indirectLight;
				PutPixelSDL(screen, x, y, currentlight*triangles[intersection_ray.triangleIndex].color);			
			}
				
			else
				PutPixelSDL(screen, x, y, Black);

		}
	}




	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles,
	Intersection& closestIntersection)
{

	bool ifIntersect = false;
	float distanceMin = m;
	for (int i = 0; i < triangles.size(); i++)
	{
		vec3 v0 = triangles[i].v0;
		vec3 v1 = triangles[i].v1;
		vec3 v2 = triangles[i].v2;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 b = start - v0;
		mat3 A(-dir, e1, e2);
		vec3 x = glm::inverse(A)*b;
		
		if (x.x >= 0 && x.y >= 0 && x.z >= 0 && (x.y + x.z) <= 1)
		{
			if (x.x < distanceMin)
			{
				distanceMin = x.x;
				closestIntersection.distance = x.x;
				closestIntersection.position = v0 + x.y*e1 + x.z*e2;
				closestIntersection.triangleIndex = i;
			}
			ifIntersect = true;
		}

	}
	return ifIntersect;
}

void RotateX()
{
	vec3 Row1(1, 0, 0);
	vec3 Row2(0, cos(yaw), -sin(yaw));
	vec3 Row3(0, sin(yaw), cos(yaw));
	R = mat3(Row1, Row2, Row3);
}

void RotateY()
{
	vec3 Row1(cos(yaw), 0, sin(yaw));
	vec3 Row2(0, 1, 0);
	vec3 Row3(-sin(yaw), 0, cos(yaw));
	R = mat3(Row1, Row2, Row3);
}

vec3 DirectLight(const Intersection&i)
{
	float r = glm::length(i.position - lightPos);
	Intersection intersection_light;
	vec3 lightDirection = glm::normalize(i.position - lightPos);
	bool happened_light = ClosestIntersection(lightPos, lightDirection, triangles, intersection_light);

	vec3 normal = triangles[i.triangleIndex].normal;
	vec3 rvector = glm::normalize(lightPos - i.position);
	float projection = glm::dot(normal, rvector);

	if (happened_light && (intersection_light.distance < r-0.0001f))
	{
		return vec3(0, 0, 0);
	}
	else
	{
		float p = (max(projection, 0.0f) / (4.f*PI*pow(r, 2)));
		return lightColor*p;
	}
	
}

