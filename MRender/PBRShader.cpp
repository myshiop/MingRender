#include "shader.h"
#include "sample.h"

static float GGX_TR(float n_dot_h, float roughness) {
	float alpha2 = roughness * roughness;	
	float n_dot_h2 = n_dot_h * n_dot_h;
	float factor = n_dot_h2 * (alpha2 - 1) + 1;

	return alpha2 / (PI * factor * factor);
}

static float SchilcGGX(float n_dot_wo, float roughness) {
	float k_direct = ((roughness + 1) * (roughness + 1)) / 8;
	return n_dot_wo / (n_dot_wo * (1 - k_direct) + k_direct);
}

static float geometry_Smith(float n_dot_wo, float n_dot_wi, float roughnees) {
	float g1 = SchilcGGX(n_dot_wo, roughnees);
	float g2 = SchilcGGX(n_dot_wi, roughnees);

	return g1 * g2;
}

static vec3 fresnelSchilk(float h_dot_wo, vec3& F0, float roughness) {
	float r = 1.0f - roughness;
	if (r < F0[0])
		r = F0[0];
	return F0 + (vec3(r, r, r) - F0) * pow(1 - h_dot_wo, 5.0f);
}

static float float_aces(float value)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	value = (value * (a * value + b)) / (value * (c * value + d) + e);
	return float_clamp(value, 0, 1);
}

static vec3 Reinhard_mapping(vec3& color)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		color[i] = float_aces(color[i]);
		//color[i] = color[i] / (color[i] + 0.5);
		color[i] = pow(color[i], 1.0 / 2.2);
	}
	return color;
}

void PBRShader::vertexShader(int nfaces, int nvertex) {
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

vec3 PBRShader::fragmentShader(float a, float b, float c) {
	vec3 CookTorrance_BRDF;
	vec3 lightPos = vec3(2, 1.5, 5);
	vec3 radiance = vec3(3, 3, 3);

	vec4* clipCoords = payload.clipCoordAttri;
	vec3* worldCoords = payload.worldCoordAttri;
	vec3* normals = payload.normalAttri;
	vec2* uvs = payload.uvAttri;
	vec3 barycentriCoordinates(a, b, c);

	//²åÖµÊôÐÔ
	vec3 normal = interpolateAttribute(normals, clipCoords, barycentriCoordinates);
	vec2 uv = interpolateAttribute(uvs, clipCoords, barycentriCoordinates);
	vec3 worldpos = interpolateAttribute(worldCoords, clipCoords, barycentriCoordinates);
	
	vec3 wi = (lightPos - worldpos).normalized();
	vec3 n = normal.normalized();
	vec3 wo = (payload.camera->eye - worldpos).normalized();
	vec3 h = (wi + wo).normalized();

	float n_dot_wi = float_max(dot(n, -wi), 0);
	vec3 color(0, 0, 0);
	
	float n_dot_wo = float_max(dot(n, wo), 0);
	float n_dot_h = float_max(dot(n, h), 0);
	float h_dot_wo = float_max(dot(h, wo), 0);

	//float roughness = payload.model->roughness(uv);
	//float metallicness = payload.model->metalness(uv);

	float roughness = 0.04;
	float metallicness = 0.01;

	float NDF = GGX_TR(n_dot_h, roughness);
	float G = geometry_Smith(n_dot_wo, n_dot_wi, roughness);
	
	vec3 albedo = payload.model->diffuse(uv);
	vec3 defaltF0 = vec3(0.04, 0.04, 0.04);
	vec3 F0 = vec3_lerp(defaltF0, albedo, metallicness);

	vec3 F = fresnelSchilk(h_dot_wo, F0, roughness);
	vec3 kd = (vec3(1.0, 1.0, 1.0) - F) * (1 - metallicness);

	CookTorrance_BRDF = NDF * G * F / (4.0 * n_dot_wi * n_dot_wo + 0.0001);

	vec3 Lo = ((kd * albedo / PI) + CookTorrance_BRDF) * radiance * n_dot_wi;

	vec3 ambient = 0.05 * albedo;

	color = Lo + ambient;

	Reinhard_mapping(color);

	return color * 255.f;
}