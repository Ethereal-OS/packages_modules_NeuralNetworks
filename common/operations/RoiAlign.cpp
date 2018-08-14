/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "CpuOperationUtils.h"
#include "Operations.h"

#include <cfloat>
#include <cmath>

#include "Tracing.h"

namespace android {
namespace nn {

bool roiAlign(const float* inputData, const Shape& inputShape, const float* roiData,
              const Shape& roiShape, float spatialScale, int32_t samplingRatio, float* outputData,
              const Shape& outputShape) {
    NNTRACE_TRANS("RoiAlign");

    const uint32_t kRoiDim = 4;

    uint32_t numBatches = getSizeOfDimension(inputShape, 0);
    uint32_t inHeight = getSizeOfDimension(inputShape, 1);
    uint32_t inWidth = getSizeOfDimension(inputShape, 2);
    uint32_t inDepth = getSizeOfDimension(inputShape, 3);
    uint32_t outHeight = getSizeOfDimension(outputShape, 1);
    uint32_t outWidth = getSizeOfDimension(outputShape, 2);
    uint32_t numRois = getSizeOfDimension(roiShape, 0);
    uint32_t roiInfoLength = getSizeOfDimension(roiShape, 1);

    float* outPtr = outputData;
    const float* roiDataEnd = roiData + numRois * roiInfoLength;
    for (const float* roiInfo = roiData; roiInfo < roiDataEnd; roiInfo += kRoiDim) {
        uint32_t batchId = 0;
        // get optional batch id
        if (roiInfoLength == kRoiDim + 1) {
            batchId = std::round(roiInfo[0]);
            roiInfo++;
        }
        const float* batchBase = inputData + batchId * inHeight * inWidth * inDepth;

        float wRoiStart = roiInfo[0] * spatialScale;
        float hRoiStart = roiInfo[1] * spatialScale;
        float wRoiEnd = roiInfo[2] * spatialScale;
        float hRoiEnd = roiInfo[3] * spatialScale;

        float roiWidth = std::max(wRoiEnd - wRoiStart, 1.0f);
        float roiHeight = std::max(hRoiEnd - hRoiStart, 1.0f);
        float wStepSize = roiWidth / static_cast<float>(outWidth);
        float hStepSize = roiHeight / static_cast<float>(outHeight);

        // if samplingRatio <= 0, use adaptive value of ceil(roiWidth/outWidth), same for height
        int32_t wSamplingRatio = samplingRatio > 0 ? samplingRatio : std::ceil(wStepSize);
        int32_t hSamplingRatio = samplingRatio > 0 ? samplingRatio : std::ceil(hStepSize);
        int32_t numSamplingPoints = wSamplingRatio * hSamplingRatio;
        float wBinSize = wStepSize / static_cast<float>(wSamplingRatio);
        float hBinSize = hStepSize / static_cast<float>(hSamplingRatio);

        for (uint32_t i = 0; i < outHeight; i++) {
            for (uint32_t j = 0; j < outWidth; j++) {
                float wStart = wStepSize * j + wRoiStart;
                float wEnd = wStepSize * (j + 1) + wRoiStart;
                float hStart = hStepSize * i + hRoiStart;
                float hEnd = hStepSize * (i + 1) + hRoiStart;

                // initialize output to zero
                for (uint32_t k = 0; k < inDepth; k++) outPtr[k] = 0;

                // calculate the sum of the sampling points
                for (float y = hStart + hBinSize / 2; y < hEnd; y += hBinSize) {
                    for (float x = wStart + wBinSize / 2; x < wEnd; x += wBinSize) {
                        // bilinear interpolation of point (x,y)
                        // w.r.t box [(x1,y1), (x1,y2), (x2,y1), (x2,y2)]
                        uint32_t x1 = std::floor(x), y1 = std::floor(y);
                        uint32_t x2 = x1 + 1, y2 = y1 + 1;
                        float dx1 = x - static_cast<float>(x1);
                        float dy1 = y - static_cast<float>(y1);

                        // dealing with out of bound samples
                        if (x1 >= inWidth - 1) {
                            x1 = x2 = inWidth - 1;
                            dx1 = 0;
                        }
                        if (y1 >= inHeight - 1) {
                            y1 = y2 = inHeight - 1;
                            dy1 = 0;
                        }

                        float dx2 = 1.0f - dx1, dy2 = 1.0f - dy1;
                        float w1 = dx2 * dy2, w2 = dx1 * dy2, w3 = dx2 * dy1, w4 = dx1 * dy1;

                        uint32_t offset1 = y1 * inWidth * inDepth + x1 * inDepth;
                        uint32_t offset2 = y1 * inWidth * inDepth + x2 * inDepth;
                        uint32_t offset3 = y2 * inWidth * inDepth + x1 * inDepth;
                        uint32_t offset4 = y2 * inWidth * inDepth + x2 * inDepth;

                        for (uint32_t k = 0; k < inDepth; k++) {
                            float interpolation =
                                    w1 * batchBase[offset1 + k] + w2 * batchBase[offset2 + k] +
                                    w3 * batchBase[offset3 + k] + w4 * batchBase[offset4 + k];
                            outPtr[k] += interpolation;
                        }
                    }
                }

                // take average
                for (uint32_t k = 0; k < inDepth; k++)
                    outPtr[k] /= static_cast<float>(numSamplingPoints);

                outPtr += inDepth;
            }
        }
    }

    return true;
}

}  // namespace nn
}  // namespace android
