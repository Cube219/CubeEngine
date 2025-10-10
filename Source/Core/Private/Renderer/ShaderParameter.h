#pragma once

#include "CoreHeader.h"

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

    template <typename T>
    struct ParameterTypeInfo
    {
        static void WriteToBuffer(Byte* pBufferData, const T& data)
        {
            memcpy(pBufferData, &data, sizeof(T));
        }

        static constexpr Uint32 size = sizeof(T);
        static constexpr Uint32 alignment = std::max((Uint32)sizeof(T), 4u);
    };

    // Boolean is treated as 4 bytes in HLSL
    // TODO: Check the other shader languages
    template <>
    struct ParameterTypeInfo<bool>
    {
        static void WriteToBuffer(Byte* pBufferData, const bool& data)
        {
            const Uint32 v = data;
            memcpy(pBufferData, &v, sizeof(Uint32));
        }

        static constexpr Uint32 size = sizeof(Uint32);
        static constexpr Uint32 alignment = sizeof(Uint32);
    };

    // Vector's alignment is the same as its element's alignment;
    // TODO: Check the other shader languages
    template <>
    struct ParameterTypeInfo<Vector2>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector2& data)
        {
            const Float2 f = data.GetFloat2();
            memcpy(pBufferData, &f, sizeof(Float2));
        }

        static constexpr Uint32 size = sizeof(Float2);
        static constexpr Uint32 alignment = sizeof(float);
    };

    template <>
    struct ParameterTypeInfo<Vector3>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector3& data)
        {
            const Float3 f = data.GetFloat3();
            memcpy(pBufferData, &f, sizeof(Float3));
        }

        static constexpr Uint32 size = sizeof(Float3);
        static constexpr Uint32 alignment = sizeof(float);
    };

    template <>
    struct ParameterTypeInfo<Vector4>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector4& data)
        {
            const Float4 f = data.GetFloat4();
            memcpy(pBufferData, &f, sizeof(Float4));
        }

        static constexpr Uint32 size = sizeof(Float4);
        static constexpr Uint32 alignment = sizeof(float);
    };
    
    template <>
    struct ParameterTypeInfo<Matrix>
    {
        static void WriteToBuffer(Byte* pBufferData, const Matrix& data)
        {
            memcpy(pBufferData, &data, sizeof(Matrix));
        }

        static constexpr Uint32 size = sizeof(Matrix);
        static constexpr Uint32 alignment = sizeof(Vector4);
    };

    template <>
    struct ParameterTypeInfo<BindlessResource>
    {
        static void WriteToBuffer(Byte* pBufferData, const BindlessResource& data)
        {
            memcpy(pBufferData, &data, sizeof(BindlessResource));
        }

        static constexpr Uint32 size = sizeof(BindlessResource);
        static constexpr Uint32 alignment = sizeof(int);
    };

    // ===== ShaderParameters =====

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

    protected:
        template <typename T>
        static Uint32 CalculateOffset(Uint32 currentOffset)
        {
            using TTypeInfo = ParameterTypeInfo<T>;

            Uint32 alignedOffset = Align(currentOffset, TTypeInfo::alignment);
            // If the data cross the 16 bytes boundary, it should be aligned to 16.
            if (alignedOffset / 16 != (alignedOffset + TTypeInfo::size - 1) / 16)
            {
                alignedOffset = Align(alignedOffset, 16u);
            }

            return alignedOffset;
        }

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
    struct ParameterInfoBegin \
    { \
        static Uint32 WriteToBuffer(const ParametersType& params, Byte* pBufferData) { return 0; } \
        static Uint32 GetTotalSize(const ParametersType& params) { return 0; } \
    }; \
    typedef ParameterInfoBegin

#define CUBE_SHADER_PARAMETER(type, name) \
    ParameterInfo_##name##_Prev; \
    \
    struct ParameterInfo_##name \
    { \
        static Uint32 WriteToBuffer(const ParametersType& params, Byte* pBufferData) \
        { \
            Uint32 offset = ParameterInfo_##name##_Prev::WriteToBuffer(params, pBufferData); \
            \
            Uint32 alignedOffset = ShaderParameters::CalculateOffset<type>(offset); \
            if constexpr (CUBE_LOG_PARAMETER_WRITING) \
            { \
                CUBE_LOG(Info, Renderer, "Parameter {0} - offset: {1} / align: {2} / size: {3}", CUBE_T(#name), offset, alignedOffset, ParameterTypeInfo<type>::size); \
            } \
            ParameterTypeInfo<type>::WriteToBuffer(pBufferData + alignedOffset, params.name); \
            \
            return alignedOffset + ParameterTypeInfo<type>::size; \
        } \
        static Uint32 GetTotalSize(const ParametersType& params) \
        { \
            Uint32 offset = ParameterInfo_##name##_Prev::GetTotalSize(params); \
            return offset + ParameterTypeInfo<type>::size; \
        } \
    }; \
    public: \
    type name; \
    private: \
    typedef ParameterInfo_##name

#define CUBE_END_SHADER_PARAMETERS \
    ParameterInfoEnd_Prev; \
    \
    struct ParameterInfoEnd \
    { \
        static Uint32 WriteToBuffer(const ParametersType& params, Byte* pBufferData) \
        { \
            return ParameterInfoEnd_Prev::WriteToBuffer(params, pBufferData); \
        } \
        static Uint32 GetTotalSize(const ParametersType& params) \
        { \
            return ParameterInfoEnd_Prev::GetTotalSize(params); \
        } \
    }; \
    \
    public: \
    void WriteAllParametersToBuffer() \
    { \
        Uint32 endOffset = ParameterInfoEnd::WriteToBuffer(*this, mBufferPointer); \
        CHECK(endOffset <= mPooledBuffer.buffer->GetSize()); \
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
            InitializeShaderParameters(parameters.get(), sizeof(T), T::GetDebugName());

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
