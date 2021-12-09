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

// Provides C++ classes to more easily use the Neural Networks API.

#ifndef ANDROID_PACKAGES_MODULES_NEURALNETWORKS_RUNTIME_NEURAL_NETWORKS_WRAPPER_H
#define ANDROID_PACKAGES_MODULES_NEURALNETWORKS_RUNTIME_NEURAL_NETWORKS_WRAPPER_H

#include <assert.h>
#include <math.h>

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#ifdef NNTEST_SLTS
#include "SupportLibrary.h"
#else
#include "NeuralNetworks.h"
#endif

namespace android {
namespace nn {
namespace wrapper {

enum class Type {
    FLOAT32 = ANEURALNETWORKS_FLOAT32,
    INT32 = ANEURALNETWORKS_INT32,
    UINT32 = ANEURALNETWORKS_UINT32,
    TENSOR_FLOAT32 = ANEURALNETWORKS_TENSOR_FLOAT32,
    TENSOR_INT32 = ANEURALNETWORKS_TENSOR_INT32,
    TENSOR_QUANT8_ASYMM = ANEURALNETWORKS_TENSOR_QUANT8_ASYMM,
    BOOL = ANEURALNETWORKS_BOOL,
    TENSOR_QUANT16_SYMM = ANEURALNETWORKS_TENSOR_QUANT16_SYMM,
    TENSOR_FLOAT16 = ANEURALNETWORKS_TENSOR_FLOAT16,
    TENSOR_BOOL8 = ANEURALNETWORKS_TENSOR_BOOL8,
    FLOAT16 = ANEURALNETWORKS_FLOAT16,
    TENSOR_QUANT8_SYMM_PER_CHANNEL = ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL,
    TENSOR_QUANT16_ASYMM = ANEURALNETWORKS_TENSOR_QUANT16_ASYMM,
    TENSOR_QUANT8_SYMM = ANEURALNETWORKS_TENSOR_QUANT8_SYMM,
    TENSOR_QUANT8_ASYMM_SIGNED = ANEURALNETWORKS_TENSOR_QUANT8_ASYMM_SIGNED,
    MODEL = ANEURALNETWORKS_MODEL,
};

enum class ExecutePreference {
    PREFER_LOW_POWER = ANEURALNETWORKS_PREFER_LOW_POWER,
    PREFER_FAST_SINGLE_ANSWER = ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER,
    PREFER_SUSTAINED_SPEED = ANEURALNETWORKS_PREFER_SUSTAINED_SPEED
};

enum class Duration {
    ON_HARDWARE = ANEURALNETWORKS_DURATION_ON_HARDWARE,
    IN_DRIVER = ANEURALNETWORKS_DURATION_IN_DRIVER,
    FENCED_ON_HARDWARE = ANEURALNETWORKS_FENCED_DURATION_ON_HARDWARE,
    FENCED_IN_DRIVER = ANEURALNETWORKS_FENCED_DURATION_IN_DRIVER,
};

enum class ExecutePriority {
    LOW = ANEURALNETWORKS_PRIORITY_LOW,
    MEDIUM = ANEURALNETWORKS_PRIORITY_MEDIUM,
    HIGH = ANEURALNETWORKS_PRIORITY_HIGH,
    DEFAULT = ANEURALNETWORKS_PRIORITY_DEFAULT,
};

enum class Result {
    NO_ERROR = ANEURALNETWORKS_NO_ERROR,
    OUT_OF_MEMORY = ANEURALNETWORKS_OUT_OF_MEMORY,
    INCOMPLETE = ANEURALNETWORKS_INCOMPLETE,
    UNEXPECTED_NULL = ANEURALNETWORKS_UNEXPECTED_NULL,
    BAD_DATA = ANEURALNETWORKS_BAD_DATA,
    OP_FAILED = ANEURALNETWORKS_OP_FAILED,
    UNMAPPABLE = ANEURALNETWORKS_UNMAPPABLE,
    BAD_STATE = ANEURALNETWORKS_BAD_STATE,
    OUTPUT_INSUFFICIENT_SIZE = ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE,
    UNAVAILABLE_DEVICE = ANEURALNETWORKS_UNAVAILABLE_DEVICE,
    MISSED_DEADLINE_TRANSIENT = ANEURALNETWORKS_MISSED_DEADLINE_TRANSIENT,
    MISSED_DEADLINE_PERSISTENT = ANEURALNETWORKS_MISSED_DEADLINE_PERSISTENT,

