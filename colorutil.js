function fromHSB(hue, saturation, brightness) {
  // copied and pasted from java.awt.Color.getHSBColor(float hue, float saturation, float brightness)

  let r = 0,
    g = 0,
    b = 0;
  if (saturation == 0) {
    r = g = b = Math.floor(brightness * 255.0 + 0.5);
  } else {
    let h = (hue - Math.floor(hue)) * 6.0;
    let f = h - Math.floor(h);
    let p = brightness * (1.0 - saturation);
    let q = brightness * (1.0 - saturation * f);
    let t = brightness * (1.0 - (saturation * (1.0 - f)));
    switch (Math.floor(h)) {
      case 0:
        r = Math.floor(brightness * 255.0 + 0.5);
        g = Math.floor(t * 255.0 + 0.5);
        b = Math.floor(p * 255.0 + 0.5);
        break;
      case 1:
        r = Math.floor(q * 255.0 + 0.5);
        g = Math.floor(brightness * 255.0 + 0.5);
        b = Math.floor(p * 255.0 + 0.5);
        break;
      case 2:
        r = Math.floor(p * 255.0 + 0.5);
        g = Math.floor(brightness * 255.0 + 0.5);
        b = Math.floor(t * 255.0 + 0.5);
        break;
      case 3:
        r = Math.floor(p * 255.0 + 0.5);
        g = Math.floor(q * 255.0 + 0.5);
        b = Math.floor(brightness * 255.0 + 0.5);
        break;
      case 4:
        r = Math.floor(t * 255.0 + 0.5);
        g = Math.floor(p * 255.0 + 0.5);
        b = Math.floor(brightness * 255.0 + 0.5);
        break;
      case 5:
        r = Math.floor(brightness * 255.0 + 0.5);
        g = Math.floor(p * 255.0 + 0.5);
        b = Math.floor(q * 255.0 + 0.5);
        break;
    }
  }

  return {
    r,
    g,
    b
  };
}

module.exports = {
  fromHSB
};
