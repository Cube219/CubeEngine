#pragma once

#include "CoreHeader.h"

#include "Checker.h"
#include "GAPI_Buffer.h"
#include "GAPI_ShaderParameter.h"
#include "Matrix.h"
#include "Renderer/RenderTypes.h"

#ifndef CUBE_CHECK_PARAMETERS
#define CUBE_CHECK_PARAMETERS 1
#endif

#if CUBE_CHECK_PARAMETERS
#define CHECK_PARAMS(x) CHECK(x)
#else
#define CHECK_PARAMS()
#endif

namespace cube
{
    class GAPI;

    namespace gapi
    {
        class Buffer;
    } // namespace gapi

    class ShaderParametersManager;

    // ===== ParameterTypeInfo =====
    // Note: Also add in SlangHelperPrivate::GetReflection.
    enum class ShaderParameterType
    {
        Bool,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Matrix,
        Bindless
    };

    template <typename T>
    struct NoEntry : std::false_type {};

    template <typename T>
    struct ShaderParameterTypeInfo
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Bool;
        static constexpr Uint32 size = sizeof(bool);
        
        static_assert(NoEntry<T>::value, "ShaderParameterTypeInfo is not specialized for this type.");
    };

    template <>
    struct ShaderParameterTypeInfo<bool>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Bool;
        static constexpr Uint32 size = sizeof(bool);
    };

    template <>
    struct ShaderParameterTypeInfo<Vector2>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Float2;
        static constexpr Uint32 size = sizeof(Float2);
    };

    template <>
    struct ShaderParameterTypeInfo<Vector3>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Float3;
        static constexpr Uint32 size = sizeof(Float3);
    };

    template <>
    struct ShaderParameterTypeInfo<Vector4>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Float4;
        static constexpr Uint32 size = sizeof(Float4);
    };
    
    template <>
    struct ShaderParameterTypeInfo<Matrix>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Matrix;
        static constexpr Uint32 size = sizeof(Matrix);
    };

    template <>
    struct ShaderParameterTypeInfo<BindlessResource>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Bindless;
        static constexpr Uint32 size = sizeof(BindlessResource);
    };

    // ===== ShaderParameters =====
    class ShaderParameters;

    struct ShaderParameterInfo
    {
        const char* name;
        ShaderParameterType type;

        Uint32 offsetInCPU;
        Uint32 sizeInCPU;

        Uint32 offsetInGPU; // Set in gapi::ShaderParameterHelper
        Uint32 sizeInGPU; // Set in gapi::ShaderParameterHelper
    };

    template <typename TShaderParameters>
        requires std::is_base_of_v<ShaderParameters, TShaderParameters>
    struct ShaderParametersInfo
    {
        static inline bool isInitialized = false;

        static inline Vector<ShaderParameterInfo> parameterInfos;
        static inline Uint32 totalBufferSize;
    };

    struct ShaderParametersPooledBuffer
    {
        SharedPtr<gapi::Buffer> buffer;
        Uint32 poolIndex;
    };

    class ShaderParameters
    {
    public:
        ShaderParameters(const ShaderParameters& other) = delete;
        ShaderParameters& operator=(const ShaderParameters& rhs) = delete;

        SharedPtr<gapi::Buffer> GetBuffer() const { return mPooledBuffer.buffer; }

    protected:
        // Only manager can create shader parameters
        friend class ShaderParametersManager;

        ShaderParameters(ShaderParametersManager& manager);
        virtual ~ShaderParameters();

        ShaderParametersManager& mManager;

        Uint32 mGPUSyncIndex;
        ShaderParametersPooledBuffer mPooledBuffer;
    };

#define CUBE_BEGIN_SHADER_PARAMETERS(parametersType) \
    \
    using ParametersType = parametersType; \
    \
public: \
    friend class ShaderParametersManager; \
    parametersType(ShaderParametersManager& manager) : ShaderParameters(manager) {} \
    \
