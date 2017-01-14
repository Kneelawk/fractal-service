const Png = require('pngjs').PNG;
const math = require('mathjs');
const fromHSB = require('./colorutil.js').fromHSB;
const mod2 = require('./mathutils.js').mod2;

class Fractal {
  constructor(imageWidth, imageHeight, fractalWidth, fractalHeight, fractalX, fractalY, iterations, f) {
    this.imageWidth = imageWidth;
    this.imageHeight = imageHeight;
    this.fractalWidth = fractalWidth;
    this.fractalHeight = fractalHeight;
    this.fractalMinX = fractalX - fractalWidth / 2;
    this.fractalMinY = fractalY - fractalHeight / 2;
    this.iterations = iterations;
    this.f = f;
    this.png = new Png({
      width: imageWidth,
      height: imageHeight,
      hasInputAlpha: true
    });
    this.progress = 0;
  }

  start() {
    this.progress = 0;

    for (let y = 0; y < this.imageHeight; y++) {
      for (let x = 0; x < this.imageWidth; x++) {
        setImmediate(() => {
          let i = (x + y * this.imageWidth) * 4;

          let color = this.genPixel(x, y);

          this.png.data[i] = color.r;
          this.png.data[i + 1] = color.g;
          this.png.data[i + 2] = color.b;
          this.png.data[i + 3] = color.a;

          this.progress = (x + y * this.imageWidth) * 100 / (this.imageWidth * this.imageHeight);
        });
      }
    }
  }

  genPixel(x, y) {
    let fx = x * this.fractalWidth / this.imageWidth + this.fractalMinX;
    let fx = y * this.fractalHeight / this.imageHeight + this.fractalMinY;

    let z = math.complex(fx, fy);

    let n;
    for (n = 0; n < this.iterations; n++) {
      z = this.f(z, math.complex(fx, fy));

      if (z.re * z.re + z.im * z.im > 16) {
        break;
      }
    }

    if (n < this.iterations) {
      let color = fromHSB(mod2(n * 3.3, 0.0, 256.0) / 256.0, 1.0, mod2(n * 16.0, 0.0, 256.0) / 256.0);
      color.a = 255;
      return color;
    } else {
      return {
        r: 0.0,
        g: 0.0,
        b: 0.0,
        a: 255.0
      };
    }
  }
}
