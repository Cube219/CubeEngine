#pragma once

#include "MetalHeader.h"

#include "GAPI_Pipeline.h"

namespace cube
{
    namespace gapi
    {
        class MetalGraphicsPipeline : public GraphicsPipeline
        {
        public:
            MetalGraphicsPipeline(const GraphicsPipelineCreateInfo& info) {}
            virtual ~MetalGraphicsPipeline() {}
        };

        class MetalComputePipeline : public ComputePipeline
        {
        public:
            MetalComputePipeline(const ComputePipelineCreateInfo& info) {}
            virtual ~MetalComputePipeline() {}
        };
    } // namespace gapi
} // namespace cube
