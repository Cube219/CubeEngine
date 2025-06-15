#pragma once

#include "DX12Header.h"

#include "DX12APIObject.h"
#include "DX12DescriptorManager.h"
#include "GAPI_Sampler.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Sampler : public Sampler, public DX12APIObject
        {
        public:
            DX12Sampler(DX12Device& device, const SamplerCreateInfo& info);
            virtual ~DX12Sampler();

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mDescriptor;
        };
    } // namespace gapi
}// namespace cube
