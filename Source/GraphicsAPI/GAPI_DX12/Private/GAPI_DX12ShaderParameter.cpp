#include "GAPI_DX12ShaderParameter.h"

#include "DX12Device.h"
#include "Allocator/AllocatorUtility.h"
#include "DX12ShaderCompiler.h"
#include "GAPI_Shader.h"
#include "GAPI_Texture.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/ShaderParameter.h"

namespace cube
{
    namespace gapi
    {
        DX12ShaderParameterHelper::DX12ShaderParameterHelper(DX12Device& device)
            : mDevice(device)
        {
            mMaxNumRegister = 4;
            mMaxNumSpace = 8;
        }

        DX12ShaderParameterHelper::~DX12ShaderParameterHelper()
        {
        }

        void DX12ShaderParameterHelper::Initialize()
        {
            FrameVector<D3D12_ROOT_PARAMETER1> parameters;

            parameters.reserve(mMaxNumRegister * mMaxNumSpace);
            for (Uint32 space = 0; space < mMaxNumSpace; ++space)
            {
                for (Uint32 reg = 0; reg < mMaxNumRegister; ++reg)
                {
                    parameters.push_back({
                        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                        .Descriptor = {
                            .ShaderRegister = reg,
                            .RegisterSpace = space,
                            .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC
                        },
                        .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
                    });
                }
            }

            auto SetStaticSampler = [](D3D12_STATIC_SAMPLER_DESC& outStaticSampler, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, UINT reg)
            {
                outStaticSampler = {
                    .Filter = filter,
                    .AddressU = addressMode,
                    .AddressV = addressMode,
                    .AddressW = addressMode,
                    .MipLODBias = 0.0f,
                    .MaxAnisotropy = 0,
                    .ComparisonFunc = D3D12_COMPARISON_FUNC_NONE,
                    .BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
                    .MinLOD = 0.0f,
                    .MaxLOD = D3D12_FLOAT32_MAX,
                    .ShaderRegister = reg,
                    .RegisterSpace = 0,
                    .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
                };
            };
            Array<D3D12_STATIC_SAMPLER_DESC, 4> staticSamplers;
            SetStaticSampler(staticSamplers[0], D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1000);
            SetStaticSampler(staticSamplers[1], D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 1001);
            SetStaticSampler(staticSamplers[2], D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1002);
            SetStaticSampler(staticSamplers[3], D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 1003);

            const D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {
                .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
                .Desc_1_1 = {
                    .NumParameters = static_cast<Uint32>(parameters.size()),
                    .pParameters = parameters.data(),
                    .NumStaticSamplers = static_cast<Uint32>(staticSamplers.size()),
                    .pStaticSamplers = staticSamplers.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                        | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
                        | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
                }
            };

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            CHECK_HR(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, mDevice.GetMaxRootSignatureVersion(), &signatureBlob, &errorBlob));
            CHECK_HR(mDevice.GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
            SET_DEBUG_NAME(mRootSignature, "GlobalRootSignature");

            InitializeCompatibleShaderParameterReflectionTypeMap();
        }

        void DX12ShaderParameterHelper::Shutdown()
        {
            mRootSignature = nullptr;
        }

        void DX12ShaderParameterHelper::UpdateShaderParameterListInfo(ShaderParameterListInfo& inOutParameterListInfo) const
        {
            Uint32 currentOffset = 0;
            Uint32 totalBufferSize = 0;

            for (ShaderParameterInfo& paramInfo : inOutParameterListInfo.parameterInfos)
            {
                Uint32 size = 0;
                Uint32 alignment = 1;

                switch (paramInfo.type)
                {
                case ShaderParameterType::Bool:
                    // Boolean is treated as 4 bytes in HLSL.
                    size = 4;
                    alignment = 4;
                    break;
                case ShaderParameterType::Int:
                    size = sizeof(Int32);
                    alignment = sizeof(Int32);
                    break;
                case ShaderParameterType::Float:
                    size = sizeof(float);
                    alignment = sizeof(float);
                    break;
                // Vector's alignment is the same as its element's alignment.
                case ShaderParameterType::Float2:
                    size = sizeof(Float2);
                    alignment = sizeof(float);
                    break;
                case ShaderParameterType::Float3:
                    size = sizeof(Float3);
                    alignment = sizeof(float);
                    break;
                case ShaderParameterType::Float4:
                    size = sizeof(Float4);
                    alignment = sizeof(float);
                    break;
                case ShaderParameterType::Matrix:
                    size = sizeof(Matrix);
                    // Matrix is treated as array of Float4 so it is aligned to 16.
                    alignment = 16;
                    break;
                case ShaderParameterType::BindlessTexture:
                case ShaderParameterType::BindlessSampler:
                case ShaderParameterType::BindlessCombinedTextureSampler:
                case ShaderParameterType::RGBufferSRV:
                case ShaderParameterType::RGBufferUAV:
                case ShaderParameterType::RGTextureSRV:
                case ShaderParameterType::RGTextureUAV:
                    // uint2
                    size = sizeof(Uint32) * 2;
                    alignment = sizeof(Uint32);
                    break;
                default:
                    NOT_IMPLEMENTED();
                    break;
                }

                Uint32 alignedOffset = Align(currentOffset, alignment);
                // If the data cross the 16 bytes boundary, it should be aligned to 16.
                if (alignedOffset / 16 != (alignedOffset + size - 1) / 16)
                {
                    alignedOffset = Align(alignedOffset, 16u);
                }

                paramInfo.sizeInGPU = size;
                paramInfo.offsetInGPU = alignedOffset;

                currentOffset = alignedOffset + size;
                totalBufferSize = std::max(totalBufferSize, currentOffset);
            }

            inOutParameterListInfo.totalBufferSize = totalBufferSize;
        }

