#include "GAPI_MetalShaderParameter.h"

#include "Allocator/AllocatorUtility.h"
#include "GAPI_Texture.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/ShaderParameter.h"

namespace cube
{
    namespace gapi
    {
        MetalShaderParameterHelper::MetalShaderParameterHelper()
        {
        }

        MetalShaderParameterHelper::~MetalShaderParameterHelper()
        {
        }

        void MetalShaderParameterHelper::Initialize()
        {
            InitializeCompatibleShaderParameterReflectionTypeMap();
        }

        void MetalShaderParameterHelper::Shutdown()
        {
        }

        void MetalShaderParameterHelper::UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const
        {
            Uint32 currentOffset = 0;
            Uint32 totalBufferSize = 0;

            for (ShaderParameterInfo& paramInfo : inOutParameterInfos)
            {
                Uint32 size = 0;
                Uint32 alignment = 1;

                switch (paramInfo.type)
                {
                case ShaderParameterType::Bool:
                    size = 1;
                    alignment = 1;
                    break;
                case ShaderParameterType::Int:
                    size = sizeof(Int32);
                    alignment = sizeof(Int32);
                    break;
                case ShaderParameterType::Float:
                    size = sizeof(float);
                    alignment = sizeof(float);
                    break;
                // Non-packed vector size is power of 2 in Metal.
                case ShaderParameterType::Float2:
                    size = 8;
                    alignment = 8;
                    break;
                case ShaderParameterType::Float3:
                case ShaderParameterType::Float4:
                    size = 16;
                    alignment = 16;
                    break;
                case ShaderParameterType::Matrix:
                    size = 64;
                    // Matrix is treated as array of Float4 so it is aligned to 16.
                    alignment = 16;
                    break;
                case ShaderParameterType::BindlessTexture:
                case ShaderParameterType::BindlessSampler:
                case ShaderParameterType::RGTextureSRV:
                case ShaderParameterType::RGTextureUAV:
                    size = sizeof(Uint64);
                    alignment = sizeof(Uint64);
                    break;
                case ShaderParameterType::BindlessCombinedTextureSampler:
                    size = sizeof(Uint64) * 2;
                    alignment = sizeof(Uint64);
                    break;
                default:
                    NOT_IMPLEMENTED();
                    break;
                }

                Uint32 alignedOffset = Align(currentOffset, alignment);

                paramInfo.sizeInGPU = size;
                paramInfo.offsetInGPU = alignedOffset;

                currentOffset = alignedOffset + size;
                totalBufferSize = std::max(totalBufferSize, currentOffset);
            }

            outTotalBufferSize = totalBufferSize;
        }

