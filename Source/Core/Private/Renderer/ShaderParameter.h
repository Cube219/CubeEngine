#pragma once

#include "CoreHeader.h"

#include <functional>
#include <memory>

#include "Allocator/AllocatorUtility.h"
#include "GAPI_Buffer.h"
#include "Renderer/RenderTypes.h"

#define CUBE_LOG_PARAMETER_WRITING 0

namespace cube
{
    class GAPI;

    namespace gapi
    {
        class Buffer;
    } // namespace gapi

    class ShaderParametersManager;

    // ===== ParameterTypeInfo =====
    enum class ShaderParmeterType
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
    struct ParameterTypeInfo
    {
        static void WriteToBuffer(Byte* pBufferData, const T& data)
        {
            memcpy(pBufferData, &data, sizeof(T));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(T);
    };

    // Boolean is treated as 4 bytes in HLSL
    template <>
    struct ParameterTypeInfo<bool>
    {
        static void WriteToBuffer(Byte* pBufferData, const bool& data)
        {
            const Uint32 v = data;
            memcpy(pBufferData, &v, sizeof(Uint32));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(Uint32);
    };

    template <>
    struct ParameterTypeInfo<Vector2>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector2& data)
        {
            const Float2 f = data.GetFloat2();
            memcpy(pBufferData, &f, sizeof(Float2));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(Float2);
    };

    template <>
    struct ParameterTypeInfo<Vector3>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector3& data)
        {
            const Float3 f = data.GetFloat3();
            memcpy(pBufferData, &f, sizeof(Float3));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(Float3);
    };

