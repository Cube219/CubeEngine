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

        void MetalShaderParameterHelper::UpdateShaderParameterListInfo(ShaderParameterListInfo& inOutParameterListInfo) const
        {
            Uint32 currentOffset = 0;
            Uint32 totalBufferSize = 0;

            for (ShaderParameterInfo& paramInfo : inOutParameterListInfo.parameterInfos)
            {
                Uint32 sizeInGPU = 0;
                Uint32 alignment = 1;

                switch (paramInfo.type)
                {
                case ShaderParameterCPUType::Bool:
                    sizeInGPU = 1;
                    alignment = 1;
                    break;
                // Non-packed vector size is power of 2 in Metal.
                case ShaderParameterCPUType::Int:
                case ShaderParameterCPUType::Uint:
                case ShaderParameterCPUType::Float:
                    sizeInGPU = 4;
                    alignment = 4;
                    break;
                case ShaderParameterCPUType::Int2:
                case ShaderParameterCPUType::Uint2:
                case ShaderParameterCPUType::Float2:
                case ShaderParameterCPUType::Vector2:
                    sizeInGPU = 8;
                    alignment = 8;
                    break;
                case ShaderParameterCPUType::Int3:
                case ShaderParameterCPUType::Int4:
                case ShaderParameterCPUType::Uint3:
                case ShaderParameterCPUType::Uint4:
                case ShaderParameterCPUType::Float3:
                case ShaderParameterCPUType::Float4:
                case ShaderParameterCPUType::Vector3:
                case ShaderParameterCPUType::Vector4:
                    sizeInGPU = 16;
                    alignment = 16;
                    break;
                case ShaderParameterCPUType::Matrix:
                    sizeInGPU = 64;
                    // Matrix is treated as array of Float4 so it is aligned to 16.
                    alignment = 16;
                    break;
                case ShaderParameterCPUType::BindlessTexture:
                case ShaderParameterCPUType::BindlessSampler:
                case ShaderParameterCPUType::RGBufferSRV:
                case ShaderParameterCPUType::RGBufferUAV:
                case ShaderParameterCPUType::RGTextureSRV:
                case ShaderParameterCPUType::RGTextureUAV:
                    sizeInGPU = sizeof(Uint64);
                    alignment = sizeof(Uint64);
                    break;
                case ShaderParameterCPUType::BindlessCombinedTextureSampler:
                    sizeInGPU = sizeof(Uint64) * 2;
                    alignment = sizeof(Uint64);
                    break;
                default:
                    NOT_IMPLEMENTED();
                    break;
                }

                Uint32 alignedOffset = Align(currentOffset, alignment);

                paramInfo.sizeInGPU = sizeInGPU;
                paramInfo.offsetInGPU = alignedOffset;

                currentOffset = alignedOffset + sizeInGPU;
                totalBufferSize = std::max(totalBufferSize, currentOffset);
            }

            inOutParameterListInfo.totalBufferSize = totalBufferSize;
        }

        void MetalShaderParameterHelper::WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const ShaderParameterListInfo& parameterListInfo, const void* pParameterList) const
        {
            Byte* bufferPtr = reinterpret_cast<Byte*>(buffer->Map());

            for (const ShaderParameterInfo& paramInfo : parameterListInfo.parameterInfos)
            {
                const Byte* src = reinterpret_cast<const Byte*>(pParameterList) + paramInfo.offsetInCPU;
                Byte* dst = bufferPtr + paramInfo.offsetInGPU;

                switch (paramInfo.type)
                {
                case ShaderParameterCPUType::Bool:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(bool));
                    bool value = *reinterpret_cast<const bool*>(src);
                    memcpy(dst, &value, sizeof(Uint32));
                    break;
                }
                case ShaderParameterCPUType::Int:
                case ShaderParameterCPUType::Uint:
                case ShaderParameterCPUType::Float:
                {
                    CHECK_PARAMS(paramInfo.sizeInCPU == paramInfo.sizeInGPU);
                    memcpy(dst, src, paramInfo.sizeInCPU);
                    break;
                }
                case ShaderParameterCPUType::Int2:
                case ShaderParameterCPUType::Int3:
                case ShaderParameterCPUType::Int4:
                case ShaderParameterCPUType::Uint2:
                case ShaderParameterCPUType::Uint3:
                case ShaderParameterCPUType::Uint4:
                case ShaderParameterCPUType::Float2:
                case ShaderParameterCPUType::Float3:
                case ShaderParameterCPUType::Float4:
                {
                    // Non-packed vector slot in Metal is power-of-2 sized, so sizeInGPU can be larger than sizeInCPU (e.g. Int3/Float3: 12 vs 16).
                    CHECK_PARAMS(paramInfo.sizeInGPU >= paramInfo.sizeInCPU);
                    memcpy(dst, src, paramInfo.sizeInCPU);
                    break;
                }
                case ShaderParameterCPUType::Vector2:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float2));
                    const Vector2* v2 = reinterpret_cast<const Vector2*>(src);
                    const Float2 f2 = v2->GetFloat2();
                    memcpy(dst, &f2, sizeof(Float2));
                    break;
                }
                case ShaderParameterCPUType::Vector3:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float3));
                    const Vector3* v3 = reinterpret_cast<const Vector3*>(src);
                    const Float3 f3 = v3->GetFloat3();
                    memcpy(dst, &f3, sizeof(Float3));
                    break;
                }
                case ShaderParameterCPUType::Vector4:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU >= sizeof(Float4));
                    const Vector4* v4 = reinterpret_cast<const Vector4*>(src);
                    const Float4 f4 = v4->GetFloat4();
                    memcpy(dst, &f4, sizeof(Float4));
                    break;
                }
                case ShaderParameterCPUType::Matrix:
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
                case ShaderParameterCPUType::BindlessTexture:
                {
                    const BindlessTexture* bindlessTexture = reinterpret_cast<const BindlessTexture*>(src);
                    Uint64 id = bindlessTexture->id;
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterCPUType::BindlessSampler:
                {
                    const BindlessSampler* bindlessSampler = reinterpret_cast<const BindlessSampler*>(src);
                    Uint64 id = bindlessSampler->id;
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterCPUType::BindlessCombinedTextureSampler:
                {
                    const BindlessCombinedTextureSampler* bindlessCombinedTextureSampler = reinterpret_cast<const BindlessCombinedTextureSampler*>(src);
                    Uint64 ids[2];
                    ids[0] = bindlessCombinedTextureSampler->textureId;
                    ids[1] = bindlessCombinedTextureSampler->samplerId;
                    memcpy(dst, ids, sizeof(ids));
                    break;
                }
                case ShaderParameterCPUType::RGBufferSRV:
                {
                    const RGBufferSRVHandle& srv = *reinterpret_cast<const RGBufferSRVHandle*>(src);
                    CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(srv->IsResourceCreated(), "RGBufferSRV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint64 id = srv->GetSRV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterCPUType::RGBufferUAV:
                {
                    const RGBufferUAVHandle& uav = *reinterpret_cast<const RGBufferUAVHandle*>(src);
                    CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(uav->IsResourceCreated(), "RGBufferUAV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint64 id = uav->GetUAV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterCPUType::RGTextureSRV:
                {
                    const RGTextureSRVHandle& srv = *reinterpret_cast<const RGTextureSRVHandle*>(src);
                    CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(srv->IsResourceCreated(), "RGTextureSRV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint64 id = srv->GetSRV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
                    break;
                }
                case ShaderParameterCPUType::RGTextureUAV:
                {
                    const RGTextureUAVHandle& uav = *reinterpret_cast<const RGTextureUAVHandle*>(src);
                    CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(uav->IsResourceCreated(), "RGTextureUAV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint64 id = uav->GetUAV()->GetBindlessId();
                    memcpy(dst, &id, sizeof(Uint64));
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
            mCompatibleShaderParameterReflectionTypeMap.resize(static_cast<int>(ShaderParameterCPUType::Num));

            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Bool)] = {
                ShaderParameterReflection::Type::Bool
            };
            // Int and Uint are bidirectionally compatible.
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Int)] = {
                ShaderParameterReflection::Type::Int,
                ShaderParameterReflection::Type::Uint
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Int2)] = {
                ShaderParameterReflection::Type::Int2,
                ShaderParameterReflection::Type::Uint2
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Int3)] = {
                ShaderParameterReflection::Type::Int3,
                ShaderParameterReflection::Type::Uint3
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Int4)] = {
                ShaderParameterReflection::Type::Int4,
                ShaderParameterReflection::Type::Uint4
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Uint)] = {
                ShaderParameterReflection::Type::Int,
                ShaderParameterReflection::Type::Uint
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Uint2)] = {
                ShaderParameterReflection::Type::Int2,
                ShaderParameterReflection::Type::Uint2
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Uint3)] = {
                ShaderParameterReflection::Type::Int3,
                ShaderParameterReflection::Type::Uint3
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Uint4)] = {
                ShaderParameterReflection::Type::Int4,
                ShaderParameterReflection::Type::Uint4
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Float)] = {
                ShaderParameterReflection::Type::Float
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Float2)] = {
                ShaderParameterReflection::Type::Float2
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Float3)] = {
                ShaderParameterReflection::Type::Float3
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Float4)] = {
                ShaderParameterReflection::Type::Float4
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Vector2)] = {
                ShaderParameterReflection::Type::Float2
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Vector3)] = {
                ShaderParameterReflection::Type::Float3
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Vector4)] = {
                ShaderParameterReflection::Type::Float4
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::Matrix)] = {
                ShaderParameterReflection::Type::Matrix
            };
            // Metal use raw resource type for bindless.
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::BindlessTexture)] = {
                ShaderParameterReflection::Type::Texture
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::BindlessSampler)] = {
                ShaderParameterReflection::Type::Sampler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::BindlessCombinedTextureSampler)] = {
                ShaderParameterReflection::Type::CombinedTextureSampler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::RGBufferSRV)] = {
                ShaderParameterReflection::Type::Buffer
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::RGBufferUAV)] = {
                ShaderParameterReflection::Type::Buffer
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::RGTextureSRV)] = {
                ShaderParameterReflection::Type::Texture
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterCPUType::RGTextureUAV)] = {
                ShaderParameterReflection::Type::Texture
            };
        }
    } // namespace gapi
} // namespace cube