    // Functionality needed for this feature is not available on the current device.
    FEATURE_LEVEL_TOO_LOW = 100001,
};

struct SymmPerChannelQuantParams {
    ANeuralNetworksSymmPerChannelQuantParams params;
    std::vector<float> scales;

    SymmPerChannelQuantParams(std::vector<float> scalesVec, uint32_t channelDim)
        : scales(std::move(scalesVec)) {
        params = {
                .channelDim = channelDim,
                .scaleCount = static_cast<uint32_t>(scales.size()),
                .scales = scales.size() > 0 ? scales.data() : nullptr,
        };
    }

    SymmPerChannelQuantParams(const SymmPerChannelQuantParams& other)
        : params(other.params), scales(other.scales) {
        params.scales = scales.size() > 0 ? scales.data() : nullptr;
    }

    SymmPerChannelQuantParams& operator=(const SymmPerChannelQuantParams& other) {
        if (this != &other) {
            params = other.params;
            scales = other.scales;
            params.scales = scales.size() > 0 ? scales.data() : nullptr;
        }
        return *this;
    }
};

struct OperandType {
    ANeuralNetworksOperandType operandType;
    std::vector<uint32_t> dimensions;
    std::optional<SymmPerChannelQuantParams> channelQuant;

    OperandType(const OperandType& other)
        : operandType(other.operandType),
          dimensions(other.dimensions),
          channelQuant(other.channelQuant) {
        operandType.dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr;
    }

    OperandType& operator=(const OperandType& other) {
        if (this != &other) {
            operandType = other.operandType;
            dimensions = other.dimensions;
            channelQuant = other.channelQuant;
            operandType.dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr;
        }
        return *this;
    }

    OperandType(Type type, std::vector<uint32_t> d, float scale = 0.0f, int32_t zeroPoint = 0)
        : dimensions(std::move(d)), channelQuant(std::nullopt) {
        operandType = {
                .type = static_cast<int32_t>(type),
                .dimensionCount = static_cast<uint32_t>(dimensions.size()),
                .dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr,
                .scale = scale,
                .zeroPoint = zeroPoint,
        };
    }

    OperandType(Type type, std::vector<uint32_t> data, SymmPerChannelQuantParams&& channelQuant)
        : dimensions(std::move(data)), channelQuant(std::move(channelQuant)) {
        assert(type == Type::TENSOR_QUANT8_SYMM_PER_CHANNEL);

        operandType = {
                .type = static_cast<int32_t>(type),
                .dimensionCount = static_cast<uint32_t>(dimensions.size()),
                .dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr,
                .scale = 0.0f,
                .zeroPoint = 0,
        };
    }

    void updateDimensions(std::vector<uint32_t> ndim) {
        dimensions = ndim;
        operandType.dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr;
    }
};

#ifdef NNTEST_SLTS
#define NNAPI_CALL(apiCall) mNnApi->apiCall
#else
#define NNAPI_CALL(apiCall) apiCall
#endif

class Memory {
   public:
#ifdef NNTEST_SLTS
    // Takes ownership of a ANeuralNetworksMemory
    Memory(const NnApiSupportLibrary* nnapi, ANeuralNetworksMemory* memory)
        : mNnApi(nnapi), mMemory(memory) {}

