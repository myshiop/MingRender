#pragma once
#include "macro.h"
#include "mathUtils.h"
#include "model.h"
#include "camera.h"

struct payload_t
{
	mat4 model_mat;
	mat4 camera_view;
	mat4 camera_perp;
	mat4 mvp;

	Camera *camera;
	Model *model;

	vec3 normalAttri[3];
	vec2 uvAttri[2];
	vec3 worldCoordAttri[3];
	vec4 clipCoordAttri[3];

	//homogeneous clipping
	vec3 inNormal[MAX_VERTEX];
	vec2 inUV[MAX_VERTEX];
	vec3 inWorldCoord[MAX_VERTEX];
	vec4 inClipCoord[MAX_VERTEX];
	vec3 outNormal[MAX_VERTEX];
	vec2 outUV[MAX_VERTEX];
	vec3 outWorldCoord[MAX_VERTEX];
	vec4 outClipCoord[MAX_VERTEX];
};

class Shader 
{
public:
	payload_t payload;
	virtual void vertexShader(int nfaces, int nvertex){}
	virtual vec3 fragmentShader(float a, float b, float c) { return vec3(0, 0, 0); }
};

class PhongShader : public Shader {
public:
	void vertexShader(int nfaces, int nvertex);
	vec3 fragmentShader(float a, float b, float c);
};

class PBRShader : public Shader {
public:
	void vertexShader(int nfacec, int vertex);
	vec3 fragmentShader(float a, float b, float c);
};