#include "pch.h"
#include "ray.h"
#include "vec3.h"
#include "hitable.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include <iostream>
#include <stdlib.h>     
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "rectangle.h"

using namespace yph;
// cosnt 和 & 一定要注意起来，不要忘了各种位置的意义及区别

vec3f color(const ray<float>& r, hitable *world, int depth) {
	hitRecord rec;
	if (world->hit(r, T_MIN, T_MAX, rec)) {			// 这里下限是为了防止shadow acne，即微小光线也漫反射，无穷无尽下去
		ray<float> scattered;
		vec3f attenuation;
		vec3f emitted = rec.materialPtr->emitted(rec.u,rec.v,rec.p);
		// std::cout << "e" << emitted << std::endl; // ??为啥都是0
		if (depth < 50 && rec.materialPtr->scatter(r, rec, attenuation, scattered)) {
			return emitted + attenuation * color(scattered, world, depth + 1);		// 漫反射了0.5的。设置了一个迭代的最深深度，与之前的T_MIN一起作用
		}
		else {
			return emitted;
		}
	}
	else {
		return vec3f(0, 0, 0);
	}
}

hitable *simpleLight() {
	texture *pertext = new noiseTexture(4);
	hitable **list = new hitable*[4];
	list[0] = new sphere(vec3f(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3f(0, 2, 0), 2, new lambertian(pertext));
	list[2] = new sphere(vec3f(0, 7, 0), 2, new diffuseLight(new constantTexture(vec3f(4, 4, 4))));
	list[3] = new xyRectangle(3,5,1,3,-2,new diffuseLight(new constantTexture(vec3f(4,4,4))));
	return new hitableList(list, 4);
}

hitable *cornellBox() {
	hitable **list = new hitable*[6];
	int i = 0;
	material *red = new lambertian(new constantTexture(vec3f(0.65,0.05,0.05)));
	material *white = new lambertian(new constantTexture(vec3f(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constantTexture(vec3f(0.12, 0.45, 0.15)));
	material *light = new diffuseLight(new constantTexture(vec3f(150, 150, 150)));
	list[i++] = new flipNormals(new yzRectangle(0, 555, -555, 0, 555, light));
	list[i++] = new yzRectangle(0, 555, -555, 0, 0, light);
	list[i++] = new flipNormals(new xzRectangle(213, 343, -332, -227, 554, light));
	list[i++] = new flipNormals(new xzRectangle(0, 555, -555, 0, 555, light));
	list[i++] = new xzRectangle(0, 555, -555, 0, 0, light);//
	list[i++] = new xyRectangle(0, 555, 0, 555, -555, light);
	return new hitableList(list, i);
}

/*
			^y
			|  
			| 
		   /|------>x
	      /
		\/_  z
*/

#define RAND ((rand() % 10) / 10.0)

int main()
{
	FILE *stream;
	srand(time(NULL));

	int nx = 400;
	int ny = 200;
	int ns = 100;
	int nn;
						
	hitable* world = cornellBox();

	vec3f lookfrom(278, 278, 800);
	vec3f lookat(278, 278, 0);
	float distToFocus = 10.0;//(lookfrom - lookat).length();
	float aperture = 2.0;
	float vfov = 40.0;
	camera cam(lookfrom, lookat, vec3f(0, 1, 0), vfov, float(nx) / float(ny), aperture, distToFocus);

	freopen_s(&stream, "test.ppm", "w", stdout);
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";
	for (int j = ny - 1; j >= 0; --j) {												 //从左上角开始绘制
		for (int i = 0; i < nx; ++i) {
			vec3f col(0, 0, 0);
			for (int s = 0; s < ns; ++s) {
				// 一个像素照射多次，每次随机产生一个偏差防止总是取一个像素的一个位置造成颜色剧变和锯齿，对颜色求和后再求平均值
				float u = float(i + RAND) / float(nx);
				float v = float(j + RAND) / float(ny);
				ray<float> r = cam.getRay(u, v);
				col += color(r, world, 0);
			}
			col /= float(ns);
			col = vec3f(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));			// 这个是做gamma矫正，视觉与相机的偏差
			int ir = int(col[0] * 255.99);
			int ig = int(col[1] * 255.99);
			int ib = int(col[2] * 255.99);
			std::cout << ir << " " << ig << " " << ib << std::endl;
		}
	}
}
