/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_ML_NN_COMMON_OPERATIONS_H
#define ANDROID_ML_NN_COMMON_OPERATIONS_H

#include <stddef.h>

namespace android {
namespace nn {

struct Shape;

bool addTensorsFloat32(const float* in1, const float* in2, float* out, const Shape& shape);
bool addTensorsFloat32Prepare(const Shape& in1, const Shape& in2, Shape* out1);

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_OPERATIONS_H
