#include <nan.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include "FractalGenerator.h"

#define JS_LENGTH_CHECK(info, length) if (info.Length() < length) {	\
	Nan::ThrowTypeError("Wrong number of arguments, must be at least " #length);	\
	return;	\
}

#define JS_TYPE_CHECK(info, index, typeCheck) if (!info[index]->typeCheck()) {	\
	Nan::ThrowTypeError("Wrong argument type, failed test " #typeCheck);	\
	return;	\
}

std::unordered_map<std::string, FractalGenerator *> generators;

typedef struct {
	v8::Global<v8::Function> jsCallback;
	std::string id;
} FractalData;

void fractalDoneCallback(v8::Isolate *isolate, std::string path,
		GenerationState state, void *customData) {
	FractalData *fd = (FractalData *) customData;

	v8::Local<v8::Function> func = fd->jsCallback.Get(isolate);
	v8::Local<v8::Value> args[] = {
			Nan::New(generationStateName(state)).ToLocalChecked(), Nan::New(
					path).ToLocalChecked() };
	func->Call(isolate->GetCurrentContext(), func, 2, args);
}

void deleteCallback(FractalGenerator *gen, void *deleteCallbackData) {
	FractalData *fd = (FractalData *) deleteCallbackData;

	generators.erase(fd->id);

	fd->jsCallback.Reset();
	delete fd;
}

void createFractalGenerator(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	/*
	 * Args:
	 * String	uuid			: frctal id
	 * Function	doneCallback	: callback for when the fractal is done
	 * String	path			: the buffer the fractal should write to
	 * Int		width			: the image width of the fractal
	 * Int		height			: the image height of the fractal
	 * Double	fractalWidth	: the complex width (real) of the fractal
	 * Double	fractalHeight	: the complex height (imaginary) of the fractal
	 * Double	fractalX		: the complex x (real) start of the fractal (x+ is right)
	 * Double	fractalY		: the complex y (imaginary) start of the fractal (y+ is down)
	 * Int		iterations		: the number of iterations before a pixel turns black
	 */

	// type checks
	JS_LENGTH_CHECK(info, 10)
	JS_TYPE_CHECK(info, 0, IsString)
	JS_TYPE_CHECK(info, 1, IsFunction)
	JS_TYPE_CHECK(info, 2, IsString)
	JS_TYPE_CHECK(info, 3, IsNumber)
	JS_TYPE_CHECK(info, 4, IsNumber)
	JS_TYPE_CHECK(info, 5, IsNumber)
	JS_TYPE_CHECK(info, 6, IsNumber)
	JS_TYPE_CHECK(info, 7, IsNumber)
	JS_TYPE_CHECK(info, 8, IsNumber)
	JS_TYPE_CHECK(info, 9, IsNumber)

	// get args
	std::string uuid(*v8::String::Utf8Value(info[0]));
	v8::Local<v8::Function> doneCallback = info[1].As<v8::Function>();
	std::string path(*v8::String::Utf8Value(info[2]));
	int width = info[3]->Int32Value();
	int height = info[4]->Int32Value();
	double fractalWidth = info[5]->NumberValue();
	double fractalHeight = info[6]->NumberValue();
	double fractalX = info[7]->NumberValue();
	double fractalY = info[8]->NumberValue();
	int iterations = info[9]->Int32Value();

	FractalData *fd = new FractalData;
	fd->id = uuid;
	fd->jsCallback = v8::Global<v8::Function>();
	fd->jsCallback.Reset(info.GetIsolate(), doneCallback);

	FractalGenerator *gen = new FractalGenerator(uuid, info.GetIsolate(),
			fractalDoneCallback, path, fd, width, height, fractalWidth,
			fractalHeight, fractalX, fractalY, iterations);
	gen->setDeleteCallback(deleteCallback, fd);
	generators.insert(std::pair<std::string, FractalGenerator *>(uuid, gen));
}

void startGenerator(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	/*
	 * Args:
	 * String	uuid	: uuid of the fractal
	 */

	JS_LENGTH_CHECK(info, 1)
	JS_TYPE_CHECK(info, 0, IsString)

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		generators[uuid]->start();
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void getStatus(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	JS_LENGTH_CHECK(info, 1)
	JS_TYPE_CHECK(info, 0, IsString)

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		GenerationStatus status = generators[uuid]->getStatus();
		v8::Local<v8::Object> out = Nan::New<v8::Object>();
		out->Set(Nan::New("width").ToLocalChecked(), Nan::New(status.width));
		out->Set(Nan::New("height").ToLocalChecked(), Nan::New(status.height));
		out->Set(Nan::New("maxProgress").ToLocalChecked(),
				Nan::New(status.width * status.height));
		out->Set(Nan::New("progress").ToLocalChecked(),
				Nan::New(status.progress));
		out->Set(Nan::New("state").ToLocalChecked(),
				Nan::New(generationStateName(status.state)).ToLocalChecked());
		info.GetReturnValue().Set(out);
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void containsFractal(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	JS_LENGTH_CHECK(info, 1)
	JS_TYPE_CHECK(info, 0, IsString)

	std::string uuid(*v8::String::Utf8Value(info[0]));
	info.GetReturnValue().Set(generators.find(uuid) != generators.end());
}

void listFractals(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// no arguments
	v8::Local<v8::Array> fractals = Nan::New<v8::Array>(generators.size());
	auto it = generators.begin();
	for (int i = 0; i < generators.size(); i++) {
		fractals->Set(i, Nan::New(it->first).ToLocalChecked());
		it++;
	}
	info.GetReturnValue().Set(fractals);
}

void cancelFractal(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	JS_LENGTH_CHECK(info, 1)
	JS_TYPE_CHECK(info, 0, IsString)

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		generators[uuid]->cancel();
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void cancelAllFractals(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// no arguments
	for (const auto &elem : generators) {
		elem.second->cancel();
	}
}

void destroyFractal(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// uuid is argument
	JS_LENGTH_CHECK(info, 1)
	JS_TYPE_CHECK(info, 0, IsString)

	std::string uuid(*v8::String::Utf8Value(info[0]));
	if (generators.find(uuid) != generators.end()) {
		delete generators[uuid];
	} else {
		Nan::ThrowRangeError(
				(std::string("No fractal with uuid ") + uuid).c_str());
	}
}

void destroyAllFractals(const Nan::FunctionCallbackInfo<v8::Value> &info) {
	// no arguments
	for (const auto &elem : generators) {
		delete elem.second;
	}
}

void init(v8::Local<v8::Object> exports) {
	exports->Set(Nan::New("createFractalGenerator").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(createFractalGenerator)->GetFunction());
	exports->Set(Nan::New("startGenerator").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(startGenerator)->GetFunction());
	exports->Set(Nan::New("getStatus").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(getStatus)->GetFunction());
	exports->Set(Nan::New("containsFractal").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(containsFractal)->GetFunction());
	exports->Set(Nan::New("listFractals").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(listFractals)->GetFunction());
	exports->Set(Nan::New("cancelFractal").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(cancelFractal)->GetFunction());
	exports->Set(Nan::New("cancelAllFractals").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(cancelAllFractals)->GetFunction());
	exports->Set(Nan::New("destroyFractal").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(destroyFractal)->GetFunction());
	exports->Set(Nan::New("destroyAllFractals").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(destroyAllFractals)->GetFunction());

}

NODE_MODULE(fractal_service_native, init)