        void MetalShaderParameterHelper::WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const
        {
            Byte* bufferPtr = reinterpret_cast<Byte*>(buffer->Map());

            for (const ShaderParameterInfo& paramInfo : paramInfos)
            {
                const Byte* src = reinterpret_cast<const Byte*>(pParameters) + paramInfo.offsetInCPU;
                Byte* dst = bufferPtr + paramInfo.offsetInGPU;

                switch (paramInfo.type)
                {
                case ShaderParameterType::Bool:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(bool));
                    bool value = *reinterpret_cast<const bool*>(src);
                    memcpy(dst, &value, sizeof(Uint32));
                    break;
                }
                case ShaderParameterType::Float2:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float2));
                    const Vector2* v2 = reinterpret_cast<const Vector2*>(src);
                    const Float2 f2 = v2->GetFloat2();
                    memcpy(dst, &f2, sizeof(Float2));
                    break;
                }
                case ShaderParameterType::Float3:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float3));
                    const Vector3* v3 = reinterpret_cast<const Vector3*>(src);
                    const Float3 f3 = v3->GetFloat3();
                    memcpy(dst, &f3, sizeof(Float3));
                    break;
                }
                case ShaderParameterType::Float4:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float4));
                    const Vector4* v4 = reinterpret_cast<const Vector4*>(src);
                    const Float4 f4 = v4->GetFloat4();
                    memcpy(dst, &f4, sizeof(Float4));
                    break;
                }
                case ShaderParameterType::Matrix:
                {
                    const Matrix* data = reinterpret_cast<const Matrix*>(src);
                    struct
                    {
                        Float4 r0;
                        Float4 r1;
                        Float4 r2;
                        Float4 r3;
                    } floatMatrix;
                    floatMatrix.r0 = data->GetRow(0).GetFloat4();
                    floatMatrix.r1 = data->GetRow(1).GetFloat4();
                    floatMatrix.r2 = data->GetRow(2).GetFloat4();
                    floatMatrix.r3 = data->GetRow(3).GetFloat4();
                    memcpy(dst, &floatMatrix, sizeof(floatMatrix));
                    break;
                }
                case ShaderParameterType::BindlessTexture:
                {
                    const BindlessTexture* bindlessTexture = reinterpret_cast<const BindlessTexture*>(src);
                    Uint64 id = bindlessTexture->id;
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterType::BindlessSampler:
                {
                    const BindlessSampler* bindlessSampler = reinterpret_cast<const BindlessSampler*>(src);
                    Uint64 id = bindlessSampler->id;
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterType::BindlessCombinedTextureSampler:
                {
                    const BindlessCombinedTextureSampler* bindlessCombinedTextureSampler = reinterpret_cast<const BindlessCombinedTextureSampler*>(src);
                    Uint64 ids[2];
                    ids[0] = bindlessCombinedTextureSampler->textureId;
                    ids[1] = bindlessCombinedTextureSampler->samplerId;
                    memcpy(dst, ids, sizeof(ids));
                    break;
                }
                case ShaderParameterType::RGTextureSRV:
                {
                    const RGTextureSRVHandle& srv = *reinterpret_cast<const RGTextureSRVHandle*>(src);
                    CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}.", paramInfo.name);

                    Uint64 id = srv->GetSRV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterType::RGTextureUAV:
                {
                    const RGTextureUAVHandle& uav = *reinterpret_cast<const RGTextureUAVHandle*>(src);
                    CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}.", paramInfo.name);

                    Uint64 id = uav->GetUAV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterType::Int:
                case ShaderParameterType::Float:
                {
                    CHECK_PARAMS(paramInfo.sizeInCPU == paramInfo.sizeInGPU);
                    memcpy(dst, src, paramInfo.sizeInCPU);
                    break;
                }
                default:
                    NOT_IMPLEMENTED();
                    break;
                }
            }

            buffer->Unmap();
        }

        void MetalShaderParameterHelper::InitializeCompatibleShaderParameterReflectionTypeMap()
        {
            mCompatibleShaderParameterReflectionTypeMap.resize(static_cast<int>(ShaderParameterType::Num));

            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Bool)] = {
                ShaderParameterReflection::Type::Bool
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Int)] = {
                ShaderParameterReflection::Type::Int
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Float)] = {
                ShaderParameterReflection::Type::Float
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Float2)] = {
                ShaderParameterReflection::Type::Float2
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Float3)] = {
                ShaderParameterReflection::Type::Float3
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Float4)] = {
                ShaderParameterReflection::Type::Float4
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::Matrix)] = {
                ShaderParameterReflection::Type::Matrix
            };
            // Metal use raw resource type for bindless.
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessTexture)] = {
                ShaderParameterReflection::Type::Texture
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessSampler)] = {
                ShaderParameterReflection::Type::Sampler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessCombinedTextureSampler)] = {
                ShaderParameterReflection::Type::CombinedTextureSampler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGTextureSRV)] = {
                ShaderParameterReflection::Type::Texture
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGTextureUAV)] = {
                ShaderParameterReflection::Type::Texture
            };
        }
    } // namespace gapi
} // namespace cube

