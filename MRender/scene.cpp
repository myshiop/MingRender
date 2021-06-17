#include "scene.h"

TGAImage* textureFormFile(const char* fileName) {
	TGAImage* texture = new TGAImage();
	texture->read_tga_file(fileName);
	texture->flip_vertically();
	return texture;
}

void build_defualt_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera) {
	m = 1; //模型中的构件数
	const char* modelname[] =
	{
		"../MRender/obj/diablo/diablo3_pose.obj",
	};

	PhongShader* shader_phong = new PhongShader();

	int vertex = 0, face = 0;
	const char* sceneName = "Diablo";
	for (int i = 0; i < m; ++i) {
		model[i] = new Model(modelname[0], 0);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	printf("scene name:%s\n", sceneName);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_PBR_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera) {
	m = 1; //模型中的构件数
	const char* modelname[] =
	{
		"../MRender/obj/diablo/diablo3_pose.obj",
	};

	PBRShader* pbrShader = new PBRShader();

	int vertex = 0, face = 0;
	const char* sceneName = "Diablo";
	for (int i = 0; i < m; ++i) {
		model[i] = new Model(modelname[0], 0);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	pbrShader->payload.camera_perp = perspective;
	pbrShader->payload.camera = camera;

	*shader_use = pbrShader;
	printf("scene name:%s\n", sceneName);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_Marry_PBR_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera) {
	m = 1; //模型中的构件数
	const char* modelname[] =
	{
		"../MRender/obj/Marry/Marry.obj",
	};

	PBRShader* pbrShader = new PBRShader();

	int vertex = 0, face = 0;
	const char* sceneName = "Marry";
	for (int i = 0; i < m; ++i) {
		model[i] = new Model(modelname[0], 0);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	pbrShader->payload.camera_perp = perspective;
	pbrShader->payload.camera = camera;

	*shader_use = pbrShader;
	printf("scene name:%s\n", sceneName);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}