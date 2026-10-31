#include "GAPI_DX12Shader.h"

#include "DX12Device.h"
#include "DX12Utility.h"
#include "SlangHelper.h"

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(DX12ShaderCompilerResult&& result, StringView warningMessage, StringView errorMessage, Vector<platform::FilePath>&& dependencyFilePaths)
            : mShaderByteCode(std::move(result.shader))
        {
            mCreated = mShaderByteCode.GetSize() > 0;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;
            mDependencyFilePaths = std::move(dependencyFilePaths);

            mReflection = std::move(result.reflection);
            mThreadGroupSizeX = mReflection.threadGroupSizeX;
            mThreadGroupSizeY = mReflection.threadGroupSizeY;
            mThreadGroupSizeZ = mReflection.threadGroupSizeZ;

            mContentHash = result.contentHash;
        }

        DX12Shader::~DX12Shader()
        {
            mShaderByteCode.Release();
        }

        DX12HitGroup::DX12HitGroup(const HitGroupCreateInfo& createInfo, DX12Device& device)
            : mDevice(device)
        {
            CHECK(createInfo.closestHitShader || createInfo.anyHitShader);

            FrameAllocationScope scope;

            FrameVector<D3D12_STATE_SUBOBJECT> subobjects;

            D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature;
            globalRootSignature.pGlobalRootSignature = device.GetShaderParameterHelper().GetRootSignature();
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
                .pDesc = &globalRootSignature,
            });

            auto AddShader = [&](Shader* shader, FrameWindowsString& shaderName, D3D12_EXPORT_DESC& exportDesc, D3D12_DXIL_LIBRARY_DESC& libraryDesc)
            {
                DX12Shader* dx12Shader = dynamic_cast<DX12Shader*>(shader);
                CHECK(dx12Shader);
                shaderName = String_Convert<FrameWindowsString>(dx12Shader->GetReflection().name);
                exportDesc.Name = shaderName.c_str();
                exportDesc.ExportToRename = nullptr;
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
                libraryDesc.DXILLibrary.pShaderBytecode = dx12Shader->GetShaderByteCode().GetData();
                libraryDesc.DXILLibrary.BytecodeLength = dx12Shader->GetShaderByteCode().GetSize();
                libraryDesc.NumExports = 1;
                libraryDesc.pExports = &exportDesc;

                subobjects.push_back({
                    .Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
                    .pDesc = &libraryDesc,
                });
            };


            D3D12_EXPORT_DESC closestHitExportDesc;
            D3D12_DXIL_LIBRARY_DESC closestHitLibraryDesc;
            FrameWindowsString closestHitShaderName;
            if (createInfo.closestHitShader)
            {
                AddShader(createInfo.closestHitShader.get(), closestHitShaderName, closestHitExportDesc, closestHitLibraryDesc);
                CUBE_DX12_BOUND_OBJECT(createInfo.closestHitShader);
            }

            D3D12_EXPORT_DESC anyHitExportDesc;
            D3D12_DXIL_LIBRARY_DESC anyHitLibraryDesc;
            FrameWindowsString anyHitShaderName;
            if (createInfo.anyHitShader)
            {
                AddShader(createInfo.anyHitShader.get(), anyHitShaderName, anyHitExportDesc, anyHitLibraryDesc);
                CUBE_DX12_BOUND_OBJECT(createInfo.anyHitShader);
            }

            D3D12_HIT_GROUP_DESC hitGroupDesc;
            FrameWindowsString hitGroupExportName;
            hitGroupExportName = Format<FrameWindowsString>(WINDOWS_T("{0}_HitGroup"), createInfo.debugName);
            hitGroupDesc.HitGroupExport = hitGroupExportName.c_str();
            hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
            hitGroupDesc.AnyHitShaderImport = createInfo.anyHitShader ? anyHitShaderName.c_str() : NULL;
            hitGroupDesc.ClosestHitShaderImport = createInfo.closestHitShader ? closestHitShaderName.c_str() : NULL;
            hitGroupDesc.IntersectionShaderImport = NULL;

            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
                .pDesc = &hitGroupDesc,
            });

            D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
            shaderConfig.MaxPayloadSizeInBytes = pipelineInfo.maxPayloadSize;
            shaderConfig.MaxAttributeSizeInBytes = pipelineInfo.maxAttributeSize;
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
                .pDesc = &shaderConfig,
                });

            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
            pipelineConfig.MaxTraceRecursionDepth = pipelineInfo.maxRecursionDepth;
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
                .pDesc = &pipelineConfig,
                });
        }

        DX12HitGroup::~DX12HitGroup()
        {
        }
    } // namespace gapi
} // namespace cube