        void DX12ShaderParameterHelper::WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const ShaderParameterListInfo& parameterListInfo, const void* pParameterList) const
        {
            Byte* bufferPtr = reinterpret_cast<Byte*>(buffer->Map());

            for (const ShaderParameterInfo& paramInfo : parameterListInfo.parameterInfos)
            {
                const Byte* src = reinterpret_cast<const Byte*>(pParameterList) + paramInfo.offsetInCPU;
                Byte* dst = bufferPtr + paramInfo.offsetInGPU;

                switch (paramInfo.type)
                {
                case ShaderParameterType::Bool:
                {
                    // Boolean is treated as 4 bytes in HLSL
                    CHECK_PARAMS(paramInfo.sizeInGPU == 4);
                    Uint32 value = *reinterpret_cast<const bool*>(src);
                    memcpy(dst, &value, sizeof(Uint32));
                    break;
                }
                case ShaderParameterType::Float2:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU == sizeof(Float2));
                    const Vector2* v2 = reinterpret_cast<const Vector2*>(src);
                    const Float2 f2 = v2->GetFloat2();
                    memcpy(dst, &f2, sizeof(Float2));
                    break;
                }
                case ShaderParameterType::Float3:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU == sizeof(Float3));
                    const Vector3* v3 = reinterpret_cast<const Vector3*>(src);
                    const Float3 f3 = v3->GetFloat3();
                    memcpy(dst, &f3, sizeof(Float3));
                    break;
                }
                case ShaderParameterType::Float4:
                {
                    CHECK_PARAMS(paramInfo.sizeInGPU == sizeof(Float4));
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
                    const BindlessTexture* data = reinterpret_cast<const BindlessTexture*>(src);
                    Uint32 uint2[2] = { static_cast<Uint32>(data->id), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::BindlessSampler:
                {
                    const BindlessSampler* data = reinterpret_cast<const BindlessSampler*>(src);
                    Uint32 uint2[2] = { static_cast<Uint32>(data->id), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::BindlessCombinedTextureSampler:
                {
                    const BindlessCombinedTextureSampler* data = reinterpret_cast<const BindlessCombinedTextureSampler*>(src);
                    Uint32 uint2[2] = { static_cast<Uint32>(data->textureId), static_cast<Uint32>(data->samplerId) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::RGBufferSRV:
                {
                    const RGBufferSRVHandle& srv = *reinterpret_cast<const RGBufferSRVHandle*>(src);
                    CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(srv->IsResourceCreated(), "RGBufferSRV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint32 uint2[2] = { static_cast<Uint32>(srv->GetSRV()->GetBindlessId()), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::RGBufferUAV:
                {
                    const RGBufferUAVHandle& uav = *reinterpret_cast<const RGBufferUAVHandle*>(src);
                    CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(uav->IsResourceCreated(), "RGBufferUAV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint32 uint2[2] = { static_cast<Uint32>(uav->GetUAV()->GetBindlessId()), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::RGTextureSRV:
                {
                    const RGTextureSRVHandle& srv = *reinterpret_cast<const RGTextureSRVHandle*>(src);
                    CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(srv->IsResourceCreated(), "RGTextureSRV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint32 uint2[2] = { static_cast<Uint32>(srv->GetSRV()->GetBindlessId()), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
                    break;
                }
                case ShaderParameterType::RGTextureUAV:
                {
                    const RGTextureUAVHandle& uav = *reinterpret_cast<const RGTextureUAVHandle*>(src);
                    CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", paramInfo.name);
                    CHECK_FORMAT(uav->IsResourceCreated(), "RGTextureUAV '{0}' is not created. Maybe call WriteAllParametersToGPUBuffer outside of RGBuilder?", paramInfo.name);

                    Uint32 uint2[2] = { static_cast<Uint32>(uav->GetUAV()->GetBindlessId()), static_cast<Uint32>(-1) };
                    memcpy(dst, uint2, sizeof(uint2));
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

        void DX12ShaderParameterHelper::InitializeCompatibleShaderParameterReflectionTypeMap()
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
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessTexture)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessSampler)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::BindlessCombinedTextureSampler)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            // RGBufferView and RGTextureView use bindless.
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGBufferSRV)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGBufferUAV)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGTextureSRV)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
            mCompatibleShaderParameterReflectionTypeMap[static_cast<int>(ShaderParameterType::RGTextureUAV)] = {
                ShaderParameterReflection::Type::BindlessHandler
            };
        }
    } // namespace gapi
} // namespace cube
