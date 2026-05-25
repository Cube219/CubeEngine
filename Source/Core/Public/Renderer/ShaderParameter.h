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

    class ShaderParameterListManager;

    // ===== ParameterTypeInfo =====
    // Note: Also update CompatibleShaderParameterReflectionTypeMap each ShaderParameterHelper implementations.
    enum class ShaderParameterCPUType
    {
        Unknown,
        Bool,
        Int,
        Int2,
        Int3,
        Int4,
        Uint,
        Uint2,
        Uint3,
        Uint4,
        Float,
        Float2,
        Float3,
        Float4,
        Vector2,
        Vector3,
        Vector4,
        Matrix,
        BindlessTexture,
        BindlessSampler,
        BindlessCombinedTextureSampler,
        RGBufferSRV,
        RGBufferUAV,
        RGTextureSRV,
        RGTextureUAV,

        Num
    };

    inline const Character* ToString(ShaderParameterCPUType type)
    {
        switch (type)
        {
        case ShaderParameterCPUType::Unknown:
            return CUBE_T("Unknown");
        case ShaderParameterCPUType::Bool:
            return CUBE_T("Bool");
        case ShaderParameterCPUType::Int:
            return CUBE_T("Int");
        case ShaderParameterCPUType::Int2:
            return CUBE_T("Int2");
        case ShaderParameterCPUType::Int3:
            return CUBE_T("Int3");
        case ShaderParameterCPUType::Int4:
            return CUBE_T("Int4");
        case ShaderParameterCPUType::Uint:
            return CUBE_T("Uint");
        case ShaderParameterCPUType::Uint2:
            return CUBE_T("Uint2");
        case ShaderParameterCPUType::Uint3:
            return CUBE_T("Uint3");
        case ShaderParameterCPUType::Uint4:
            return CUBE_T("Uint4");
        case ShaderParameterCPUType::Float:
            return CUBE_T("Float");
        case ShaderParameterCPUType::Float2:
            return CUBE_T("Float2");
        case ShaderParameterCPUType::Float3:
            return CUBE_T("Float3");
        case ShaderParameterCPUType::Float4:
            return CUBE_T("Float4");
        case ShaderParameterCPUType::Vector2:
            return CUBE_T("Vector2");
        case ShaderParameterCPUType::Vector3:
            return CUBE_T("Vector3");
        case ShaderParameterCPUType::Vector4:
            return CUBE_T("Vector4");
        case ShaderParameterCPUType::Matrix:
            return CUBE_T("Matrix");
        case ShaderParameterCPUType::BindlessTexture:
            return CUBE_T("BindlessTexture");
        case ShaderParameterCPUType::BindlessSampler:
            return CUBE_T("BindlessSampler");
        case ShaderParameterCPUType::BindlessCombinedTextureSampler:
            return CUBE_T("BindlessCombinedTextureSampler");
        case ShaderParameterCPUType::RGBufferSRV:
            return CUBE_T("RGBufferSRV");
        case ShaderParameterCPUType::RGBufferUAV:
            return CUBE_T("RGBufferUAV");
        case ShaderParameterCPUType::RGTextureSRV:
            return CUBE_T("RGTextureSRV");
        case ShaderParameterCPUType::RGTextureUAV:
            return CUBE_T("RGTextureUAV");
        default:
            break;
        }
        return CUBE_T("Undefined");
    }

    template <typename T>
    struct NoEntry : std::false_type {};

    template <typename T>
    struct ShaderParameterCPUTypeInfo
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Bool;
        static constexpr Uint32 size = sizeof(bool);
        
        static_assert(NoEntry<T>::value, "ShaderParameterTypeInfo is not specialized for this type.");
    };

    template <>
    struct ShaderParameterCPUTypeInfo<bool>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Bool;
        static constexpr Uint32 size = sizeof(bool);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<int>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Int;
        static constexpr Uint32 size = sizeof(int);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Int2>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Int2;
        static constexpr Uint32 size = sizeof(Int2);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Int3>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Int3;
        static constexpr Uint32 size = sizeof(Int3);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Int4>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Int4;
        static constexpr Uint32 size = sizeof(Int4);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Uint32>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Uint;
        static constexpr Uint32 size = sizeof(Uint32);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Uint2>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Uint2;
        static constexpr Uint32 size = sizeof(Uint2);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Uint3>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Uint3;
        static constexpr Uint32 size = sizeof(Uint3);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Uint4>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Uint4;
        static constexpr Uint32 size = sizeof(Uint4);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<float>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Float;
        static constexpr Uint32 size = sizeof(float);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Float2>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Float2;
        static constexpr Uint32 size = sizeof(Float2);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Float3>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Float3;
        static constexpr Uint32 size = sizeof(Float3);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Float4>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Float4;
        static constexpr Uint32 size = sizeof(Float4);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Vector2>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Vector2;
        static constexpr Uint32 size = sizeof(Vector2);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Vector3>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Vector3;
        static constexpr Uint32 size = sizeof(Vector3);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Vector4>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Vector4;
        static constexpr Uint32 size = sizeof(Vector4);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<Matrix>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::Matrix;
        static constexpr Uint32 size = sizeof(Matrix);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<BindlessTexture>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::BindlessTexture;
        static constexpr Uint32 size = sizeof(BindlessTexture);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<BindlessSampler>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::BindlessSampler;
        static constexpr Uint32 size = sizeof(BindlessSampler);
    };

    template <>
    struct ShaderParameterCPUTypeInfo<BindlessCombinedTextureSampler>
    {
        static constexpr ShaderParameterCPUType type = ShaderParameterCPUType::BindlessCombinedTextureSampler;
        static constexpr Uint32 size = sizeof(BindlessCombinedTextureSampler);
    };

    // ===== ShaderParameterList =====
    class ShaderParameterList;

    struct ShaderParameterInfo
    {
        const char* name;
        ShaderParameterCPUType type;

        Uint32 offsetInCPU;
        Uint32 sizeInCPU;

        Uint32 offsetInGPU; // Set in gapi::ShaderParameterHelper
        Uint32 sizeInGPU; // Set in gapi::ShaderParameterHelper
    };

    struct ShaderParameterListInfo
    {
        const Character* name;

        Vector<ShaderParameterInfo> parameterInfos;
        Uint32 totalBufferSize;
    };

    struct ShaderParameterListPooledBuffer
    {
        SharedPtr<gapi::Buffer> buffer;
        SharedPtr<gapi::BufferSRV> srv;
        Uint32 poolIndex;
    };

    class ShaderParameterList
    {
    public:
        ShaderParameterList(const ShaderParameterList& other) = delete;
        ShaderParameterList& operator=(const ShaderParameterList& rhs) = delete;

        SharedPtr<gapi::Buffer> GetBuffer() const { return mPooledBuffer.buffer; }
        SharedPtr<gapi::BufferSRV> GetSRV() const { return mPooledBuffer.srv; }
        ShaderParameterListManager& GetManager() const { return mManager; }

        virtual void WriteAllParametersToGPUBuffer() = 0;

    protected:
        // Only manager can create shader parameter list
        friend class ShaderParameterListManager;

        ShaderParameterList(ShaderParameterListManager& manager);
        virtual ~ShaderParameterList();

        ShaderParameterListManager& mManager;

        Uint32 mGPUSyncIndex;
        ShaderParameterListPooledBuffer mPooledBuffer;
    };

