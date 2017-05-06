#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <algorithm>

using namespace std;
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES
const float PI = 3.141592653589f;
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
const int focalLength = SCREEN_HEIGHT;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
vec3 cameraPos(0, 0, -3.1);
vec3 currentColor;
vec3 currentNormal;
vec3 currentReflectance;
float yaw = 0.0f;
float pitch = 0.0f;
mat3 R;
float depthBuffer[SCREEN_WIDTH][SCREEN_HEIGHT];
vec3 lightPos(0, -0.5, -0.7);
vec3 lightPower = 1.1f*vec3(1, 1, 1);
vec3 indirectLightPowerPerArea = 0.5f*vec3(1, 1, 1);
// ----------------------------------------------------------------------------
// STRUCT
struct Pixel
{
	int x;
	int y;
	float zinv;
	//vec3 illumination;
	vec3 pos3d;
};
struct Vertex
{
	vec3 position;
	//vec3 normal;
	//vec2 reflectance;
	//float reflectance;
};
// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void VertexShader(const Vertex& v, Pixel& p);
void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color);
void DrawPolygonEdges(const vector<Vertex>& vertices);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void RotateX();
void RotateY();
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels);
void DrawPolygon(const vector<Vertex>& vertices);
void PixelShader(const Pixel& p);

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
		pitch += 0.05f;
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

	if (keystate[SDLK_RSHIFT])
		;

	if (keystate[SDLK_RCTRL])
		;

	if (keystate[SDLK_w])
		lightPos += forward;

	if (keystate[SDLK_s])
		lightPos -= forward;

	if (keystate[SDLK_d])
		lightPos += right;

	if (keystate[SDLK_a])
		lightPos -= right;

	if (keystate[SDLK_e])
		;

	if (keystate[SDLK_q])
		;

}

void Draw()
{
	//Clear the depth buffer
	for (int x = 0; x < SCREEN_WIDTH; ++x)
	{
		for (int y = 0; y < SCREEN_HEIGHT; ++y)
		{
			depthBuffer[x][y] = 0;
		}
	}
	SDL_FillRect(screen, 0, 0);

	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	for (int i = 0; i<triangles.size(); ++i)
	{
		currentColor = triangles[i].color;
		currentNormal = triangles[i].normal;
		currentReflectance = vec3(1, 1, 1);
		vector<Vertex> vertices(3);
		vertices[0].position = triangles[i].v0;
		vertices[1].position = triangles[i].v1;
		vertices[2].position = triangles[i].v2;
		//vertices[i].reflectance = 1.f;
		//vertices[i].normal;
		DrawPolygon(vertices);
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color)
{
	Pixel delta;
	delta.x = glm::abs(a.x - b.x);
	delta.y = glm::abs(a.y - b.y);

	int pixels = glm::max(delta.x, delta.y) + 1;

	vector<Pixel> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < line.size(); i++)
	{
		PixelShader(line[i]);
	}
}

void DrawPolygonEdges(const vector<Vertex>& vertices)
{
	int V = vertices.size();
	vector<Pixel> projectedVertices(V);
	for (int i = 0; i < V; i++)
	{
		VertexShader(vertices[i], projectedVertices[i]);
	}
	for (int i = 0; i < V; i++)
	{
		int j = (i + 1) % V;
		vec3 color(1, 1, 1);
		DrawLineSDL(screen, projectedVertices[i], projectedVertices[j], color);
	}
}

