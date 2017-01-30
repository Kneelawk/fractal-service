#include <nan.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include "FractalGenerator.h"

std::unordered_map<std::string, FractalGenerator *> generators;

typedef struct {
	v8::Global<v8::Function> jsCallback;
	std::string id;
} FractalData;

void fractalDoneCallback(v8::Isolate *isolate, v8::Local<v8::Object> buffer,
		void *customData) {
	FractalData *fd = (FractalData *) customData;

	v8::Local<v8::Function> func = fd->jsCallback.Get(isolate);
	v8::Local<v8::Value> args[] = { buffer };
	func->Call(isolate->GetCurrentContext(), func, 1, args);

	generators.erase(fd->id);

	fd->jsCallback.Reset();
	delete *fd;
}

void createFractalGenerator(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	/*
	 * Args:
	 * String	uuid			: frctal id
	 * Function	doneCallback	: callback for when the fractal is done
	 * Buffer	buffer			: the buffer the fractal should write to
	 * Int		width			: the image width of the fractal
	 * Int		height			: the image height of the fractal
	 * Double	fractalWidth	: the complex width (real) of the fractal
	 * Double	fractalHeight	: the complex height (imaginary) of the fractal
	 * Double	fractalX		: the complex x (real) start of the fractal (x+ is right)
	 * Double	fractalY		: the complex y (imaginary) start of the fractal (y+ is down)
	 * Int		iterations		: the number of iterations before a pixel turns black
	 */
}

void init(v8::Local<v8::Object> exports) {

}

NODE_MODULE(fractal_service_native, init)