#define CUBE_BEGIN_SHADER_PARAMETER_LIST(parameterListType) \
    \
    using ParameterListType = parameterListType; \
    \
public: \
    friend class ShaderParameterListManager; \
    parameterListType(ShaderParameterListManager& manager) : ShaderParameterList(manager) {} \
    \
    static const Character* GetName() \
    { \
        return CUBE_T(#parameterListType); \
    } \
    private: \
    \
    struct ParameterIterHelperBegin \
    { \
        static void InitializeParameterInfo(ShaderParameterListInfo& infos) {} \
    }; \
    typedef ParameterIterHelperBegin

#define CUBE_SHADER_PARAMETER(paramType, paramName) \
    ParameterIterHelper_##paramName##_Prev; \
    \
    struct ParameterIterHelper_##paramName \
    { \
        static void InitializeParameterInfo(ShaderParameterListInfo& infos) \
        { \
            ParameterIterHelper_##paramName##_Prev::InitializeParameterInfo(infos); \
            \
            infos.parameterInfos.emplace_back(); \
            auto& paramInfo = infos.parameterInfos.back(); \
            \
            paramInfo.name = #paramName; \
            paramInfo.type = ShaderParameterCPUTypeInfo<paramType>::type; \
            paramInfo.sizeInCPU = ShaderParameterCPUTypeInfo<paramType>::size; \
            paramInfo.offsetInCPU = offsetof(ParameterListType, paramName); \
        } \
    }; \
    \
public: \
    paramType paramName; \
    \
private: \
    typedef ParameterIterHelper_##paramName

#define CUBE_END_SHADER_PARAMETER_LIST \
    ParameterIterHelperEnd_Prev; \
    \
    struct ParameterIterHelperEnd \
    { \
        static void InitializeParameterInfo(ShaderParameterListInfo& infos) \
        { \
            ParameterIterHelperEnd_Prev::InitializeParameterInfo(infos); \
        } \
    }; \
    \
public: \
    static void InitializeParameterListInfo(const gapi::ShaderParameterHelper& shaderParemeterHelper, ShaderParameterListInfo& infos) \
    { \
        infos.name = GetName(); \
        ParameterIterHelperEnd::InitializeParameterInfo(infos); \
        shaderParemeterHelper.UpdateShaderParameterListInfo(infos); \
    } \
    virtual void WriteAllParametersToGPUBuffer() override \
    { \
        mManager.GetShaderParameterHelper().WriteParametersToGPUBuffer( \
            mPooledBuffer.buffer, \
            ShaderParameterListManager::GetShaderParameterListInfo<ParameterListType>(), \
            this); \
    }

#define CUBE_REGISTER_SHADER_PARAMETER_LIST(parameterListType) \
    namespace internal \
    { \
        struct RegisterShaderParameterList_##parameterListType \
        { \
            using ParameterListType = parameterListType; \
            \
            RegisterShaderParameterList_##parameterListType() \
            { \
                ShaderParameterListManager::AddDeferredInitializingParameterListInfos({ ParameterListType::GetName(), &ParameterListType::InitializeParameterListInfo }); \
            } \
        }; \
        static RegisterShaderParameterList_##parameterListType gRegisterShaderParameterList_##parameterListType; \
    }


    // ===== ShaderParameterListManager =====

    class ShaderParameterListManager
    {
    public:
        struct DeferredInitializingParameterListInfos
        {
            const Character* typeName;
            void (*initFunction)(const gapi::ShaderParameterHelper&, ShaderParameterListInfo&);
        };
        static void AddDeferredInitializingParameterListInfos(const DeferredInitializingParameterListInfos& initInfos);

        template <typename T>
            requires std::derived_from<T, ShaderParameterList>
        static const ShaderParameterListInfo& GetShaderParameterListInfo()
        {
            static int cachedIndex = -1;
            if (cachedIndex == -1)
            {
                cachedIndex = GetShaderParameterListInfoIndex(T::GetName());
                CHECK_FORMAT(cachedIndex != -1, "Uninitialized shader parameter list type! ({0})", T::GetName());
            }

            return mShaderParameterListInfos[cachedIndex];
        }
        static const ShaderParameterListInfo& GetShaderParameterListInfo(StringView parameterListTypeName)
        {
            return mShaderParameterListInfos[GetShaderParameterListInfoIndex(parameterListTypeName)];
        }

    private:
        static void ProcessDeferredInitializingParameterListInfos(const gapi::ShaderParameterHelper& shaderParemeterHelper);

        static int GetShaderParameterListInfoIndex(StringView parameterListTypeName);

        static constexpr int MAX_NUM_DEFERRED_INIT = 1024;
        static DeferredInitializingParameterListInfos mDeferredInitializingParameterListInfos[MAX_NUM_DEFERRED_INIT];
        static int mDeferredInitializingParameterListInfosIndex;
        static bool mIsDeferredInitOverflow;

        static Vector<ShaderParameterListInfo> mShaderParameterListInfos;
        static Map<String, int> mShaderParameterListTypeNameToIndexMap;

    public:
        ShaderParameterListManager() = default;
        ~ShaderParameterListManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        const gapi::ShaderParameterHelper& GetShaderParameterHelper() const { return *mShaderParameterHelper; }

        void MoveNextFrame();

        template <typename T>
            requires std::derived_from<T, ShaderParameterList>
        SharedPtr<T> CreateShaderParameterList()
        {
            SharedPtr<T> parameterList = std::make_shared<T>(*this);

            AllocateShaderParameterList(parameterList.get(), GetShaderParameterListInfo<T>());

            return parameterList;
        }

        bool ValidateShader(const gapi::ShaderReflection& shaderReflection);
        bool ValidateShaderParameterList(const ShaderParameterListInfo& parameterListInfo, const gapi::ShaderParameterBlockReflection& parameterBlockReflection);

    private:
        friend class ShaderParameterList;

        struct ShaderParameterListBufferPool;

        void AllocateShaderParameterList(ShaderParameterList* parameterList, const ShaderParameterListInfo& parameterListInfo);

        // TODO: Use whole buffer in each frame and suballocate?
        ShaderParameterListPooledBuffer AllocateBuffer(Uint32 size, StringView debugName);
        void FreeBuffer(ShaderParameterList& parameterList);

        GAPI* mGAPI;
        const gapi::ShaderParameterHelper* mShaderParameterHelper;
        Vector<Vector<gapi::ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;

        Uint32 mCurrentIndex;

        struct ShaderParameterListBufferPool
        {
            Vector<SharedPtr<gapi::Buffer>> buffers;
            Vector<SharedPtr<gapi::BufferSRV>> srvs;
            MultiMap<Uint32, Uint32> pooledBufferIndices;
            Vector<Uint32> freedBufferIndices;

            void CheckConsistency();

            void Clear()
            {
                CheckConsistency();

                srvs.clear();
                buffers.clear();
                pooledBufferIndices.clear();
                freedBufferIndices.clear();
            }
        };
        Vector<ShaderParameterListBufferPool> mBufferPools;
    };
} // namespace cube