void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{

	int Max = -numeric_limits<int>::max();
	int Min = numeric_limits<int>::max();

	for (int i = 0; i < vertexPixels.size(); i++)
	{
		if (vertexPixels[i].y > Max)
		{
			Max = vertexPixels[i].y;
		}
		if (vertexPixels[i].y < Min)
		{
			Min = vertexPixels[i].y;
		}
	}
	int ROWS = Max - Min + 1;
	leftPixels.resize(ROWS);
	rightPixels.resize(ROWS);

	for (int i = 0; i < leftPixels.size(); i++)
	{
		leftPixels[i].x = SCREEN_WIDTH;
		leftPixels[i].y = Min + i;
		rightPixels[i].x = 0;
		rightPixels[i].y = Min + i;
	}

	for (int i = 0; i<vertexPixels.size(); i++)
	{
		Pixel p1 = vertexPixels[i];
		Pixel p2 = vertexPixels[(i + 1) % vertexPixels.size()];

		int steps = abs(p1.y - p2.y) + 1;
		vector<Pixel> edge = vector<Pixel>(steps);
		Interpolate(p1, p2, edge);

		for (Pixel p : edge)
		{
			int i = p.y - Min;
			if (p.x < leftPixels[i].x)
			{
				leftPixels[i].x = p.x;
				leftPixels[i].zinv = p.zinv;
				leftPixels[i].pos3d = p.pos3d;
				//leftPixels[i].step = p.step;
				//leftPixels[i].illumination = p.illumination;
			}

			if (p.x > rightPixels[i].x)
			{
				rightPixels[i].x = p.x;
				rightPixels[i].zinv = p.zinv;
				rightPixels[i].pos3d = p.pos3d;
				//rightPixels[i].step = p.step;
				//rightPixels[i].illumination = p.illumination;
			}
		}
	}
}

void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); i++)
	{
		DrawLineSDL(screen, leftPixels[i], rightPixels[i], currentColor);
	}
}

void DrawPolygon(const vector<Vertex>& vertices)
{
	int V = vertices.size();
	vector<Pixel> vertexPixels(V);
	for (int i = 0; i < V; i++)
		VertexShader(vertices[i], vertexPixels[i]);
	vector<Pixel> leftPixels;
	vector<Pixel> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
}

void VertexShader(const Vertex& v, Pixel& p)
{
	vec3 pos = (v.position - cameraPos)*R;
	p.zinv = 1 / pos.z;
	p.x = int(focalLength*pos.x*p.zinv) + SCREEN_WIDTH / 2;
	p.y = int(focalLength*pos.y*p.zinv) + SCREEN_HEIGHT / 2;
	p.pos3d = v.position;
	//float r = glm::length(p.pos3d - lightPos);
	//float d = glm::dot(v.normal, glm::normalize(lightPos - p.pos3d)) / (4.f * PI*r*r);
	//vec3 D = lightPower * max(d, 0.0f);

	//p.illumination =v.reflectance*(D + indirectLightPowerPerArea);
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result)
{
	int N = result.size();
	float stepX = float(b.x - a.x) / float(max(N - 1, 1));
	float stepY = float(b.y - a.y) / float(max(N - 1, 1));
	float stepZinv = float(b.zinv - a.zinv) / float(max(N - 1, 1));
	//vec3 lightStep = (b.illumination - a.illumination) / float(max(N - 1, 1));
	vec3 stepPos3d = (b.pos3d - a.pos3d) / float(max(N - 1, 1));

	Pixel current(a);
	for (int i = 0; i < N; ++i)
	{
		current.x = a.x + i*stepX;
		current.y = a.y + i*stepY;
		current.zinv = a.zinv + i*stepZinv;
		current.pos3d += stepPos3d;
		//current.illumination += lightStep;
		result[i] = current;
	}
}

void PixelShader(const Pixel& p)
{
	int x = p.x;
	int y = p.y;
	float r = glm::length(p.pos3d - lightPos);
	float d = glm::dot(currentNormal, glm::normalize(lightPos - p.pos3d)) / (4.f * PI*r*r);
	vec3 D = lightPower * max(d, 0.0f);

	vec3 illumination = currentReflectance*(D + indirectLightPowerPerArea);
	if (p.zinv >= depthBuffer[x][y])
	{
		depthBuffer[x][y] = p.zinv;

		PutPixelSDL(screen, x, y, illumination*currentColor);
	}
}

void RotateX()
{
	vec3 Row1(1, 0, 0);
	vec3 Row2(0, cos(pitch), -sin(pitch));
	vec3 Row3(0, sin(pitch), cos(pitch));
	R = mat3(Row1, Row2, Row3);
}

void RotateY()
{
	vec3 Row1(cos(yaw), 0, sin(yaw));
	vec3 Row2(0, 1, 0);
	vec3 Row3(-sin(yaw), 0, cos(yaw));
	R = mat3(Row1, Row2, Row3);
}
