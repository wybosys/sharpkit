// Copyright 2013, 2014, 2015, 2016, 2017 Lovell Fuller and contributors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <numeric>
#include <vector>

#include <node.h>
#include <nan.h>
#include <vips/vips8>

#include "common.h"
#include "bbx.h"

class BbxWorker : public Nan::AsyncWorker {
 public:
  BbxWorker(
    Nan::Callback *callback, BbxBaton *baton, Nan::Callback *debuglog,
    std::vector<v8::Local<v8::Object>> const buffersToPersist) :
    Nan::AsyncWorker(callback, "sharp:BbxWorker"),
    baton(baton), debuglog(debuglog),
    buffersToPersist(buffersToPersist) {
    // Protect Buffer objects from GC, keyed on index
    std::accumulate(buffersToPersist.begin(), buffersToPersist.end(), 0,
      [this](uint32_t index, v8::Local<v8::Object> const buffer) -> uint32_t {
        SaveToPersistent(index, buffer);
        return index + 1;
      });
  }
  ~BbxWorker() {}

  void Execute() {
    // Decrement queued task counter
    g_atomic_int_dec_and_test(&sharp::counterQueue);

    vips::VImage image;
    sharp::ImageType imageType = sharp::ImageType::UNKNOWN;
    try {
      std::tie(image, imageType) = OpenInput(baton->input, VIPS_ACCESS_SEQUENTIAL);
    } catch (vips::VError const &err) {
      (baton->err).append(err.what());
    }
    if (imageType != sharp::ImageType::UNKNOWN) {
    }

    // Clean up
    vips_error_clear();
    vips_thread_shutdown();
  }

  void HandleOKCallback() {
    using Nan::New;
    using Nan::Set;
    Nan::HandleScope();

    v8::Local<v8::Value> argv[2] = { Nan::Null(), Nan::Null() };
    if (!baton->err.empty()) {
      argv[0] = Nan::Error(baton->err.data());
    } else {
      // Bbx Object
      v8::Local<v8::Object> info = New<v8::Object>();
      Set(info, New("top").ToLocalChecked(), New<v8::Uint32>(baton->top));
      Set(info, New("left").ToLocalChecked(), New<v8::Uint32>(baton->left));
      Set(info, New("width").ToLocalChecked(), New<v8::Uint32>(baton->width));
      Set(info, New("height").ToLocalChecked(), New<v8::Uint32>(baton->height));
      argv[1] = info;
    }

    // Dispose of Persistent wrapper around input Buffers so they can be garbage collected
    std::accumulate(buffersToPersist.begin(), buffersToPersist.end(), 0,
      [this](uint32_t index, v8::Local<v8::Object> const buffer) -> uint32_t {
        GetFromPersistent(index);
        return index + 1;
      });
    delete baton->input;
    delete baton;

    // Handle warnings
    std::string warning = sharp::VipsWarningPop();
    while (!warning.empty()) {
      v8::Local<v8::Value> message[1] = { New(warning).ToLocalChecked() };
      debuglog->Call(1, message, async_resource);
      warning = sharp::VipsWarningPop();
    }

    // Return to JavaScript
    callback->Call(2, argv, async_resource);
  }

 private:
  BbxBaton* baton;
  Nan::Callback *debuglog;
  std::vector<v8::Local<v8::Object>> buffersToPersist;
};

/*
  bbx(options, callback)
*/
NAN_METHOD(bbx) {
  // Input Buffers must not undergo GC compaction during processing
  std::vector<v8::Local<v8::Object>> buffersToPersist;

  // V8 objects are converted to non-V8 types held in the baton struct
  BbxBaton *baton = new BbxBaton;
  v8::Local<v8::Object> options = info[0].As<v8::Object>();

  // Input
  baton->input = sharp::CreateInputDescriptor(sharp::AttrAs<v8::Object>(options, "input"), buffersToPersist);

  // Function to notify of libvips warnings
  Nan::Callback *debuglog = new Nan::Callback(sharp::AttrAs<v8::Function>(options, "debuglog"));

  // Join queue for worker thread
  Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
  Nan::AsyncQueueWorker(new BbxdataWorker(callback, baton, debuglog, buffersToPersist));

  // Increment queued task counter
  g_atomic_int_inc(&sharp::counterQueue);
}
