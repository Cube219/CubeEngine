#pragma once

#include "MetalHeader.h"

#include "GAPI_Sampler.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalSampler : public Sampler
        {
        public:
            MetalSampler(const SamplerCreateInfo& info, MetalDevice& device);
            virtual ~MetalSampler();

        private:
            id<MTLSamplerState> mSamplerState;
        };
    } // namespace gapi
} // namespace cube
