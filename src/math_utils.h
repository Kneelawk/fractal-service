#ifndef COLOR_UTILS_H_
#define COLOR_UTILS_H_

typedef struct {
	char r;
	char g;
	char b;
} RGBData;

RGBData fromHSB(float hue, float saturation, float brightness);

float mod2(float value, float min, float max);

#endif
