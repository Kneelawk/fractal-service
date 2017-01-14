function mod2(value, min, max) {
  let size = max - min;
  if (size == 0)
    return max;
  if (size < 0)
    return max;
  if (value < min || value >= max) {
    let quot = (value - min) / size;
    quot -= Math.floor(quot);
    value = (size * quot) + min;
  }
  return value;
}

module.exports = {
  mod2
};
