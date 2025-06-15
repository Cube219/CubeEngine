#pragma once

#include "MetalHeader.h"

#include "GAPI_Sampler.h"

namespace cube
{
    namespace gapi
    {
        class MetalSampler : public Sampler
        {
        public:
            MetalSampler(const SamplerCreateInfo& info)
            {
            }
            virtual ~MetalSampler()
            {
            }
        };
    } // namespace gapi
} // namespace cube
