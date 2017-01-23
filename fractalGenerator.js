const Png = require('pngjs').PNG;
const math = require('mathjs');
const fs = require('fs');
const path = require('path');
const fromHSB = require('./colorutil.js').fromHSB;
const mod2 = require('./mathutils.js').mod2;

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
    this.pixelsGenerated = 0;
  }

  start() {
    this.pixelsGenerated = 0;

    for (let y = 0; y < this.imageHeight; y++) {
      for (let x = 0; x < this.imageWidth; x++) {
        // let other fractals gen at the same time
        setImmediate(() => {
          let i = (x + y * this.imageWidth) * 4;

          this.genPixel(x, y, (color) => {
            this.png.data[i] = color.r;
            this.png.data[i + 1] = color.g;
            this.png.data[i + 2] = color.b;
            this.png.data[i + 3] = color.a;

            // node is single threaded
            this.pixelsGenerated++;
            if (this.done()) {
              this.save();
            }
          });
        });
      }
    }
  }

  save() {
    this.png.pack().pipe(fs.createWriteStream(this.file));
  }

  stream() {
    return fs.createReadStream(this.file);
  }

  destroy() {
    fs.unlink(this.file);
  }

  progress() {
    return this.pixelsGenerated / (this.imageWidth * this.imageHeight);
  }

  done() {
    return this.pixelsGenerated >= (this.imageWidth * this.imageHeight);
  }

  genPixel(x, y, callback) {
    let fx = x * this.fractalWidth / this.imageWidth + this.fractalMinX;
    let fy = y * this.fractalHeight / this.imageHeight + this.fractalMinY;

    let z = math.complex(fx, fy);
    let n = 0;

    let calc = () => {
      if (n >= this.iterations) {
        callback({
          r: 0,
          g: 0,
          b: 0,
          a: 255
        });
      } else if (z.re * z.re + z.im + z.im > 16) {
        let color = fromHSB(mod2(n * 3.3, 0.0, 256.0) / 256.0, 1.0, mod2(n * 16.0, 0.0, 256.0) / 256.0);
        color.a = 255;
        callback(color);
      } else {
        n++;
        z = this.f(z, math.complex(fx, fy), this.constant);
        setImmediate(calc);
      }
    };

    setImmediate(calc);
  }
}

module.exports = Fractal;
