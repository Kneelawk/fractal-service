#include <stdexcept>
#include "FractalGenerator.h"
#include "math_utils.h"

/*
 * Public member functions
 */

FractalGenerator::FractalGenerator(fractalId id, v8::Isolate *isolate,
		void (*doneCallback)(v8::Isolate *isolate,
				v8::Local<v8::Object> nodeBuffer, bool halted,
				void *doneCallbackData), v8::Local<v8::Object> buf,
		void *doneCallbackData, int width, int height, double fractalWidth,
		double fractalHeight, double fracgtalX, double fractalY, int iterations) :
		id(id), doneCallback(doneCallback), doneCallbackData(doneCallbackData), width(
				width), height(height), fractalWidth(fractalWidth), fractalHeight(
				fractalHeight), fractalX(fractalX), fractalY(fractalY), iterations(
				iterations) {
	if (buf->IsNull() || buf->IsUndefined()
			|| node::Buffer::Length(buf) < (width * height * 4)) {
		buf = Nan::NewBuffer(width * height * 4).ToLocalChecked();
	}

	nodeBuffer = new v8::Global<v8::Object>;
	nodeBuffer->Reset(isolate, buf);
	buffer = node::Buffer::Data(buf);

	doneAsync = new uv_async_t;
	doneAsync->data = this;
	uv_loop_t *loop = uv_default_loop();
	uv_async_init(loop, doneAsync, doneAsyncCallback);
}

FractalGenerator::~FractalGenerator() {
	if (deleteCallback) {
		deleteCallback(this, deleteCallbackData);
	}

	nodeBuffer->Reset();
	delete nodeBuffer;
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

void FractalGenerator::halt() {
	halting.store(true);
}

int FractalGenerator::getProgress() {
	return progress.load();
}

bool FractalGenerator::isGenerating() {
	return generating.load();
}

/*
 * Private member functions
 */

void FractalGenerator::doneAsyncCallback(uv_async_t *handle) {
	FractalGenerator *self = ((FractalGenerator *) handle->data);
	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	(*self->doneCallback)(isolate, self->nodeBuffer->Get(isolate),
			self->halting.load(), self->doneCallbackData);
	self->nodeBuffer->Reset();
}

void FractalGenerator::threadFunction() {
	// build fractal here and call uv_async_send when done

	generating.store(true);
	progress.store(0);

	// does this make things faster or is this redundant with g++ optimization?
	float fx, fy, a, b, aa, bb, twoab;
	int x, y, i, n;
	RGBData color;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (halting.load()) {
				generating.store(false);
				uv_async_send(doneAsync);
				return;
			}

			// get buffer index
			i = x + y * width;

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
	generating.store(false);

	// send callback
	uv_async_send(doneAsync);
}
