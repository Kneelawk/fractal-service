const express = require('express');
const Fractal = require('./fractalGenerator.js');
const uuid = require('uuid');
const math = require('mathjs');
const bodyParser = require('body-parser');

const app = express();

app.use(bodyParser.json());

let fractals = {};

app.post('/api/generate', (req, res) => {
  let id = uuid.v4();
  fractals[id] = new Fractal(
    id,
    req.body.imageWidth || 300,
    req.body.imageHeight || 200,
    req.body.fractalWidth || 3,
    req.body.fractalHeight || 2,
    req.body.fractalX || 0,
    req.body.fractalY || 0,
    req.body.constantReal || 0,
    req.body.constantImmaginary || 0,
    req.body.iterations || 100,
    (z, f, c) => {
      return math.sum(math.square(z), f);
    });
  fractals[id].start();

  res.type('json');
  return res.send(JSON.stringify({
    uuid: id
  }));
});

app.get('/api/progress', (req, res) => {
  if (!req.query.uuid) {
    return res.status(400).send({
      error: 'invalid uuid'
    });
  }
  if (!fractals[req.query.uuid]) {
    return res.status(404).send({
      error: 'no fractal for uuid'
    });
  }

  let fractal = fractals[req.query.uuid];
  let status = fractal.status();
  return res.send({
    pixelsGenerated: status.progress,
    totalPixels: status.maxProgress,
    width: status.width,
    height: status.height,
    state: status.state,
    done: status.done
  });
});

app.get('/api/result', (req, res) => {
  if (!req.query.uuid) {
    return res.status(400).send({
      error: 'invalid uuid'
    });
  }
  if (!fractals[req.query.uuid]) {
    return res.status(404).send({
      error: 'no fractal for uuid'
    });
  }
  let fractal = fractals[req.query.uuid];
  let status = fractal.status();
  if (status.done) {
    res.type('image/png');
    return fractal.stream().pipe(res);
  } else {
    return res.send({
      pixelsGenerated: status.progress,
      totalPixels: status.maxProgress,
      width: status.width,
      height: status.height,
      state: status.state,
      done: false
    });
  }
});

app.listen(8080);