    template <>
    struct ParameterTypeInfo<Vector4>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector4& data)
        {
            const Float4 f = data.GetFloat4();
            memcpy(pBufferData, &f, sizeof(Float4));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(Float4);
    };
    
    template <>
    struct ParameterTypeInfo<Matrix>
    {
        static void WriteToBuffer(Byte* pBufferData, const Matrix& data)
        {
            struct
            {
                Float4 r0;
                Float4 r1;
                Float4 r2;
                Float4 r3;
            } floatMatrix;
            floatMatrix.r0 = data.GetRow(0).GetFloat4();
            floatMatrix.r1 = data.GetRow(1).GetFloat4();
            floatMatrix.r2 = data.GetRow(2).GetFloat4();
            floatMatrix.r3 = data.GetRow(3).GetFloat4();

            memcpy(pBufferData, &floatMatrix, sizeof(floatMatrix));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(Matrix);
    };

    template <>
    struct ParameterTypeInfo<BindlessResource>
    {
        static void WriteToBuffer(Byte* pBufferData, const BindlessResource& data)
        {
            memcpy(pBufferData, &data, sizeof(BindlessResource));
        }

        static constexpr bool writtenInBuffer = true;
        static constexpr Uint32 size = sizeof(BindlessResource);
    };

    // ===== ShaderParameters =====
    class ShaderParameters;

    struct ShaderParameterInfo
    {
        const char* name;
        int offset;
        int size;
        bool writtenInBuffer;
    };

    template <typename TShaderParameters>
        requires std::is_base_of_v<ShaderParameters, TShaderParameters>
    struct ShaderParametersInfo
    {
        static inline bool isInitialized = true;

        static inline Vector<ShaderParameterInfo> parameterInfos;
        static inline Uint32 totalBufferSize;
    };

    struct ShaderParametersPooledBuffer
    {
        SharedPtr<gapi::Buffer> buffer;
        Uint32 poolIndex;
    };

    class alignas(256) ShaderParameters
    {
    public:
        ShaderParameters(const ShaderParameters& other) = delete;
        ShaderParameters& operator=(const ShaderParameters& rhs) = delete;

        SharedPtr<gapi::Buffer> GetBuffer() const { return mPooledBuffer.buffer; }

    // protected:
    //     template <typename T>
    //     static Uint32 CalculateOffset(Uint32 currentOffset)
    //     {
    //         using TTypeInfo = ParameterTypeInfo<T>;
    //
    //         Uint32 alignedOffset = Align(currentOffset, TTypeInfo::alignment);
    //         // If the data cross the 16 bytes boundary, it should be aligned to 16.
    //         if (alignedOffset / 16 != (alignedOffset + TTypeInfo::size - 1) / 16)
    //         {
    //             alignedOffset = Align(alignedOffset, 16u);
    //         }
    //
    //         return alignedOffset;
    //     }

    protected:
        // Only manager can create shader parameters
        friend class ShaderParametersManager;

        ShaderParameters(ShaderParametersManager& manager);
        virtual ~ShaderParameters();

        ShaderParametersManager& mManager;

        Uint32 mGPUSyncIndex;
        ShaderParametersPooledBuffer mPooledBuffer;
        Byte* mBufferPointer;
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
        /* Return next index */ \
        static int WriteToBuffer(const ParametersType& params, Byte* pBufferData) { return 0; } \
        static int InitializeParameterInfo(ParametersType& params, std::function<void(ShaderParameterInfo&)>& func) { return 0; } \
    }; \
    typedef ParameterIterHelperBegin

#define CUBE_SHADER_PARAMETER(type, varName) \
    ParameterIterHelper_##varName##_Prev; \
    \
    struct ParameterIterHelper_##varName \
    { \
        static int WriteToBuffer(const ParametersType& params, Byte* pBufferData) \
        { \
            int index = ParameterIterHelper_##varName##_Prev::WriteToBuffer(params, pBufferData); \
            \
            const ShaderParameterInfo& paramInfo = ShaderParametersInfo<ParametersType>::parameterInfos[index]; \
            CHECK_FORMAT(strcmp(paramInfo.name, #varName) == 0, "Mismatch shader parameter name! (Info: {0} / Variable: {1})", paramInfo.name, #varName); \
            CHECK_FORMAT(paramInfo.writtenInBuffer == ParameterTypeInfo<type>::writtenInBuffer, "Mismatch writtenInBuffer! (Info: {0} / Variable: {1})", paramInfo.writtenInBuffer, ParameterTypeInfo<type>::writtenInBuffer); \
            if (paramInfo.writtenInBuffer) \
            { \
                if constexpr (CUBE_LOG_PARAMETER_WRITING) \
                { \
                    CUBE_LOG(Info, Renderer, "Parameter {0} - offset: {1} / size: {2}", CUBE_T(#varName), paramInfo.offset, paramInfo.size); \
                } \
                /* TODO: Override write to buffer in GAPI? */ \
                CHECK_FORMAT(paramInfo.size == ParameterTypeInfo<type>::size, "Mismatch size! (Info: {0} / Variable: {1})", paramInfo.size, ParameterTypeInfo<type>::size); \
                \
                ParameterTypeInfo<type>::WriteToBuffer(pBufferData + paramInfo.offset, params.varName); \
            } \
            \
            return index + 1; \
        } \
        static int InitializeParameterInfo(ParametersType& params, std::function<void(ShaderParameterInfo&)>& func) \
        { \
            int index = ParameterIterHelper_##varName##_Prev::InitializeParameterInfo(params, func); \
            \
            ShaderParametersInfo<ParametersType>::parameterInfos.emplace_back(); \
            auto& paramInfo = ShaderParametersInfo<ParametersType>::parameterInfos.back(); \
            \
            paramInfo.name = #varName; \
            paramInfo.writtenInBuffer = ParameterTypeInfo<type>::writtenInBuffer; \
            func(paramInfo); \
            \
            ShaderParametersInfo<ParametersType>::totalBufferSize = std::max(ShaderParametersInfo<ParametersType>::totalBufferSize, (Uint32)paramInfo.offset + paramInfo.size); \
            \
            return index + 1; \
        } \
    }; \
    \
public: \
    type varName; \
    \
private: \
    typedef ParameterIterHelper_##varName

#define CUBE_END_SHADER_PARAMETERS \
    ParameterIterHelperEnd_Prev; \
    \
    struct ParameterIterHelperEnd \
    { \
        static Uint32 WriteToBuffer(const ParametersType& params, Byte* pBufferData) \
        { \
            return ParameterIterHelperEnd_Prev::WriteToBuffer(params, pBufferData); \
        } \
        static void InitializeParameterInfo(ParametersType& params, std::function<void(ShaderParameterInfo&)>& func) \
        { \
            ParameterIterHelperEnd_Prev::InitializeParameterInfo(params, func); \
            \
            ShaderParametersInfo<ParametersType>::isInitialized = true; \
        } \
    }; \
    \
public: \
    void WriteAllParametersToBuffer() \
    { \
        ParameterIterHelperEnd::WriteToBuffer(*this, mBufferPointer); \
    } \
    void InitializeParameterInfo(ParametersType& params, std::function<void(ShaderParameterInfo&)>& func) \
    { \
        ParameterIterHelperEnd::InitializeParameterInfo(params, func); \
    }

    // ===== ShaderParametersManager =====

    class ShaderParametersManager
    {
    public:
        ShaderParametersManager() = default;
        ~ShaderParametersManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        void MoveNextFrame();

        template <typename T>
            requires std::derived_from<T, ShaderParameters>
        SharedPtr<T> CreateShaderParameters()
        {
            SharedPtr<T> parameters = std::make_shared<T>(*this);
            InitializeShaderParameters(parameters.get(), ShaderParametersInfo<T>::totalBufferSize, T::GetDebugName());

            return parameters;
        }

    private:
        friend class ShaderParameters;

        struct ShaderParametersBufferPool;

        void InitializeShaderParameters(ShaderParameters* parameters, Uint32 bufferSize, StringView debugName);

        ShaderParametersPooledBuffer AllocateBuffer(Uint32 size, StringView debugName);
        void FreeBuffer(ShaderParameters& parameters);

        GAPI* mGAPI;

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
