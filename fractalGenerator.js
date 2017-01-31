const Png = require('pngjs').PNG;
const math = require('mathjs');
const fs = require('fs');
const path = require('path');
const fromHSB = require('./colorutil.js').fromHSB;
const mod2 = require('./mathutils.js').mod2;
const native_fractal_service = require('bindings')('fractal_service_native');

const fractalsDir = './fractals';

if (!fs.existsSync(fractalsDir)) {
  fs.mkdirSync(fractalsDir);
}

class Fractal {
  constructor(uuid, imageWidth, imageHeight, fractalWidth, fractalHeight, fractalX, fractalY, constantReal, constantImmaginary, iterations, f) {
    this.uuid = uuid;
    this.file = path.join(fractalsDir, uuid);
    this.imageWidth = imageWidth;
    this.imageHeight = imageHeight;
    this.fractalWidth = fractalWidth;
    this.fractalHeight = fractalHeight;
    this.fractalMinX = fractalX - fractalWidth / 2;
    this.fractalMinY = fractalY - fractalHeight / 2;
    this.constant = math.complex(constantReal, constantImmaginary);
    this.iterations = iterations;
    this.f = f;
    this.png = new Png({
      width: imageWidth,
      height: imageHeight,
      hasInputAlpha: true
    });
    native_fractal_service.createFractalGenerator(this.uuid,
      (halted, buffer) => this.save(halted, buffer), this.png.data, this.imageWidth,
      this.imageHeight, this.fractalWidth, this.fractalHeight, this.fractalMinX,
      this.fractalMinY, this.iterations);
  }

  start() {
    native_fractal_service.startGenerator(this.uuid);
  }

  save(halted, buffer) {
    this.png.pack().pipe(fs.createWriteStream(this.file));
  }

  stream() {
    return fs.createReadStream(this.file);
  }

  destroy() {
    native_fractal_service.destroyFractal(this.uuid);
    fs.unlink(this.file);
  }

  status() {
    return native_fractal_service.getStatus(this.uuid);
  }
}

Fractal.done = function (fractal, pixelsGenerated) {
  return pixelsGenerated >= (fractal.imageWidth * fractal.imageHeight);
}

Fractal.progress = function (fractal, pixelsGenerated) {
  return pixelsGenerated / (this.imageWidth * this.imageHeight);
}

module.exports = Fractal;
