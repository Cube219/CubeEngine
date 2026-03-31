#pragma once

#include "CoreHeader.h"

#include "Checker.h"
#include "GAPI_Buffer.h"
#include "GAPI_ShaderParameter.h"
#include "GAPI_ShaderReflection.h"
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
    // Note: Also update CompatibleShaderParameterReflectionTypeMap each ShaderParameterHelper implementations.
    enum class ShaderParameterType
    {
        Unknown,
        Bool,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Matrix,
        BindlessTexture,
        BindlessSampler,
        BindlessCombinedTextureSampler,
        RGTextureSRV,
        RGTextureUAV,

        Num
    };
    // TODO: Use formatter style?
    inline const Character* ToString(ShaderParameterType type)
    {
        switch (type)
        {
        case ShaderParameterType::Unknown:
            return CUBE_T("Unknown");
        case ShaderParameterType::Bool:
            return CUBE_T("Bool");
        case ShaderParameterType::Int:
            return CUBE_T("Int");
        case ShaderParameterType::Float:
            return CUBE_T("Float");
        case ShaderParameterType::Float2:
            return CUBE_T("Float2");
        case ShaderParameterType::Float3:
            return CUBE_T("Float3");
        case ShaderParameterType::Float4:
            return CUBE_T("Float4");
        case ShaderParameterType::Matrix:
            return CUBE_T("Matrix");
        case ShaderParameterType::BindlessTexture:
            return CUBE_T("BindlessTexture");
        case ShaderParameterType::BindlessSampler:
            return CUBE_T("BindlessSampler");
        case ShaderParameterType::BindlessCombinedTextureSampler:
            return CUBE_T("BindlessCombinedTextureSampler");
        case ShaderParameterType::RGTextureSRV:
            return CUBE_T("RGTextureSRV");
        case ShaderParameterType::RGTextureUAV:
            return CUBE_T("RGTextureUAV");
        default:
            break;
        }
        return CUBE_T("Undefined");
    }

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
    struct ShaderParameterTypeInfo<float>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Float;
        static constexpr Uint32 size = sizeof(float);
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
    struct ShaderParameterTypeInfo<BindlessTexture>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::BindlessTexture;
        static constexpr Uint32 size = sizeof(BindlessTexture);
    };

    template <>
    struct ShaderParameterTypeInfo<BindlessSampler>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::BindlessSampler;
        static constexpr Uint32 size = sizeof(BindlessSampler);
    };

    template <>
    struct ShaderParameterTypeInfo<BindlessCombinedTextureSampler>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::BindlessCombinedTextureSampler;
        static constexpr Uint32 size = sizeof(BindlessCombinedTextureSampler);
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

    // TODO: Change name (s -> List)
    struct ShaderParametersInfo
    {
        const Character* name;

        Vector<ShaderParameterInfo> parameterInfos;
        Uint32 totalBufferSize;
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
        ShaderParametersManager& GetManager() const { return mManager; }

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
    static const Character* GetName() \
    { \
        return CUBE_T(#parametersType); \
    } \
    private: \
    \
    struct ParameterIterHelperBegin \
    { \
        static void InitializeParameterInfo(ShaderParametersInfo& infos) {} \
    }; \
    typedef ParameterIterHelperBegin

#define CUBE_SHADER_PARAMETER(paramType, paramName) \
    ParameterIterHelper_##paramName##_Prev; \
    \
    struct ParameterIterHelper_##paramName \
    { \
        static void InitializeParameterInfo(ShaderParametersInfo& infos) \
        { \
            ParameterIterHelper_##paramName##_Prev::InitializeParameterInfo(infos); \
            \
            infos.parameterInfos.emplace_back(); \
            auto& paramInfo = infos.parameterInfos.back(); \
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
        static void InitializeParameterInfo(ShaderParametersInfo& infos) \
        { \
            ParameterIterHelperEnd_Prev::InitializeParameterInfo(infos); \
        } \
    }; \
    \
public: \
    static void InitializeParametersInfo(const gapi::ShaderParameterHelper& shaderParemeterHelper, ShaderParametersInfo& infos) \
    { \
        infos.name = GetName(); \
        ParameterIterHelperEnd::InitializeParameterInfo(infos); \
        shaderParemeterHelper.UpdateShaderParametersInfo(infos); \
    } \
    void WriteAllParametersToGPUBuffer() \
    { \
        mManager.GetShaderParameterHelper().WriteParametersToGPUBuffer( \
            mPooledBuffer.buffer, \
            ShaderParametersManager::GetShaderParametersInfo<ParametersType>(), \
            this); \
    }

#define CUBE_REGISTER_SHADER_PARAMETERS(parametersType) \
    namespace internal \
    { \
        struct RegisterShaderParameters_##parametersType \
        { \
            using ParametersType = parametersType; \
            \
            RegisterShaderParameters_##parametersType() \
            { \
                ShaderParametersManager::AddDeferredInitializingParameterInfos({ ParametersType::GetName(), &ParametersType::InitializeParametersInfo }); \
            } \
        }; \
        static RegisterShaderParameters_##parametersType gRegisterShaderParameters_##parametersType; \
    }


    // ===== ShaderParametersManager =====

    class ShaderParametersManager
    {
    public:
        struct DeferredInitializingParameterInfos
        {
            const Character* typeName;
            void (*initFunction)(const gapi::ShaderParameterHelper&, ShaderParametersInfo&);
        };
        static void AddDeferredInitializingParameterInfos(const DeferredInitializingParameterInfos& initInfos);

        template <typename T>
            requires std::derived_from<T, ShaderParameters>
        static const ShaderParametersInfo& GetShaderParametersInfo()
        {
            static int cachedIndex = -1;
            if (cachedIndex == -1)
            {
                cachedIndex = GetShaderParametersInfoIndex(T::GetName());
            }

            return mShaderParametersInfos[cachedIndex];
        }
        static const ShaderParametersInfo& GetShaderParametersInfo(StringView parametersTypeName)
        {
            return mShaderParametersInfos[GetShaderParametersInfoIndex(parametersTypeName)];
        }

    private:
        static void ProcessDeferredInitializingParametersInfos(const gapi::ShaderParameterHelper& shaderParemeterHelper);

        static int GetShaderParametersInfoIndex(StringView parametersTypeName);

        static constexpr int MAX_NUM_DEFERRED_INIT = 1024;
        static DeferredInitializingParameterInfos mDeferredInitializingParametersInfos[MAX_NUM_DEFERRED_INIT];
        static int mDeferredInitializingParametersInfosIndex;
        static bool mIsDeferredInitOverflow;

        static Vector<ShaderParametersInfo> mShaderParametersInfos;
        static Map<String, int> mShaderParametersTypeNameToIndexMap;

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

            AllocateShaderParameters(parameters.get(), GetShaderParametersInfo<T>());

            return parameters;
        }

        bool ValidateShaderParameters(const Vector<ShaderParameterInfo>& parameterInfos, const gapi::ShaderParameterBlockReflection& parameterBlockReflection);

    private:
        friend class ShaderParameters;

        struct ShaderParametersBufferPool;

        void AllocateShaderParameters(ShaderParameters* parameters, const ShaderParametersInfo& parametersInfo);

        ShaderParametersPooledBuffer AllocateBuffer(Uint32 size, StringView debugName);
        void FreeBuffer(ShaderParameters& parameters);

        GAPI* mGAPI;
        const gapi::ShaderParameterHelper* mShaderParameterHelper;
        Vector<Vector<gapi::ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;

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
