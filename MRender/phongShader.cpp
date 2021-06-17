#include "shader.h"
#include "sample.h"

static vec3 calNormal(vec3& normal, vec3* worldCoords, const vec2* uvs, const vec2 &uv, TGAImage* normalMap) {
	float u1Delta = uvs[1][0] - uvs[0][0];
	float v1Delta = uvs[1][1] - uvs[0][1];
	float u2Delta = uvs[2][1] - uvs[0][1];
	float v2Delta = uvs[2][1] - uvs[0][1];
	float det = (u2Delta * v1Delta - v2Delta * u1Delta);

	vec3 e1 = worldCoords[1] - worldCoords[0];
	vec3 e2 = worldCoords[2] - worldCoords[0];

	vec3 tangent = v1Delta * e2 - e1 * v2Delta;
	vec3 bitangent = -u1Delta * e2 + u2Delta * e1;
	tangent /= det;
	bitangent /= det;

	//斯密特正交化
	normal = unit_vector(normal);
	tangent = unit_vector(tangent - dot(tangent, normal) * normal);
	bitangent = unit_vector(bitangent - dot(bitangent, normal) * normal - dot(bitangent, tangent) * tangent);

	vec3 sample = textureSample(uv, normalMap);
	sample = vec3(sample[0] * 2 - 1, sample[1] * 2 - 1, sample[2] * 2 - 1);

	vec3 normal_new = tangent * sample[0] + bitangent * sample[1] * normal * sample[2];
	return normal_new;
}

void PhongShader::vertexShader(int nfaces, int nvertex) {
	vec4 tempVert = to_vec4(payload.model->vert(nfaces, nvertex), 1.0f);
	vec4 tempNormal = to_vec4(payload.model->normal(nfaces, nvertex), 1.0f);

	payload.uvAttri[nvertex] = payload.model->uv(nfaces, nvertex);
	payload.inUV[nvertex] = payload.uvAttri[nvertex];
	payload.clipCoordAttri[nvertex] = payload.mvp * tempVert; 
	payload.inClipCoord[nvertex] = payload.clipCoordAttri[nvertex];

	for (int i = 0; i < 3; ++i) {
		payload.worldCoordAttri[nvertex][i] = tempVert[i];
		payload.inWorldCoord[nvertex][i] = tempVert[i];
		payload.normalAttri[nvertex][i] = tempNormal[i];
		payload.inNormal[nvertex][i] = tempNormal[i];
	}
}

vec3 PhongShader::fragmentShader(float a, float b, float c) {
	vec4* clipCoords = payload.clipCoordAttri;
	vec3* worldCoords = payload.worldCoordAttri;
	vec3* normals = payload.normalAttri;
	vec2* uvs = payload.uvAttri;
	vec3 barycentriCoordinates(a, b, c);

	//插值属性
	vec3 normal = interpolateAttribute(normals, clipCoords, barycentriCoordinates);
	vec2 uv = interpolateAttribute(uvs, clipCoords, barycentriCoordinates);
	vec3 worldpos = interpolateAttribute(worldCoords, clipCoords, barycentriCoordinates);

	if (payload.model->normalmap) {
		normal = calNormal(normal, worldCoords, uvs, uv, payload.model->normalmap);
	}

	vec3 ka(0.35, 0.35, 0.35);
	vec3 kd = payload.model->diffuse(uv);
	vec3 ks(0.8, 0.8, 0.8);

	//设置光源
	float p = 150.0f;
	vec3 l = unit_vector(vec3(1, 1, 1));
	vec3 light_ambient_intensity = kd;
	vec3 light_diffuse_intensity = vec3(0.9, 0.9, 0.9);
	vec3 light_specular_intensity = vec3(0.15, 0.15, 0.15);

	vec3 resColor(0, 0, 0);
	vec3 ambient, diffuse, specular;
	normal = unit_vector(normal);
	vec3 v = unit_vector(payload.camera->eye - worldpos);
	vec3 h = unit_vector(l + v);

	ambient = cwise_product(ka, light_ambient_intensity);
	diffuse = cwise_product(kd, light_diffuse_intensity * float_max(0, dot(l, normal)));
	specular = cwise_product(ks, light_specular_intensity * float_max(0, pow(dot(normal, h), p)));

	resColor = ambient + diffuse + specular;
	return resColor * 255.f;
}