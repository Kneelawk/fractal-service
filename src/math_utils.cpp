#include <cmath>
#include "math_utils.h"

// calculate rgb data from hsb data
// this was copied from the java library's java.awt.Color
RGBData fromHSB(float hue, float saturation, float brightness) {
	int r = 0, g = 0, b = 0;
	if (saturation == 0) {
		r = g = b = (int) (brightness * 255.0 + 0.5);
	} else {
		float h = (hue - std::floor(hue)) * 6.0f;
		float f = h - std::floor(h);
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));
		switch ((int) h) {
		case 0:
			r = (int) (brightness * 255.0f + 0.5f);
			g = (int) (t * 255.0f + 0.5f);
			b = (int) (p * 255.0f + 0.5f);
			break;
		case 1:
			r = (int) (q * 255.0f + 0.5f);
			g = (int) (brightness * 255.0f + 0.5f);
			b = (int) (p * 255.0f + 0.5f);
			break;
		case 2:
			r = (int) (p * 255.0f + 0.5f);
			g = (int) (brightness * 255.0f + 0.5f);
			b = (int) (t * 255.0f + 0.5f);
			break;
		case 3:
			r = (int) (p * 255.0f + 0.5f);
			g = (int) (q * 255.0f + 0.5f);
			b = (int) (brightness * 255.0f + 0.5f);
			break;
		case 4:
			r = (int) (t * 255.0f + 0.5f);
			g = (int) (p * 255.0f + 0.5f);
			b = (int) (brightness * 255.0f + 0.5f);
			break;
		case 5:
			r = (int) (brightness * 255.0f + 0.5f);
			g = (int) (p * 255.0f + 0.5f);
			b = (int) (q * 255.0f + 0.5f);
			break;
		}
	}
	RGBData data;
	data.r = (unsigned char) r;
	data.g = (unsigned char) g;
	data.b = (unsigned char) b;
	return data;
}

float mod2(float value, float min, float max) {
	float size = max - min;
	if (size > 0) {
		float quot = (value - min) / size;
		quot -= std::floor(quot);
		return quot * size + min;
	} else {
		return min;
	}
}
