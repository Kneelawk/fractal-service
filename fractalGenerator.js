const Png = require('pngjs').PNG;

class Fractal {
  constructor(imageWidth, imageHeight, fractalWidth, fractalHeight, fractalX, fractalY, iterations, f) {
    this.imageWidth = imageWidth;
    this.imageHeight = imageHeight;
    this.fractalWidth = fractalWidth;
    this.fractalHeight = fractalHeight;
    this.fractalX = fractalX;
    this.fractalY = fractalY;
    this.iterations = iterations;
    this.f = f;
  }

  start() {
    this.progress = 0;
  }
}
