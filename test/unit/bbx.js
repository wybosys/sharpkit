'use strict';

const assert = require('assert');
const sharp = require('../../');
const fixtures = require('../fixtures');

describe('Image Bbx', function () {
  it('PNG', function (done) {
    sharp(fixtures.inputPng).bbx(0, function (err, bbx) {
      if (err) throw err;
      assert.strictEqual(2809, bbx.width);
      assert.strictEqual(2023, bbx.height);
      done();
    });
  });
});
