#pragma once

#include "CoreHeader.h"
#include "Allocator/AllocatorUtility.h"
#include "Renderer/RenderTypes.h"

namespace cube
{
    class GAPI;

    namespace gapi
    {
        class Buffer;
    } // namespace gapi

    template <typename T>
    struct ParameterTypeInfo
    {
        static void WriteToBuffer(Byte* pBufferData, const T& data)
        {
            memcpy(pBufferData, &data, sizeof(T));
        }

        constexpr Uint32 size = sizeof(T);
        constexpr Uint32 alignment = std::max(sizeof(T), 4);
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

        constexpr Uint32 size = sizeof(Uint32);
        constexpr Uint32 alignment = sizeof(Uint32);
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

        constexpr Uint32 size = sizeof(Float2);
        constexpr Uint32 alignment = sizeof(float);
    };

    template <>
    struct ParameterTypeInfo<Vector3>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector3& data)
        {
            const Float3 f = data.GetFloat3();
            memcpy(pBufferData, &f, sizeof(Float3));
        }

        constexpr Uint32 size = sizeof(Float3);
        constexpr Uint32 alignment = sizeof(float);
    };

    template <>
    struct ParameterTypeInfo<Vector4>
    {
        static void WriteToBuffer(Byte* pBufferData, const Vector4& data)
        {
            const Float4 f = data.GetFloat4();
            memcpy(pBufferData, &f, sizeof(Float4));
        }

        constexpr Uint32 size = sizeof(Float4);
        constexpr Uint32 alignment = sizeof(float);
    };

    template <>
    struct ParameterTypeInfo<BindlessResource>
    {
        static void WriteToBuffer(Byte* pBufferData, const BindlessResource& data)
        {
        }

        constexpr Uint32 size = sizeof(BindlessResource);
        constexpr Uint32 alignment = sizeof(BindlessResource);
    };

#define CUBE_BEGIN_SHADER_PARAMETERS

#define CUBE_SHADER_PARAMETER(type, name)

    typedef asdf ParameterInfo_name_Prev;

    struct ParameterInfo_name
    {
        static Uint32 CalculateOffset(Uint32 currentOffset)
        {
            using MyTypeInfo = ParameterTypeInfo<type>;

            Uint32 alignedOffset = Align(currentOffset, MyTypeInfo::alignment);

            // If the data cross the 16 bytes boundary, it should be aligned to 16.
            if (alignedOffset / 16 != (alignedOffset + MyTypeInfo::size) / 16)
            {
                alignedOffset = Align(alignedOffset, 16);
            }

            return alignedOffset;
        }

        static Uint32 WriteToBuffer(Byte* pBufferData)
        {
            Uint32 offset = ParameterInfo_name_Prev::WriteToBuffer(pBufferData);

            using MyTypeInfo = ParameterTypeInfo<type>;
            offset = CalculateOffset(offset);
            MyTypeInfo::WriteToBuffer(pBufferData + offset, name);

            return offset + MyTypeInfo::size;
        }
    };

#define CUBE_END_SHADER_PARAMETERS

    class ShaderParametersManager;

    class ShaderParameters
    {
    private:
        friend class ShaderParametersManager;

        ShaderParameters()
        {
        }
        virtual ~ShaderParameters()
        {
            
        }

        SharedPtr<gapi::Buffer> mBuffer;
        Byte* mBufferPointer;

        Uint32 mBufferSize;
    };

    class ShaderParametersManager
    {
    public:
        ShaderParametersManager() = default;
        ~ShaderParametersManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

    private:
        Uint32 mCurrentIndex;
    };
} // namespace cube
