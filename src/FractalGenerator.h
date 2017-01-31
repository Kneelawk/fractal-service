#ifndef FRACTALGENERATOR_H_
#define FRACTALGENERATOR_H_

#include <nan.h>
#include <thread>
#include <atomic>
#include <string>

enum GenerationState {
	INITIALIZING, GENERATING, WRITING, DONE, CANCELING, CANCELED, FAILURE
};

std::string generationStateName(GenerationState state);

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int progress;
	GenerationState state;
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
	std::string path;

	FractalGenerator(fractalId id, v8::Isolate *isolate,
			void (*doneCallback)(v8::Isolate *isolate, std::string path,
					GenerationState state, void *doneCallbackData),
			std::string path, void *doneCallbackData, unsigned int width,
			unsigned int height, double fractalWidth, double fractalHeight,
			double fractalX, double fractalY, unsigned int iterations);
	virtual ~FractalGenerator();

	void setDeleteCallback(
			void (*deleteCallback)(FractalGenerator *gen,
					void *deleteCallbackData), void *deleteCallbackData);

	void start();

	void cancel();

	GenerationStatus getStatus();

private:
	// thread
	std::thread *thread = NULL;

	// fractal generation stats
	std::atomic_uint progress;
	std::atomic<GenerationState> state;

	// callback control object
	uv_async_t *doneAsync;

	// callback
	void (*doneCallback)(v8::Isolate *isolate, std::string path,
			GenerationState state, void *doneCallbackData);
	void *doneCallbackData;

	// delete callback
	void (*deleteCallback)(FractalGenerator *gen,
			void *deleteCallbackData) = NULL;
	void *deleteCallbackData = NULL;

	static void doneAsyncCallback(uv_async_t *handle);

	void threadFunction();

	void fail();
};

#endif
