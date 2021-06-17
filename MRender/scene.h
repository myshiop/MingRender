#pragma once
#include "tgaImage.h"
#include "shader.h"

struct scene_t
{
	const char* sceneName;
	void(*build_scene)(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera);
};

TGAImage* textureFormFile(const char *fileName);
void build_defualt_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera);
void build_PBR_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera);
void build_Marry_PBR_scene(Model** model, int& m, Shader** shader_use, mat4 perspective, Camera* camera);