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

#ifndef SRC_BBX_H_
#define SRC_BBX_H_

#include <string>
#include <nan.h>

#include "./common.h"

struct BbxBaton {
  // Input
  sharp::InputDescriptor *input;
  int tolerance;
  // Output
  int left;
  int top;
  int width;
  int height;
  int density;
  std::string err;

  BbxBaton():
    input(nullptr),
    tolerance(10),
    left(0),
    top(0),
    width(0),
    height(0)
    {}
};

NAN_METHOD(bbx);

#endif  // SRC_BBX_H_
