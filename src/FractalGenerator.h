#ifndef FRACTALGENERATOR_H_
#define FRACTALGENERATOR_H_

#include <nan.h>
#include <thread>
#include <atomic>
#include <string>

typedef std::string fractalId;

class FractalGenerator {
public:
	// info
	fractalId id;

	// settings
	int width;
	int height;
	double fractalWidth;
	double fractalHeight;
	double fractalX;
	double fractalY;
	int iterations;

	FractalGenerator(fractalId id, v8::Isolate *isolate,
			void (*doneCallback)(v8::Isolate *isolate,
					v8::Local<v8::Object> nodeBuffer, bool halted,
					void *doneCallbackData), v8::Local<v8::Value> buf,
			void *doneCallbackData, int width, int height, double fractalWidth,
			double fractalHeight, double fracgtalX, double fractalY,
			int iterations);
	virtual ~FractalGenerator();

	void setDeleteCallback(
			void (*deleteCallback)(FractalGenerator *gen,
					void *deleteCallbackData), void *deleteCallbackData);

	void start();

	void halt();

	int getProgress();

	bool isGenerating();

private:
	// thread
	std::thread *thread = NULL;

	// fractal generation stats
	std::atomic_bool generating;
	std::atomic_uint progress;
	std::atomic_bool halting;

	// buffers
	v8::Global<v8::Object> *nodeBuffer;
	char *buffer = NULL;

	// callback control object
	uv_async_t *doneAsync;

	// callback
	void (*doneCallback)(v8::Isolate *isolate, v8::Local<v8::Object> nodeBuffer,
			bool halted, void *doneCallbackData);
	void *doneCallbackData;

	// delete callback
	void (*deleteCallback)(FractalGenerator *gen,
			void *deleteCallbackData) = NULL;
	void *deleteCallbackData = NULL;

	static void doneAsyncCallback(uv_async_t *handle);

	void threadFunction();
};

#endif
