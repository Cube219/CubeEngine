#pragma once

#include "MetalHeader.h"

#include "GAPI_Pipeline.h"

namespace cube
{
    namespace gapi
    {
        class MetalPipeline : public Pipeline
        {
        public:
            MetalPipeline(const GraphicsPipelineCreateInfo& info) {}
            virtual ~MetalPipeline() {}
        };
    } // namespace gapi
} // namespace cube
