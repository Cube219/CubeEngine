#pragma once

#include "DX12Header.h"

#include "GAPI_Shader.h"

#include "DX12APIObject.h"
#include "DX12ShaderCompiler.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Shader : public Shader, public DX12APIObject
        {
        public:
            DX12Shader(DX12ShaderCompilerResult&& result, StringView warningMessage, StringView errorMessage, Vector<platform::FilePath>&& dependencyFilePaths);
            virtual ~DX12Shader();

            BlobView GetShaderByteCode() const { return mShaderByteCode; }

        private:
            Blob mShaderByteCode;

            Blob mClosestHitShaderByteCode;
            Blob mAnyHitShaderByteCode;
        };

        class DX12HitGroup : public HitGroup, public DX12APIObject
        {
        public:
            DX12HitGroup(const HitGroupCreateInfo& createInfo, DX12Device& device);
            virtual ~DX12HitGroup();

        private:
            DX12Device& mDevice;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };
    } // namespace gapi
} // namespace cube