    Memory(const NnApiSupportLibrary* nnapi, size_t size, int protect, int fd, size_t offset)
        : mNnApi(nnapi) {
#else
    Memory(size_t size, int protect, int fd, size_t offset) {
#endif
        mValid = NNAPI_CALL(ANeuralNetworksMemory_createFromFd(
                         size, protect, fd, offset, &mMemory)) == ANEURALNETWORKS_NO_ERROR;
    }

#ifdef __ANDROID__
#ifdef NNTEST_SLTS
    Memory(const NnApiSupportLibrary* nnapi, AHardwareBuffer* buffer) : mNnApi(nnapi) {
#else   // NNTEST_SLTS
    Memory(AHardwareBuffer* buffer) {
#endif  // NNTEST_SLTS
        mValid = NNAPI_CALL(ANeuralNetworksMemory_createFromAHardwareBuffer(buffer, &mMemory)) ==
                 ANEURALNETWORKS_NO_ERROR;
    }
#endif  // __ANDROID__

    ~Memory() {
        if (mMemory) {
            NNAPI_CALL(ANeuralNetworksMemory_free(mMemory));
        }
    }

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    // Move semantics to remove access to the runtime object from the wrapper
    // object that is being moved. This ensures the runtime object will be
    // freed only once.
    Memory(Memory&& other) { *this = std::move(other); }
    Memory& operator=(Memory&& other) {
        if (this != &other) {
            if (mMemory) {
                NNAPI_CALL(ANeuralNetworksMemory_free(mMemory));
            }
            mMemory = other.mMemory;
            mValid = other.mValid;
            other.mMemory = nullptr;
            other.mValid = false;
        }
        return *this;
    }

    ANeuralNetworksMemory* get() const { return mMemory; }
    bool isValid() const { return mValid; }

   private:
#ifdef NNTEST_SLTS
    const NnApiSupportLibrary* mNnApi = nullptr;
#endif
    ANeuralNetworksMemory* mMemory = nullptr;
    bool mValid = true;
};

class Model {
   public:
#ifdef NNTEST_SLTS
    Model(const NnApiSupportLibrary* nnapi) : mNnApi(nnapi) {
#else
    Model() {
#endif
        // TODO handle the value returned by this call
        NNAPI_CALL(ANeuralNetworksModel_create(&mModel));
    }
    ~Model() {
        if (mModel) {
            NNAPI_CALL(ANeuralNetworksModel_free(mModel));
        }
    }

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // Move semantics to remove access to the runtime object from the wrapper
    // object that is being moved. This ensures the runtime object will be
    // freed only once.
    Model(Model&& other) { *this = std::move(other); }
    Model& operator=(Model&& other) {
        if (this != &other) {
            if (mModel) {
                NNAPI_CALL(ANeuralNetworksModel_free(mModel));
            }
            mModel = other.mModel;
            mNextOperandId = other.mNextOperandId;
            mValid = other.mValid;
            other.mModel = nullptr;
            other.mNextOperandId = 0;
            other.mValid = false;
        }
        return *this;
    }

    Result finish() {
        if (mValid) {
            auto result = static_cast<Result>(NNAPI_CALL(ANeuralNetworksModel_finish(mModel)));
            if (result != Result::NO_ERROR) {
                mValid = false;
            }
            return result;
        } else {
            return Result::BAD_STATE;
        }
    }

    uint32_t addOperand(const OperandType* type) {
        if (NNAPI_CALL(ANeuralNetworksModel_addOperand(mModel, &(type->operandType))) !=
            ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
        if (type->channelQuant) {
            if (NNAPI_CALL(ANeuralNetworksModel_setOperandSymmPerChannelQuantParams(
                        mModel, mNextOperandId, &type->channelQuant.value().params)) !=
                ANEURALNETWORKS_NO_ERROR) {
                mValid = false;
            }
        }
        return mNextOperandId++;
    }

    void setOperandValue(uint32_t index, const void* buffer, size_t length) {
        if (NNAPI_CALL(ANeuralNetworksModel_setOperandValue(mModel, index, buffer, length)) !=
            ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
    }

    void setOperandValueFromMemory(uint32_t index, const Memory* memory, uint32_t offset,
                                   size_t length) {
        if (NNAPI_CALL(ANeuralNetworksModel_setOperandValueFromMemory(
                    mModel, index, memory->get(), offset, length)) != ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
    }

    void setOperandValueFromModel(uint32_t index, const Model* model) {
        if (__builtin_available(android /* Android R / FL4 */ 30, *)) {
            if (NNAPI_CALL(ANeuralNetworksModel_setOperandValueFromModel(
                        mModel, index, model->getHandle())) != ANEURALNETWORKS_NO_ERROR) {
                mValid = false;
            }
        } else {
            mValid = false;
        }
    }

    void addOperation(ANeuralNetworksOperationType type, const std::vector<uint32_t>& inputs,
                      const std::vector<uint32_t>& outputs) {
        if (NNAPI_CALL(ANeuralNetworksModel_addOperation(
                    mModel, type, static_cast<uint32_t>(inputs.size()), inputs.data(),
                    static_cast<uint32_t>(outputs.size()), outputs.data())) !=
            ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
    }
    void identifyInputsAndOutputs(const std::vector<uint32_t>& inputs,
                                  const std::vector<uint32_t>& outputs) {
        if (NNAPI_CALL(ANeuralNetworksModel_identifyInputsAndOutputs(
                    mModel, static_cast<uint32_t>(inputs.size()), inputs.data(),
                    static_cast<uint32_t>(outputs.size()), outputs.data())) !=
            ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
    }

    void relaxComputationFloat32toFloat16(bool isRelax) {
        if (NNAPI_CALL(ANeuralNetworksModel_relaxComputationFloat32toFloat16(mModel, isRelax)) ==
            ANEURALNETWORKS_NO_ERROR) {
            mRelaxed = isRelax;
        }
    }

    ANeuralNetworksModel* getHandle() const { return mModel; }
    bool isValid() const { return mValid; }
    bool isRelaxed() const { return mRelaxed; }

#ifdef NNTEST_SLTS
   private:
    const NnApiSupportLibrary* mNnApi = nullptr;
#endif

   protected:
    ANeuralNetworksModel* mModel = nullptr;
    // We keep track of the operand ID as a convenience to the caller.
    uint32_t mNextOperandId = 0;
    bool mValid = true;
    bool mRelaxed = false;
};

class Event {
   public:
#ifdef NNTEST_SLTS
    Event(const NnApiSupportLibrary* nnapi) : mNnApi(nnapi) {}
    Event(const NnApiSupportLibrary* nnapi, int syncFd) : mNnApi(nnapi) {
#else
    Event() {}
    Event(int syncFd) {
#endif
        mValid = NNAPI_CALL(ANeuralNetworksEvent_createFromSyncFenceFd(syncFd, &mEvent)) ==
                 ANEURALNETWORKS_NO_ERROR;
    }

    ~Event() {
        if (mEvent) {
            NNAPI_CALL(ANeuralNetworksEvent_free(mEvent));
        }
    }

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    // Move semantics to remove access to the runtime object from the wrapper
    // object that is being moved. This ensures the runtime object will be
    // freed only once.
    Event(Event&& other) { *this = std::move(other); }
    Event& operator=(Event&& other) {
        if (this != &other) {
            if (mEvent) {
                NNAPI_CALL(ANeuralNetworksEvent_free(mEvent));
            }
#ifdef NNTEST_SLTS
            mNnApi = other.mNnApi;
#endif
            mEvent = other.mEvent;
            other.mEvent = nullptr;
        }
        return *this;
    }

    Result wait() { return static_cast<Result>(NNAPI_CALL(ANeuralNetworksEvent_wait(mEvent))); }

    // Only for use by Execution
    void set(ANeuralNetworksEvent* newEvent) {
        if (mEvent) {
            NNAPI_CALL(ANeuralNetworksEvent_free(mEvent));
        }
        mEvent = newEvent;
    }

    // Only for use by Execution
    ANeuralNetworksEvent* getHandle() const { return mEvent; }

    Result getSyncFenceFd(int* sync_fence_fd) {
        return static_cast<Result>(
                NNAPI_CALL(ANeuralNetworksEvent_getSyncFenceFd(mEvent, sync_fence_fd)));
    }

    bool isValid() const { return mValid; }

#ifdef NNTEST_SLTS
   private:
    const NnApiSupportLibrary* mNnApi = nullptr;
#endif

   private:
    bool mValid = true;
    ANeuralNetworksEvent* mEvent = nullptr;
};

class Compilation {
   public:
#ifdef NNTEST_SLTS
    // On success, createForDevice(s) will return Result::NO_ERROR and the created compilation;
    // otherwise, it will return the error code and Compilation object wrapping a nullptr handle.
    static std::pair<Result, Compilation> createForDevice(const NnApiSupportLibrary* nnapi,
                                                          const Model* model,
                                                          const ANeuralNetworksDevice* device) {
        return createForDevices(nnapi, model, {device});
    }
    static std::pair<Result, Compilation> createForDevices(
            const NnApiSupportLibrary* nnapi, const Model* model,
            const std::vector<const ANeuralNetworksDevice*>& devices) {
        ANeuralNetworksCompilation* compilation = nullptr;
        const Result result =
                static_cast<Result>(nnapi->ANeuralNetworksCompilation_createForDevices(
                        model->getHandle(), devices.empty() ? nullptr : devices.data(),
                        devices.size(), &compilation));
        return {result, Compilation(nnapi, compilation)};
    }
#else
    Compilation(const Model* model) {
        int result =
                NNAPI_CALL(ANeuralNetworksCompilation_create(model->getHandle(), &mCompilation));
        if (result != 0) {
            // TODO Handle the error
        }
    }
#endif

    ~Compilation() { NNAPI_CALL(ANeuralNetworksCompilation_free(mCompilation)); }

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Compilation(const Compilation&) = delete;
    Compilation& operator=(const Compilation&) = delete;

    // Move semantics to remove access to the runtime object from the wrapper
    // object that is being moved. This ensures the runtime object will be
    // freed only once.
    Compilation(Compilation&& other) { *this = std::move(other); }
    Compilation& operator=(Compilation&& other) {
        if (this != &other) {
            NNAPI_CALL(ANeuralNetworksCompilation_free(mCompilation));
            mCompilation = other.mCompilation;
            other.mCompilation = nullptr;
        }
        return *this;
    }

    Result setPreference(ExecutePreference preference) {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksCompilation_setPreference(
                mCompilation, static_cast<int32_t>(preference))));
    }

    Result setPriority(ExecutePriority priority) {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksCompilation_setPriority(
                mCompilation, static_cast<int32_t>(priority))));
    }

    Result setCaching(const std::string& cacheDir, const std::vector<uint8_t>& token) {
        if (token.size() != ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN) {
            return Result::BAD_DATA;
        }
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksCompilation_setCaching(
                mCompilation, cacheDir.c_str(), token.data())));
    }

    Result finish() {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksCompilation_finish(mCompilation)));
    }

    Result getPreferredMemoryAlignmentForInput(uint32_t index, uint32_t* alignment) const {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(
                    NNAPI_CALL(ANeuralNetworksCompilation_getPreferredMemoryAlignmentForInput(
                            mCompilation, index, alignment)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    };

    Result getPreferredMemoryPaddingForInput(uint32_t index, uint32_t* padding) const {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(
                    NNAPI_CALL(ANeuralNetworksCompilation_getPreferredMemoryPaddingForInput(
                            mCompilation, index, padding)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    };

    Result getPreferredMemoryAlignmentForOutput(uint32_t index, uint32_t* alignment) const {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(
                    NNAPI_CALL(ANeuralNetworksCompilation_getPreferredMemoryAlignmentForOutput(
                            mCompilation, index, alignment)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    };

    Result getPreferredMemoryPaddingForOutput(uint32_t index, uint32_t* padding) const {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(
                    NNAPI_CALL(ANeuralNetworksCompilation_getPreferredMemoryPaddingForOutput(
                            mCompilation, index, padding)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    };

    ANeuralNetworksCompilation* getHandle() const { return mCompilation; }

#ifdef NNTEST_SLTS
   protected:
    // Takes the ownership of ANeuralNetworksCompilation.
    Compilation(const NnApiSupportLibrary* nnapi, ANeuralNetworksCompilation* compilation)
        : mNnApi(nnapi), mCompilation(compilation) {}

   private:
    const NnApiSupportLibrary* mNnApi = nullptr;
#else
   private:
#endif
    ANeuralNetworksCompilation* mCompilation = nullptr;
};

class Execution {
   public:
#ifdef NNTEST_SLTS
    Execution(const NnApiSupportLibrary* nnapi, const Compilation* compilation) : mNnApi(nnapi) {
#else
    Execution(const Compilation* compilation) {
#endif
        int result =
                NNAPI_CALL(ANeuralNetworksExecution_create(compilation->getHandle(), &mExecution));
        if (result != 0) {
            // TODO Handle the error
        }
    }

    ~Execution() {
        if (mExecution) {
            NNAPI_CALL(ANeuralNetworksExecution_free(mExecution));
        }
    }

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Execution(const Execution&) = delete;
    Execution& operator=(const Execution&) = delete;

    // Move semantics to remove access to the runtime object from the wrapper
    // object that is being moved. This ensures the runtime object will be
    // freed only once.
    Execution(Execution&& other) { *this = std::move(other); }
    Execution& operator=(Execution&& other) {
        if (this != &other) {
            if (mExecution) {
                NNAPI_CALL(ANeuralNetworksExecution_free(mExecution));
            }
            mExecution = other.mExecution;
            other.mExecution = nullptr;
        }
        return *this;
    }

    Result setInput(uint32_t index, const void* buffer, size_t length,
                    const ANeuralNetworksOperandType* type = nullptr) {
        return static_cast<Result>(NNAPI_CALL(
                ANeuralNetworksExecution_setInput(mExecution, index, type, buffer, length)));
    }

    Result setInputFromMemory(uint32_t index, const Memory* memory, uint32_t offset,
                              uint32_t length, const ANeuralNetworksOperandType* type = nullptr) {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksExecution_setInputFromMemory(
                mExecution, index, type, memory->get(), offset, length)));
    }

    Result setOutput(uint32_t index, void* buffer, size_t length,
                     const ANeuralNetworksOperandType* type = nullptr) {
        return static_cast<Result>(NNAPI_CALL(
                ANeuralNetworksExecution_setOutput(mExecution, index, type, buffer, length)));
    }

    Result setOutputFromMemory(uint32_t index, const Memory* memory, uint32_t offset,
                               uint32_t length, const ANeuralNetworksOperandType* type = nullptr) {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksExecution_setOutputFromMemory(
                mExecution, index, type, memory->get(), offset, length)));
    }

    Result enableInputAndOutputPadding(bool enable) {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(NNAPI_CALL(
                    ANeuralNetworksExecution_enableInputAndOutputPadding(mExecution, enable)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    }

    Result setReusable(bool reusable) {
        if (__builtin_available(android __NNAPI_FL5_MIN_ANDROID_API__, *)) {
            return static_cast<Result>(
                    NNAPI_CALL(ANeuralNetworksExecution_setReusable(mExecution, reusable)));
        } else {
            return Result::FEATURE_LEVEL_TOO_LOW;
        }
    }

#ifndef NNTEST_SLTS
    Result startCompute(Event* event) {
        ANeuralNetworksEvent* ev = nullptr;
        Result result = static_cast<Result>(
                NNAPI_CALL(ANeuralNetworksExecution_startCompute(mExecution, &ev)));
        event->set(ev);
        return result;
    }

    Result startComputeWithDependencies(const std::vector<const Event*>& dependencies,
                                        uint64_t duration, Event* event) {
        std::vector<const ANeuralNetworksEvent*> deps(dependencies.size());
        std::transform(dependencies.begin(), dependencies.end(), deps.begin(),
                       [](const Event* e) { return e->getHandle(); });
        ANeuralNetworksEvent* ev = nullptr;
        Result result = static_cast<Result>(
                NNAPI_CALL(ANeuralNetworksExecution_startComputeWithDependencies(
                        mExecution, deps.data(), deps.size(), duration, &ev)));
        event->set(ev);
        return result;
    }
#endif

    Result compute() {
        return static_cast<Result>(NNAPI_CALL(ANeuralNetworksExecution_compute(mExecution)));
    }

    Result getOutputOperandDimensions(uint32_t index, std::vector<uint32_t>* dimensions) {
        uint32_t rank = 0;
        Result result = static_cast<Result>(NNAPI_CALL(
                ANeuralNetworksExecution_getOutputOperandRank(mExecution, index, &rank)));
        dimensions->resize(rank);
        if ((result != Result::NO_ERROR && result != Result::OUTPUT_INSUFFICIENT_SIZE) ||
            rank == 0) {
            return result;
        }
        result = static_cast<Result>(NNAPI_CALL(ANeuralNetworksExecution_getOutputOperandDimensions(
                mExecution, index, dimensions->data())));
        return result;
    }

   private:
#ifdef NNTEST_SLTS
    const NnApiSupportLibrary* mNnApi = nullptr;
#endif
    ANeuralNetworksExecution* mExecution = nullptr;
};

}  // namespace wrapper
}  // namespace nn
}  // namespace android

#endif  //  ANDROID_PACKAGES_MODULES_NEURALNETWORKS_RUNTIME_NEURAL_NETWORKS_WRAPPER_H
