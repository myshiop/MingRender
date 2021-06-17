#include <ctime>
#include <iostream>

#include "macro.h"
#include "mathUtils.h"
#include "win32.h"
#include "pipeline.h"
#include "scene.h"

using namespace std;

extern window_t *window;

const vec3 eye(0, 1, 5);
const vec3 up(0, 1, 0);
const vec3 target(0, 1, 0);

const scene_t Scenes[]{
	{"diablo", build_defualt_scene},
	{"diablo", build_PBR_scene},
	{"Marry", build_Marry_PBR_scene}
};

void clearFramebuffer(int width, int height, unsigned char* framebuffer) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int index = (i * width + j) * 4;

			framebuffer[index + 2] = 80;
			framebuffer[index + 1] = 56;
			framebuffer[index] = 56;
		}
	}
}

void clearZbuffer(int width, int height, float* zbuffer) {
	for (int i = 0; i < width * height; ++i) {
		zbuffer[i] = INT_MAX;
	}
}

void updateMatrix(Camera& camera, mat4 view, mat4 perspective, Shader* shader) {
	view = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 mvp = perspective * view;
	shader->payload.camera_view = view;
	shader->payload.mvp = mvp;
}

int main() 
{
	int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
	unsigned char* framebuffer = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4); //创建帧缓冲
	float* zbuffer = (float*)malloc(sizeof(float) * width * height); //创建深度缓冲
	memset(framebuffer, 0, sizeof(unsigned char) * width * height);

	//创建相机
	Camera camera(eye, target, up, (float)width / height);

	//设置mvp矩阵
	mat4 model = mat4::identity();
	mat4 view = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 perspective = mat4_perspective(60, (float)width / height, -0.1, -10000);

	srand((unsigned int)time(NULL)); //用时间来设置随机数种子
	int scene_index = 1;
	int model_num = 0;
	Model *sceneModel[MAX_MODEL_NUM]; //包含MAX_MODEL_NUM个指针的数组
	Shader *shader_model;

	Scenes[scene_index].build_scene(sceneModel, model_num, &shader_model, perspective, &camera);


	initWindow(width, height, "MRender");

	int num_frames = 0;
	float print_time = platformGetTime();

	while (!window->is_close) {
		float current_time = platformGetTime();

		//clear buffers
		clearFramebuffer(width, height, framebuffer);
		clearZbuffer(width, height, zbuffer);

		handleEvents(camera);
		updateMatrix(camera, view, perspective, shader_model);

		//绘制
		for (int i = 0; i < model_num; ++i) {
			shader_model->payload.model = sceneModel[i];

			Shader* shader = shader_model;
			
			for (int j = 0; j < sceneModel[i]->nfaces(); ++j) {
				drawTriangles(framebuffer, zbuffer, *shader, j);
			}
		}

		// calculate and display FPS
		num_frames += 1;
		if (current_time - print_time >= 1) {
			int sum_millis = (int)((current_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = current_time;
		}

		// reset mouse information
		window->mouse_info.wheel_delta = 0;
		window->mouse_info.orbit_delta = vec2(0, 0);
		window->mouse_info.fv_delta = vec2(0, 0);

		// send framebuffer to window 
		drawWindow(framebuffer);
		msgDispatch();

	}


	for (int i = 0; i < model_num; ++i) {
		if (sceneModel[i] != NULL) {
			delete sceneModel[i];
		}
	}
	if (shader_model != nullptr) {
		delete shader_model;
	}
	free(zbuffer);
	free(framebuffer);
	destroyWindow();

	system("pause");
	return 0;
}