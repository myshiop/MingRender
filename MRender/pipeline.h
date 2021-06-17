#pragma once
#include "macro.h"
#include "mathUtils.h"
#include "shader.h"
#include "win32.h"

const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

void rasterize(vec4* clipCoordAttri, unsigned char* framebuffer, float* zbuffer, Shader& shader);
void drawTriangles(unsigned char* framebuffer, float *zbuffer, Shader& shader, int nface);