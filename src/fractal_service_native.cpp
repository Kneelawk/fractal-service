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
bool halted, void *customData) {
	FractalData *fd = (FractalData *) customData;

	v8::Local<v8::Function> func = fd->jsCallback.Get(isolate);
	v8::Local<v8::Value> args[] = { Nan::New(halted), buffer };
	func->Call(isolate->GetCurrentContext(), func, 2, args);

	generators.erase(fd->id);

	fd->jsCallback.Reset();
	delete fd;
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

	// TODO check arg types
	// get args
	std::string uuid(*v8::String::Utf8Value(info[0]));
	v8::Local<v8::Function> doneCallback = info[1].As<v8::Function>();
	v8::Local<v8::Object> buffer = info[2];
	int width = info[3]->Int32Value();
	int height = info[4]->Int32Value();
	double fractalWidth = info[5]->NumberValue();
	double fractalHeight = info[6]->NumberValue();
	double fractalX = info[7]->NumberValue();
	double fractalY = info[8]->NumberValue();
	int iterations = info[9]->Int32Value();

	FractalData *fd = new FractalData;
	fd->id = uuid;
	fd->jsCallback = v8::Global<v8::Object>;
	fd->jsCallback.Reset(info.GetIsolate(), buffer);

	FractalGenerator *gen = new FractalGenerator(uuid, info.GetIsolate(),
			fractalDoneCallback, buffer, fd, width, height, fractalWidth,
			fractalHeight, fractalX, fractalY, iterations);
	generators.insert(std::pair<std::string, FractalGenerator *>(uuid, gen));
}

void startGenerator(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	/*
	 * Args:
	 * String	uuid	: uuid of the fractal
	 */

	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		generators[uuid]->start();
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void getProgress(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		info.GetReturnValue().Set(generators[uuid]->getProgress());
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void containsFractal(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	std::string uuid(*v8::String::Utf8Value(info[0]));
	info.GetReturnValue().Set(generators.find(uuid) != generators.end());
}

void listFractals(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// TODO list fractals
}

void haltFractal(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		generators[uuid]->halt();
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void haltAllFractals(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// no arguments
	for (const auto &elem : generators) {
		elem.second->halt();
	}
}

void init(v8::Local<v8::Object> exports) {
	exports->Set(Nan::New("createFractalGenerator").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(createFractalGenerator)->GetFunction());
	exports->Set(Nan::New("startGenerator").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(startGenerator)->GetFunction());
	exports->Set(Nan::New("getProgress").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(getProgress)->GetFunction());
	exports->Set(Nan::New("containsFractal").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(containsFractal)->GetFunction());

}

NODE_MODULE(fractal_service_native, init)
