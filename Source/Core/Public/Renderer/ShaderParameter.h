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
    struct ShaderParameterTypeInfo<int>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::Int;
        static constexpr Uint32 size = sizeof(int);
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

    // ===== ShaderParameterList =====
    class ShaderParameterList;

    struct ShaderParameterInfo
    {
        const char* name;
        ShaderParameterType type;

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
        Uint32 poolIndex;
    };

    class ShaderParameterList
    {
    public:
        ShaderParameterList(const ShaderParameterList& other) = delete;
        ShaderParameterList& operator=(const ShaderParameterList& rhs) = delete;

        SharedPtr<gapi::Buffer> GetBuffer() const { return mPooledBuffer.buffer; }
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
            paramInfo.type = ShaderParameterTypeInfo<paramType>::type; \
            paramInfo.sizeInCPU = ShaderParameterTypeInfo<paramType>::size; \
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

        ShaderParameterListPooledBuffer AllocateBuffer(Uint32 size, StringView debugName);
        void FreeBuffer(ShaderParameterList& parameterList);

        GAPI* mGAPI;
        const gapi::ShaderParameterHelper* mShaderParameterHelper;
        Vector<Vector<gapi::ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;

        Uint32 mCurrentIndex;

        struct ShaderParameterListBufferPool
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
        Vector<ShaderParameterListBufferPool> mBufferPools;
    };
} // namespace cube
