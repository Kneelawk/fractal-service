#include <png.h>
#include <cstdio>
#include <iostream>
#include "FractalGenerator.h"
#include "math_utils.h"

/*
 * Public member functions
 */

FractalGenerator::FractalGenerator(fractalId id, v8::Isolate *isolate,
		void (*doneCallback)(v8::Isolate *isolate, std::string path,
				GenerationState state, void *doneCallbackData),
		std::string path, void *doneCallbackData, unsigned int width,
		unsigned int height, double fractalWidth, double fractalHeight,
		double fractalX, double fractalY, unsigned int iterations) :
		id(id), doneCallback(doneCallback), path(path), doneCallbackData(
				doneCallbackData), width(width), height(height), fractalWidth(
				fractalWidth), fractalHeight(fractalHeight), fractalX(fractalX), fractalY(
				fractalY), iterations(iterations) {

	doneAsync = new uv_async_t;
	doneAsync->data = this;
	uv_loop_t *loop = uv_default_loop();
	uv_async_init(loop, doneAsync, doneAsyncCallback);

	progress.store(0);
	state.store(INITIALIZING);
}

FractalGenerator::~FractalGenerator() {
	if (deleteCallback) {
		deleteCallback(this, deleteCallbackData);
	}

	delete doneAsync;
}

void FractalGenerator::setDeleteCallback(
		void (*deleteCallback)(FractalGenerator *gen, void *deleteCallbackData),
		void *deleteCallbackData) {
	this->deleteCallback = deleteCallback;
	this->deleteCallbackData = deleteCallbackData;
}

void FractalGenerator::start() {
	thread = new std::thread(&FractalGenerator::threadFunction, this);
}

void FractalGenerator::cancel() {
	state.store(CANCELING);
}

GenerationStatus FractalGenerator::getStatus() {
	return {
		width,
		height,
		progress.load(),
		state.load()
	};
}

/*
 * Private member functions
 */

void FractalGenerator::doneAsyncCallback(uv_async_t *handle) {
	FractalGenerator *self = ((FractalGenerator *) handle->data);
	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	GenerationState state = self->state.load();
	(*self->doneCallback)(isolate, self->path, state, self->doneCallbackData);
}

void FractalGenerator::threadFunction() {
	// build fractal here and call uv_async_send when done

	state.store(GENERATING);
	progress.store(0);

	// build buffer
	unsigned char *buffer = new unsigned char[width * height * 4];

	// does this make things faster or is this redundant with g++ optimization?
	float fx, fy, a, b, aa, bb, twoab;
	unsigned int x, y, i, n;
	RGBData color;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (state.load() == CANCELING) {
				state.store(CANCELED);
				uv_async_send(doneAsync);
				return;
			}

			// get buffer index
			i = (x + y * width) * 4;

			// pixel value generation
			fx = x * fractalWidth / width + fractalX;
			fy = y * fractalHeight / height + fractalY;

			// z = (a + bi)
			a = fx;
			b = fy;

			for (n = 0; n < iterations; n++) {
				// z = z * z
				aa = a * a;
				bb = b * b;
				twoab = 2.0f * a * b;

				a = aa - bb + fx;
				b = twoab + fy;

				// if old sqrt(a * a + b * b) > 4
				// z is outside a circle with radius 4 in the complex plane
				if (aa + bb > 16) {
					break;
				}
			}

			// pixel coloring
			if (n < iterations) {
				// magic numbers
				color = fromHSB(mod2(n * 3.3f, 0, 256.0f) / 256.0f, 1.0f,
						mod2(n * 16.0f, 0, 256.0f) / 256.0f);
				buffer[i] = color.r;
				buffer[i + 1] = color.g;
				buffer[i + 2] = color.b;
				buffer[i + 3] = 0xFF;
			} else {
				buffer[i] = 0x00;
				buffer[i + 1] = 0x00;
				buffer[i + 2] = 0x00;
				buffer[i + 3] = 0xFF;
			}

			progress++;
		}
	}

	// set status
	state.store(WRITING);

	// write png to path

	// get a file handle
	FILE *file = std::fopen(path.c_str(), "wb");
	if (!file) {
		std::cerr << "Failed to open " << path << '\n';
		fail();
		return;
	}

	// create png struct
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
	NULL, NULL);
	if (!png) {
		std::cerr << "Failed to create the png struct\n";
		std::fclose(file);
		fail();
		return;
	}

	// create png info struct
	png_infop info = png_create_info_struct(png);
	if (!info) {
		std::cerr << "Failed to create the png info struct\n";
		std::fclose(file);
		png_destroy_write_struct(&png, NULL);
		fail();
		return;
	}

	// error handling
	if (setjmp(png_jmpbuf(png))) {
		std::cerr << "Png error\n";
		std::fclose(file);
		png_destroy_write_struct(&png, &info);
		fail();
		return;
	}

	// hand file to png
	png_init_io(png, file);

	// set image settings
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
	PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_DEFAULT,
	PNG_FILTER_TYPE_DEFAULT);

	// write image settings
	png_write_info(png, info);

	// allocate array of arrays (image pointers)
	png_bytepp image = new png_bytep[height];

	// assign pointers to parts of the array
	for (int y = 0; y < height; y++) {
		image[y] = buffer + (y * width * 4);
	}

	// write the image data
	png_write_image(png, image);

	// finish the image
	png_write_end(png, info);

	// free the png struct memory
	png_destroy_write_struct(&png, &info);

	// release the file handle
	std::fclose(file);

	// free the image memory
	delete image;
	delete buffer;

	// everything is done
	state.store(DONE);

	// send callback
	uv_async_send(doneAsync);
}

void FractalGenerator::fail() {
	state.store(FAILURE);
	uv_async_send(doneAsync);
}

/*
 * Util functions
 */

std::string generationStateName(GenerationState state) {
	switch (state) {
	case GENERATING:
		return "generating";
		break;
	case WRITING:
		return "writing";
		break;
	case DONE:
		return "done";
		break;
	case CANCELING:
		return "canceling";
		break;
	case CANCELED:
		return "canceled";
		break;
	case FAILURE:
		return "failure";
		break;
	default:
		return "initializing";
	}
}
