#ifndef FRACTALGENERATOR_H_
#define FRACTALGENERATOR_H_

#include <nan.h>
#include <thread>
#include <atomic>
#include <string>

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int progress;
	bool generating;
	bool halting;
} GenerationStatus;

typedef std::string fractalId;

class FractalGenerator {
public:
	// info
	fractalId id;

	// settings
	unsigned int width;
	unsigned int height;
	double fractalWidth;
	double fractalHeight;
	double fractalX;
	double fractalY;
	unsigned int iterations;

	FractalGenerator(fractalId id, v8::Isolate *isolate,
			void (*doneCallback)(v8::Isolate *isolate,
					v8::Local<v8::Object> nodeBuffer, bool halted,
					void *doneCallbackData), v8::Local<v8::Value> bufVal,
			void *doneCallbackData, unsigned int width, unsigned int height,
			double fractalWidth, double fractalHeight, double fractalX,
			double fractalY, unsigned int iterations);
	virtual ~FractalGenerator();

	void setDeleteCallback(
			void (*deleteCallback)(FractalGenerator *gen,
					void *deleteCallbackData), void *deleteCallbackData);

	void start();

	void halt();

	GenerationStatus getStatus();

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
