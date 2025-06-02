#pragma once

#include "DX12Header.h"

#include "GAPI_Pipeline.h"

#include "DX12APIObject.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Pipeline : public Pipeline, public DX12APIObject
        {
        public:
            DX12Pipeline(DX12Device& device, const GraphicsPipelineCreateInfo& info);
            virtual ~DX12Pipeline();

            ID3D12PipelineState* GetPipelineState() const { return mPipelineState.Get(); }

        private:
            ComPtr<ID3D12PipelineState> mPipelineState;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };
    } // namespace gapi
} // namespace cube
