#include "pipeline.h"

extern window_t* window;

enum clipPlane
{
	W_PLANE,
	X_RIGHT,
	X_LEFT,
	Y_TOP,
	Y_BOTTOM,
	Z_NEAR,
	Z_FAR,
};

//v[0],v[1],v[2]代表A、B、C三点
static vec3 computeBarycentric(float x, float y, const vec3* v) {
	vec3 s[2];
	s[0] = vec3(v[1][0] - v[0][0], v[2][0] - v[0][0], v[0][0] - x);
	s[1] = vec3(v[1][1] - v[0][1], v[2][1] - v[0][1], v[0][1] - y);
	vec3 u = cross(s[0], s[1]);
	if (abs(u.z()) > EPSILON) {
		return vec3(1.0 - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
	}
	return vec3(-1, -1, -1);
}

static vec3 compute_barycentric2D(float x, float y, const vec3* v)
{
	float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
	float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
	return vec3(c1, c2, 1 - c1 - c2);
}

static bool isInsideTriangle(float a, float b, float c) {
	if (a > -EPSILON && b > -EPSILON && c > -EPSILON) {
		return true;
	}
	return false;

}

static int getIndex(int x, int y) {
	return (WINDOW_HEIGHT - y - 1) * WINDOW_WIDTH + x;
}

static void setColor(unsigned char* framebuffer, int x, int y, unsigned char color[]) {
	int index = index = ((WINDOW_HEIGHT - y - 1) * WINDOW_WIDTH + x) * 4;
	for (int i = 0; i < 3; ++i) {
		framebuffer[index + i] = color[i];
	}
}

static int isInsidePlane(clipPlane plane, vec4 vertex) {
	switch (plane)
	{
	case W_PLANE:
		return vertex.w() <= -EPSILON;
	case X_RIGHT:
		return vertex.x() >= vertex.w();
	case X_LEFT:
		return vertex.x() <= -vertex.w();
	case Y_TOP:
		return vertex.y() >= vertex.w();
	case Y_BOTTOM:
		return vertex.y() <= -vertex.w();
	case Z_NEAR:
		return vertex.z() >= vertex.w();
	case Z_FAR:
		return vertex.z() <= -vertex.w();
	default:
		return 0;
	}
}

static float getIntersectRatio(vec4 prev, vec4 curv, clipPlane plane) {
	switch (plane)
	{
	case W_PLANE:
		return (prev.w() + EPSILON) / (prev.w() - curv.w());
	case X_RIGHT:
		return (prev.w() - prev.x()) / ((prev.w() - prev.x()) - (curv.w() - curv.x()));
	case X_LEFT:
		return (prev.w() + prev.x()) / ((prev.w() + prev.x()) - (curv.w() + curv.x()));
	case Y_TOP:
		return (prev.w() - prev.y()) / ((prev.w() - prev.y()) - (curv.w() - curv.y()));
	case Y_BOTTOM:
		return (prev.w() + prev.y()) / ((prev.w() + prev.y()) - (curv.w() + curv.y()));
	case Z_NEAR:
		return (prev.w() - prev.z()) / ((prev.w() - prev.z()) - (curv.w() - curv.z()));
	case Z_FAR:
		return (prev.w() + prev.z()) / ((prev.w() + prev.z()) - (curv.w() + curv.z()));
	default:
		return 0;
	}
}

static int clipWithPlane(clipPlane plane, int num_vert, payload_t& payload) {
	int in_vert_num = 0; 
	int previous_index, current_index;
	int is_odd = (plane + 1) % 2;

	vec4* in_clipcoord = is_odd ? payload.inClipCoord : payload.outClipCoord;
	vec3* in_worldcoord = is_odd ? payload.inWorldCoord : payload.outWorldCoord;
	vec3* in_normal = is_odd ? payload.inNormal : payload.outNormal;
	vec2* in_uv = is_odd ? payload.inUV : payload.outUV;
	vec4* out_clipcoord = is_odd ? payload.outClipCoord : payload.inClipCoord;
	vec3* out_worldcoord = is_odd ? payload.outWorldCoord : payload.inWorldCoord;
	vec3* out_normal = is_odd ? payload.outNormal : payload.inNormal;
	vec2* out_uv = is_odd ? payload.outUV : payload.inUV;

	for (int i = 0; i < num_vert; ++i) {
		current_index = i;
		previous_index = (i - 1 + num_vert) % num_vert;
		vec4 pre_vertex = in_clipcoord[previous_index]; //边的起点
		vec4 cur_vertex = in_clipcoord[current_index]; //边的终点

		//判断边与平面是否有交点
		int is_cur_inside = isInsidePlane(plane, cur_vertex); 
		int is_pre_inside = isInsidePlane(plane, pre_vertex);

		if (is_cur_inside != is_pre_inside) {
			float ratio = getIntersectRatio(pre_vertex, cur_vertex, plane);

			out_clipcoord[in_vert_num] = vec4_lerp(pre_vertex, cur_vertex, ratio);
			out_worldcoord[in_vert_num] = vec3_lerp(in_worldcoord[previous_index], in_worldcoord[current_index], ratio);
			out_normal[in_vert_num] = vec3_lerp(in_normal[previous_index], in_normal[current_index], ratio);
			out_uv[in_vert_num] = vec2_lerp(in_uv[previous_index], in_uv[current_index], ratio);

			in_vert_num++;
		}

		//如果边的终点就在平面内侧，就直接送入
		if (is_cur_inside) {
			out_clipcoord[in_vert_num] = cur_vertex;
			out_worldcoord[in_vert_num] = in_worldcoord[current_index];
			out_normal[in_vert_num] = in_normal[current_index];
			out_uv[in_vert_num] = in_uv[current_index];

			in_vert_num++;
		}

	}
	return in_vert_num;
}

static int homogeneousClipping(payload_t& payload) {
	int num_vertex = 3;
	num_vertex = clipWithPlane(W_PLANE, num_vertex, payload);
	num_vertex = clipWithPlane(X_RIGHT, num_vertex, payload);
	num_vertex = clipWithPlane(X_LEFT, num_vertex, payload);
	num_vertex = clipWithPlane(Y_TOP, num_vertex, payload);
	num_vertex = clipWithPlane(Y_BOTTOM, num_vertex, payload);
	num_vertex = clipWithPlane(Z_NEAR, num_vertex, payload);
	num_vertex = clipWithPlane(Z_FAR, num_vertex, payload);
	
	int a = num_vertex;

	return num_vertex;
}

static void transformAttri(payload_t& payload, int index0, int index1, int index2) {
	payload.clipCoordAttri[0] = payload.outClipCoord[index0];
	payload.clipCoordAttri[1] = payload.outClipCoord[index1];
	payload.clipCoordAttri[2] = payload.outClipCoord[index2];
	payload.worldCoordAttri[0] = payload.outWorldCoord[index0];
	payload.worldCoordAttri[1] = payload.outWorldCoord[index1];
	payload.worldCoordAttri[2] = payload.outWorldCoord[index2];
	payload.normalAttri[0] = payload.outNormal[index0];
	payload.normalAttri[1] = payload.outNormal[index1];
	payload.normalAttri[2] = payload.outNormal[index2];
	payload.uvAttri[0] = payload.outUV[index0];
	payload.uvAttri[1] = payload.outUV[index1];
	payload.uvAttri[2] = payload.outUV[index2];
}

void rasterize(vec4* clipCoordAttri, unsigned char* framebuffer, float* zbuffer, Shader& shader) {
	vec3 ndc_pos[3];
	vec3 screen_pos[3];
	unsigned char colorClamp[3];
	int width = window->width;
	int height = window->height;

	//得到ndc齐次坐标
	for (int i = 0; i < 3; ++i) {
		ndc_pos[i][0] = clipCoordAttri[i][0] / clipCoordAttri[i].w();
		ndc_pos[i][1] = clipCoordAttri[i][1] / clipCoordAttri[i].w();
		ndc_pos[i][2] = clipCoordAttri[i][2] / clipCoordAttri[i].w();
	}

	//得到屏幕坐标
	for (int i = 0; i < 3; ++i) {
		screen_pos[i][0] = (ndc_pos[i][0] + 1.0) * 0.5 * (width - 1); 
		screen_pos[i][1] = (ndc_pos[i][1] + 1.0) * 0.5 * (height - 1); 
		screen_pos[i][2] = -clipCoordAttri[i].w(); //裁剪空间w分量就是观察空间的-z
	}

	//得到三角形包围盒
	float xmin = 10000, xmax = -10000, ymin = 10000, ymax = -10000;
	for (int i = 0; i < 3; ++i) {
		xmin = float_min(xmin, screen_pos[i][0]);
		xmax = float_max(xmax, screen_pos[i][0]);
		ymin = float_min(ymin, screen_pos[i][1]);
		ymax = float_max(ymax, screen_pos[i][1]);
	}

	//光栅化
	for (int x = (int)xmin; x <= (int)xmax; ++x) {
		for (int y = (int)ymin; y <= (int)ymax; ++y) {
			vec3 barycentric = compute_barycentric2D((float)(x + 0.5), (float)(y + 0.5), screen_pos);
			float a = barycentric.x();
			float b = barycentric.y();
			float c = barycentric.z();
			if (isInsideTriangle(a, b, c)) {
				int index = getIndex(x, y); //用来索引zbuffer
				float normalizer = 1.0 / (a / clipCoordAttri[0].w() + b / clipCoordAttri[1].w() + c / clipCoordAttri[2].w());
				//得到当前像素点的深度
				float z = (a * screen_pos[0].z() / clipCoordAttri[0].w() + b * screen_pos[1].z() / clipCoordAttri[1].w() 
					+ c * screen_pos[2].z() / clipCoordAttri[2].w()) * normalizer;

				if (zbuffer[index] > z) {
					zbuffer[index] = z;
					vec3 color = shader.fragmentShader(a, b, c);

					for (int i = 0; i < 3; ++i) {
						colorClamp[i] = (int)float_clamp(color[i], 0, 255);
					}

					setColor(framebuffer, x, y, colorClamp);
				}
			}
		}
	}
}

void drawTriangles(unsigned char* framebuffer, float *zbuffer, Shader& shader, int nface) {
	for (int i = 0; i < 3; ++i) {
		shader.vertexShader(nface, i);
	}

	int num_vertex = homogeneousClipping(shader.payload);

	for (int i = 0; i < num_vertex - 2; ++i) {
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;

		transformAttri(shader.payload, index0, index1, index2);

		rasterize(shader.payload.clipCoordAttri, framebuffer, zbuffer, shader);
	}
}