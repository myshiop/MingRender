#pragma once
#include "mathUtils.h"

class Camera
{
public:
	Camera(vec3 e, vec3 t, vec3 up, float aspect);
	~Camera();

	vec3 eye;
	vec3 target;
	vec3 up;
	vec3 x;
	vec3 y;
	vec3 z;
	float aspect;
};

//handle event
void updateCameraPos(Camera& camera);
void handleEvents(Camera& camera);