private: \
    static const Character* GetDebugName() \
    { \
        return CUBE_T(#parametersType); \
    } \
    \
    struct ParameterIterHelperBegin \
    { \
        static void InitializeParameterInfo() {} \
    }; \
    typedef ParameterIterHelperBegin

#define CUBE_SHADER_PARAMETER(paramType, paramName) \
    ParameterIterHelper_##paramName##_Prev; \
    \
    struct ParameterIterHelper_##paramName \
    { \
        static void InitializeParameterInfo() \
        { \
            ParameterIterHelper_##paramName##_Prev::InitializeParameterInfo(); \
            \
            ShaderParametersInfo<ParametersType>::parameterInfos.emplace_back(); \
            auto& paramInfo = ShaderParametersInfo<ParametersType>::parameterInfos.back(); \
            \
            paramInfo.name = #paramName; \
            paramInfo.type = ShaderParameterTypeInfo<paramType>::type; \
            paramInfo.sizeInCPU = ShaderParameterTypeInfo<paramType>::size; \
            paramInfo.offsetInCPU = offsetof(ParametersType, paramName); \
        } \
    }; \
    \
public: \
    paramType paramName; \
    \
private: \
    typedef ParameterIterHelper_##paramName

#define CUBE_END_SHADER_PARAMETERS \
    ParameterIterHelperEnd_Prev; \
    \
    struct ParameterIterHelperEnd \
    { \
        static void InitializeParameterInfo() \
        { \
            ParameterIterHelperEnd_Prev::InitializeParameterInfo(); \
        } \
    }; \
    \
public: \
    static void InitializeParametersInfo(const gapi::ShaderParameterHelper& shaderParemeterHelper) \
    { \
        if (!ShaderParametersInfo<ParametersType>::isInitialized) \
        { \
            ParameterIterHelperEnd::InitializeParameterInfo(); \
            shaderParemeterHelper.UpdateShaderParameterInfo(ShaderParametersInfo<ParametersType>::parameterInfos, ShaderParametersInfo<ParametersType>::totalBufferSize); \
            ShaderParametersInfo<ParametersType>::isInitialized = true; \
        } \
    } \
    void WriteAllParametersToBuffer() \
    { \
        mManager.GetShaderParameterHelper().WriteParametersToBuffer( \
            mPooledBuffer.buffer, \
            ShaderParametersInfo<ParametersType>::parameterInfos, \
            this); \
    }

    // ===== ShaderParametersManager =====

    class ShaderParametersManager
    {
    public:
        ShaderParametersManager() = default;
        ~ShaderParametersManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        const gapi::ShaderParameterHelper& GetShaderParameterHelper() const { return *mShaderParameterHelper; }

        void MoveNextFrame();

        template <typename T>
            requires std::derived_from<T, ShaderParameters>
        SharedPtr<T> CreateShaderParameters()
        {
            SharedPtr<T> parameters = std::make_shared<T>(*this);
            T::InitializeParametersInfo(*mShaderParameterHelper);

            AllocateShaderParameters(parameters.get(), ShaderParametersInfo<T>::parameterInfos, ShaderParametersInfo<T>::totalBufferSize, T::GetDebugName());

            return parameters;
        }

    private:
        friend class ShaderParameters;

        struct ShaderParametersBufferPool;

        void AllocateShaderParameters(ShaderParameters* parameters, const Vector<ShaderParameterInfo>& paramInfos, Uint32 bufferSize, StringView debugName);

        ShaderParametersPooledBuffer AllocateBuffer(Uint32 size, StringView debugName);
        void FreeBuffer(ShaderParameters& parameters);

        GAPI* mGAPI;
        const gapi::ShaderParameterHelper* mShaderParameterHelper;

        Uint32 mCurrentIndex;

        struct ShaderParametersBufferPool
        {
            Vector<SharedPtr<gapi::Buffer>> buffers;
            MultiMap<Uint32, Uint32> pooledBufferIndices;
            Vector<Uint32> freedBufferIndices;

            void CheckConsistency();

            void Clear()
            {
                CheckConsistency();

                buffers.clear();
                pooledBufferIndices.clear();
                freedBufferIndices.clear();
            }
        };
        Vector<ShaderParametersBufferPool> mBufferPools;
    };
} // namespace cube
