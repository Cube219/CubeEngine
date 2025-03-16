#pragma once

#include "DX12Header.h"

#include "GAPI_Pipeline.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Pipeline : public Pipeline
        {
        public:
            DX12Pipeline(DX12Device& device, const GraphicsPipelineCreateInfo& info);
            virtual ~DX12Pipeline();

            ID3D12PipelineState* GetPipelineState() const { return mPipelineState.Get(); }

        private:
            // Referenced objects
            SharedPtr<Shader> mVertexShader;
            SharedPtr<Shader> mPixelShader;
            SharedPtr<ShaderVariablesLayout> mShaderVariablesLayout;

            ComPtr<ID3D12PipelineState> mPipelineState;
        };
    } // namespace gapi
} // namespace cube
