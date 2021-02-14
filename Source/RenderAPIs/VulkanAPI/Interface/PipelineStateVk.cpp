#include "PipelineStateVk.h"

#include "../VulkanDevice.h"
#include "../VulkanTypeConversion.h"
#include "../VulkanUtility.h"
#include "RenderPassVk.h"
#include "ShaderVariablesVk.h"
#include "ShaderVk.h"
#include "Core/Allocator/FrameAllocator.h"
#include "Core/Assertion.h"

#include "RenderAPIs/VulkanAPI/VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        VkVertexInputRate ConvertToVkVertexInputRate(InputRate inputRate)
        {
            switch(inputRate) {
                case InputRate::PerVertex:
                    return VK_VERTEX_INPUT_RATE_VERTEX;
                case InputRate::PerInstance:
                    return VK_VERTEX_INPUT_RATE_INSTANCE;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid InputRate({}).", inputRate);
            return VK_VERTEX_INPUT_RATE_VERTEX;
        }

        VkPrimitiveTopology ConvertToVkPrimitiveTopology(PrimitiveTopology topology)
        {
            switch(topology) {
                case PrimitiveTopology::TriangleList:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::TriangleStrip:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                case PrimitiveTopology::LineList:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopology::LineStrip:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                case PrimitiveTopology::ControlPointPatchList1:
                case PrimitiveTopology::ControlPointPatchList2:
                case PrimitiveTopology::ControlPointPatchList3:
                case PrimitiveTopology::ControlPointPatchList4:
                case PrimitiveTopology::ControlPointPatchList5:
                case PrimitiveTopology::ControlPointPatchList6:
                case PrimitiveTopology::ControlPointPatchList7:
                case PrimitiveTopology::ControlPointPatchList8:
                case PrimitiveTopology::ControlPointPatchList9:
                case PrimitiveTopology::ControlPointPatchList10:
                case PrimitiveTopology::ControlPointPatchList11:
                case PrimitiveTopology::ControlPointPatchList12:
                case PrimitiveTopology::ControlPointPatchList13:
                case PrimitiveTopology::ControlPointPatchList14:
                case PrimitiveTopology::ControlPointPatchList15:
                case PrimitiveTopology::ControlPointPatchList16:
                case PrimitiveTopology::ControlPointPatchList17:
                case PrimitiveTopology::ControlPointPatchList18:
                case PrimitiveTopology::ControlPointPatchList19:
                case PrimitiveTopology::ControlPointPatchList20:
                case PrimitiveTopology::ControlPointPatchList21:
                case PrimitiveTopology::ControlPointPatchList22:
                case PrimitiveTopology::ControlPointPatchList23:
                case PrimitiveTopology::ControlPointPatchList24:
                case PrimitiveTopology::ControlPointPatchList25:
                case PrimitiveTopology::ControlPointPatchList26:
                case PrimitiveTopology::ControlPointPatchList27:
                case PrimitiveTopology::ControlPointPatchList28:
                case PrimitiveTopology::ControlPointPatchList29:
                case PrimitiveTopology::ControlPointPatchList30:
                case PrimitiveTopology::ControlPointPatchList31:
                case PrimitiveTopology::ControlPointPatchList32:
                    return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid PrimitiveTopology({}).", topology);
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        VkPolygonMode ConvertToVkPolygonMode(FillMode fillMode)
        {
            switch(fillMode) {
                case FillMode::Point: return VK_POLYGON_MODE_POINT;
                case FillMode::Line:  return VK_POLYGON_MODE_LINE;
                case FillMode::Solid: return VK_POLYGON_MODE_FILL;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid FillMode({}).", fillMode);
            return VK_POLYGON_MODE_FILL;
        }

        VkCullModeFlags ConvertToVkCullModeFlags(CullMode cullMode)
        {
            switch(cullMode) {
                case CullMode::None:  return VK_CULL_MODE_NONE;
                case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
                case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid CullMode({}).", cullMode);
            return VK_CULL_MODE_NONE;
        }

        VkFrontFace ConvertToVkFrontFace(FrontFace frontFace)
        {
            switch(frontFace) {
                case FrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
                case FrontFace::Counterclockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid FrontFace({}).", frontFace);
            return VK_FRONT_FACE_CLOCKWISE;
        }

        VkCompareOp ConvertToVkCompareOp(CompareFunction compareFunction)
        {
            switch(compareFunction) {
                case CompareFunction::Less:         return VK_COMPARE_OP_LESS;
                case CompareFunction::LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
                case CompareFunction::Greater:      return VK_COMPARE_OP_GREATER;
                case CompareFunction::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
                case CompareFunction::Equal:        return VK_COMPARE_OP_EQUAL;
                case CompareFunction::NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
                case CompareFunction::Never:        return VK_COMPARE_OP_NEVER;
                case CompareFunction::Always:       return VK_COMPARE_OP_ALWAYS;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid CompareFunction({}).", compareFunction);
            return VK_COMPARE_OP_ALWAYS;
        }

        VkStencilOp ConvertToVkStencilOp(StencilOperator stencilOperator)
        {
            switch(stencilOperator) {
                case StencilOperator::Keep:              return VK_STENCIL_OP_KEEP;
                case StencilOperator::Zero:              return VK_STENCIL_OP_ZERO;
                case StencilOperator::Replace:           return VK_STENCIL_OP_REPLACE;
                case StencilOperator::Invert:            return VK_STENCIL_OP_INVERT;
                case StencilOperator::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case StencilOperator::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case StencilOperator::IncrementAndWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case StencilOperator::DecrementAndWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid StencilOperator({}).", stencilOperator);
            return VK_STENCIL_OP_KEEP;
        }

        VkStencilOpState ConvertToVkStencilOpState(CompareFunction compareFunction, StencilOperator stencilFailOp, StencilOperator stencilPassOp,
            StencilOperator stencilDepthFailOp, Uint8 stencilReadMask, Uint8 stencilWriteMask)
        {
            VkStencilOpState state;
            state.failOp = ConvertToVkStencilOp(stencilFailOp);
            state.passOp = ConvertToVkStencilOp(stencilPassOp);
            state.depthFailOp = ConvertToVkStencilOp(stencilDepthFailOp);
            state.compareOp = ConvertToVkCompareOp(compareFunction);
            state.compareMask = stencilReadMask;
            state.writeMask = stencilWriteMask;
            state.reference = 0; // Set dynamically

            return state;
        }

        VkBlendFactor ConvertToVkBlendFactor(BlendFactor blendFactor)
        {
            switch(blendFactor) {
                case BlendFactor::Zero:                    return VK_BLEND_FACTOR_ZERO;
                case BlendFactor::One:                     return VK_BLEND_FACTOR_ONE;
                case BlendFactor::SourceColor:             return VK_BLEND_FACTOR_SRC_COLOR;
                case BlendFactor::InverseSourceColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case BlendFactor::SourceAlpha:             return VK_BLEND_FACTOR_SRC_ALPHA;
                case BlendFactor::InverseSourceAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::DestinationColor:        return VK_BLEND_FACTOR_DST_COLOR;
                case BlendFactor::InverseDestinationColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case BlendFactor::DestinationAlpha:        return VK_BLEND_FACTOR_DST_ALPHA;
                case BlendFactor::InverseDestinationAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case BlendFactor::Constant:                return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case BlendFactor::InverseConstant:         return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case BlendFactor::Source1Color:            return VK_BLEND_FACTOR_SRC1_COLOR;
                case BlendFactor::InverseSource1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
                case BlendFactor::Source1Alpha:            return VK_BLEND_FACTOR_SRC1_ALPHA;
                case BlendFactor::InverseSource1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
                default: 
                    break;
            }

            ASSERTION_FAILED("Invalid BlendFactor({}).", blendFactor);
            return VK_BLEND_FACTOR_ZERO;
        }

        VkBlendOp ConvertToVkBlendOp(BlendOperator blendOperator)
        {
            switch(blendOperator) {
                case BlendOperator::Add: return VK_BLEND_OP_ADD;
                case BlendOperator::Subtract: return VK_BLEND_OP_SUBTRACT;
                case BlendOperator::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendOperator::Min: return VK_BLEND_OP_MIN;
                case BlendOperator::Max: return VK_BLEND_OP_MAX;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid BlendOperator({}).", blendOperator);
            return VK_BLEND_OP_ADD;
        }

        VkColorComponentFlags ConvertToVkColorComponentFlags(ColorWriteMaskFlags colorWriteMaskFlags)
        {
            VkColorComponentFlags flags = 0;

            if(colorWriteMaskFlags.IsSet(ColorWriteMaskFlag::Red)) flags |= VK_COLOR_COMPONENT_R_BIT;
            if(colorWriteMaskFlags.IsSet(ColorWriteMaskFlag::Green)) flags |= VK_COLOR_COMPONENT_G_BIT;
            if(colorWriteMaskFlags.IsSet(ColorWriteMaskFlag::Blue)) flags |= VK_COLOR_COMPONENT_B_BIT;
            if(colorWriteMaskFlags.IsSet(ColorWriteMaskFlag::Alpha)) flags |= VK_COLOR_COMPONENT_A_BIT;

            return flags;
        }

        GraphicsPipelineStateVk::GraphicsPipelineStateVk(VulkanDevice& device, const GraphicsPipelineStateCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;

            // Vertex input (InputLayout)
            // layout -> binding
            // element -> attribute
            Uint32 totalNumElements = 0;
            for(Uint32 i = 0; i < info.numInputLayouts; ++i) {
                totalNumElements += info.inputLayouts[i].numElements;
            }

            FrameVector<VkVertexInputBindingDescription> vertexInputBindingDescs(info.numInputLayouts);
            FrameVector<VkVertexInputAttributeDescription> vertexInputAttributeDescs(totalNumElements);
            Uint64 attributeIndex = 0;

            for(Uint32 i = 0; i < info.numInputLayouts; ++i) {
                auto& inputLayout = info.inputLayouts[i];

                vertexInputBindingDescs[i].binding = i;
                vertexInputBindingDescs[i].stride = inputLayout.stride;
                vertexInputBindingDescs[i].inputRate = ConvertToVkVertexInputRate(inputLayout.inputRate);

                for(Uint32 j = 0; j < inputLayout.numElements; ++j) {
                    auto& element = inputLayout.elements[j];

                    vertexInputAttributeDescs[attributeIndex].binding = i;
                    vertexInputAttributeDescs[attributeIndex].location = j;
                    vertexInputAttributeDescs[attributeIndex].format = TextureFormatToVkFormat(element.format);
                    vertexInputAttributeDescs[attributeIndex].offset = element.offset;
                    attributeIndex++;
                }
            }

            VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
            vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputCreateInfo.pNext = nullptr;
            vertexInputCreateInfo.flags = 0;
            vertexInputCreateInfo.vertexBindingDescriptionCount = SCast(Uint32)(vertexInputBindingDescs.size());
            vertexInputCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescs.data();
            vertexInputCreateInfo.vertexAttributeDescriptionCount = SCast(Uint32)(vertexInputAttributeDescs.size());
            vertexInputCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescs.data();

            // Input assembly (PrimitiveTopology)
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
            inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyStateCreateInfo.pNext = nullptr;
            inputAssemblyStateCreateInfo.flags = 0;
            inputAssemblyStateCreateInfo.topology = ConvertToVkPrimitiveTopology(info.topology);
            inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

            // Viewport / Scissor
            // TODO: 이것은 어떻게 할까? 일단 무조건 dynamic으로?
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
            viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.pNext = nullptr;
            viewportStateCreateInfo.flags = 0;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = nullptr; // Use dynamic viewports
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = nullptr; // Use dynamic scissors

            // Rasterization (RasterizerState)
            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
            rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.pNext = nullptr;
            rasterizationStateCreateInfo.flags = 0;
            rasterizationStateCreateInfo.polygonMode = ConvertToVkPolygonMode(info.rasterizerState.fillMode);
            rasterizationStateCreateInfo.cullMode = ConvertToVkCullModeFlags(info.rasterizerState.cullMode);
            rasterizationStateCreateInfo.frontFace = ConvertToVkFrontFace(info.rasterizerState.frontFace);
            rasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // TODO: Rasterization 기능확장 때 구현
            rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
            rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0;
            rasterizationStateCreateInfo.depthBiasClamp = 0;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
            rasterizationStateCreateInfo.lineWidth = 1.0f;

            // Multisampler (TODO: 차후 구현)
            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
            multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.pNext = nullptr;
            multisampleStateCreateInfo.flags = 0;
            multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
            multisampleStateCreateInfo.minSampleShading = 0.0f;
            multisampleStateCreateInfo.pSampleMask = nullptr;
            multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
            multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

            // Depth Stencil (DepthStencilState)
            VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
            depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilStateCreateInfo.pNext = nullptr;
            depthStencilStateCreateInfo.flags = 0;
            depthStencilStateCreateInfo.depthTestEnable = info.depthStencilState.enableDepthWrite;
            depthStencilStateCreateInfo.depthWriteEnable = info.depthStencilState.enableDepthWrite;
            depthStencilStateCreateInfo.depthCompareOp = ConvertToVkCompareOp(info.depthStencilState.depthFunction);
            depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.minDepthBounds = 0.0f;
            depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
            depthStencilStateCreateInfo.stencilTestEnable = (info.depthStencilState.enableFrontFaceStencil || info.depthStencilState.enableBackFaceStencil) ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.back = ConvertToVkStencilOpState(info.depthStencilState.backFaceStencilFunction, info.depthStencilState.backFaceStencilFailOp,
                info.depthStencilState.backFaceStencilPassOp, info.depthStencilState.backFaceStencilDepthFailOp,
                info.depthStencilState.stencilReadMask, info.depthStencilState.stencilWriteMask);
            depthStencilStateCreateInfo.front = ConvertToVkStencilOpState(info.depthStencilState.frontFaceStencilFunction, info.depthStencilState.frontFaceStencilFailOp,
                info.depthStencilState.frontFaceStencilPassOp, info.depthStencilState.frontFaceStencilDepthFailOp,
                info.depthStencilState.stencilReadMask, info.depthStencilState.stencilWriteMask);

            // Color blend (BlendState)
            FrameVector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(info.numRenderTargets);
            for(Uint32 i = 0; i < info.numRenderTargets; ++i) {
                auto& blendState = info.blendStates[i];
                auto& attachment = colorBlendAttachments[i];

                attachment.blendEnable = blendState.enableBlend;
                attachment.srcColorBlendFactor = ConvertToVkBlendFactor(blendState.colorSrcBlend);
                attachment.dstColorBlendFactor = ConvertToVkBlendFactor(blendState.colorDstBlend);
                attachment.colorBlendOp = ConvertToVkBlendOp(blendState.colorBlendOp);
                attachment.srcAlphaBlendFactor = ConvertToVkBlendFactor(blendState.alphaSrcBlend);
                attachment.dstAlphaBlendFactor = ConvertToVkBlendFactor(blendState.alphaDstBlend);
                attachment.alphaBlendOp = ConvertToVkBlendOp(blendState.alphaBlendOp);
                attachment.colorWriteMask = ConvertToVkColorComponentFlags(blendState.colorWriteMask);
            }
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
            colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.pNext = nullptr;
            colorBlendStateCreateInfo.flags = 0;
            colorBlendStateCreateInfo.logicOpEnable = false;
            colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
            colorBlendStateCreateInfo.attachmentCount = SCast(Uint32)(colorBlendAttachments.size());
            colorBlendStateCreateInfo.pAttachments = colorBlendAttachments.data();
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            // Dynamic states
            FrameVector<VkDynamicState> dynamicState;
            dynamicState.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            dynamicState.push_back(VK_DYNAMIC_STATE_SCISSOR);

            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
            dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.pNext = nullptr;
            dynamicStateCreateInfo.flags = 0;
            dynamicStateCreateInfo.dynamicStateCount = SCast(uint32_t)(dynamicState.size());
            dynamicStateCreateInfo.pDynamicStates = dynamicState.data();

            // Shader stages
            FrameVector<VkPipelineShaderStageCreateInfo> shaderStages;
            if(info.vertexShader != nullptr)
                shaderStages.push_back(DCast(ShaderVk*)(info.vertexShader)->GetVkPipelineShaderStageCreateInfo());
            if(info.pixelShader != nullptr)
                shaderStages.push_back(DCast(ShaderVk*)(info.pixelShader)->GetVkPipelineShaderStageCreateInfo());
            if(info.domainShader != nullptr)
                shaderStages.push_back(DCast(ShaderVk*)(info.domainShader)->GetVkPipelineShaderStageCreateInfo());
            if(info.hullShader != nullptr)
                shaderStages.push_back(DCast(ShaderVk*)(info.hullShader)->GetVkPipelineShaderStageCreateInfo());
            if(info.geometryShader != nullptr)
                shaderStages.push_back(DCast(ShaderVk*)(info.geometryShader)->GetVkPipelineShaderStageCreateInfo());

            // Pipeline layout (ShaderParametersLayout)
            FrameVector<VkDescriptorSetLayout> descSetLayouts(info.numShaderVariablesLayouts);

            for(Uint32 i = 0; i < info.numShaderVariablesLayouts; ++i) {
                ShaderVariablesLayoutVk* layout = DCast(ShaderVariablesLayoutVk*)(info.shaderVariablesLayouts[i]);
                descSetLayouts[i] = layout->GetVkDescriptorSetLayout();
            }

            VkPipelineLayoutCreateInfo layoutInfo;
            layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutInfo.pNext = nullptr;
            layoutInfo.flags = 0;
            layoutInfo.setLayoutCount = SCast(Uint32)(descSetLayouts.size());
            layoutInfo.pSetLayouts = descSetLayouts.data();
            layoutInfo.pushConstantRangeCount = 0;
            layoutInfo.pPushConstantRanges = nullptr;
            res = vkCreatePipelineLayout(mDevice.GetHandle(), &layoutInfo, nullptr, &mPipelineLayout);
            CHECK_VK(res, "Failed to create VkPipelineLayout");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mPipelineLayout, FrameFormat("PipelineLayout of {}", info.debugName).c_str());

            // Graphics pipeline
            VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.pNext = nullptr;
            pipelineCreateInfo.flags = 0;
            pipelineCreateInfo.layout = mPipelineLayout;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // TODO: 구현?
            pipelineCreateInfo.basePipelineIndex = 0;
            pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
            pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineCreateInfo.pTessellationState = nullptr; // TODO: Tessellation 구현시 구현
            pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
            pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
            pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
            pipelineCreateInfo.stageCount = SCast(Uint32)(shaderStages.size());
            pipelineCreateInfo.pStages = shaderStages.data();
            pipelineCreateInfo.renderPass = DCast(RenderPassVk*)(info.renderPass)->GetVkRenderPass();
            pipelineCreateInfo.subpass = 0;

            res = vkCreateGraphicsPipelines(mDevice.GetHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline);
            CHECK_VK(res, "Failed to create VkGraphicsPipeline");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mPipeline, info.debugName);
        }

        GraphicsPipelineStateVk::~GraphicsPipelineStateVk()
        {
            vkDestroyPipeline(mDevice.GetHandle(), mPipeline, nullptr);
            vkDestroyPipelineLayout(mDevice.GetHandle(), mPipelineLayout, nullptr);
        }

        ComputePipelineStateVk::ComputePipelineStateVk(VulkanDevice& device, const ComputePipelineStateCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;

            // Shader stage
            VkPipelineShaderStageCreateInfo shaderStage;
            shaderStage = DCast(ShaderVk*)(info.shader)->GetVkPipelineShaderStageCreateInfo();

            // Pipeline layout (ShaderParametersLayout)
            FrameVector<VkDescriptorSetLayout> descSetLayouts(info.numShaderVariablesLayouts);

            for(Uint32 i = 0; i < info.numShaderVariablesLayouts; ++i) {
                ShaderVariablesLayoutVk* layout = DCast(ShaderVariablesLayoutVk*)(info.shaderVariablesLayouts[i]);
                descSetLayouts[i] = layout->GetVkDescriptorSetLayout();
            }

            VkPipelineLayoutCreateInfo layoutInfo;
            layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutInfo.pNext = nullptr;
            layoutInfo.flags = 0;
            layoutInfo.setLayoutCount = SCast(Uint32)(descSetLayouts.size());
            layoutInfo.pSetLayouts = descSetLayouts.data();
            layoutInfo.pushConstantRangeCount = 0;
            layoutInfo.pPushConstantRanges = nullptr;
            res = vkCreatePipelineLayout(mDevice.GetHandle(), &layoutInfo, nullptr, &mPipelineLayout);
            CHECK_VK(res, "Failed to create VkPipelineLayout");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mPipelineLayout, FrameFormat("PipelineLayout of {}", info.debugName).c_str());

            // Compute pipeline
            VkComputePipelineCreateInfo pipelineCreateInfo;
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.pNext = nullptr;
            pipelineCreateInfo.flags = 0;
            pipelineCreateInfo.stage = shaderStage;
            pipelineCreateInfo.layout = mPipelineLayout;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineCreateInfo.basePipelineIndex = 0;

            res = vkCreateComputePipelines(mDevice.GetHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline);
            CHECK_VK(res, "Failed to create VkComputePipeline");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mPipeline, info.debugName);
        }

        ComputePipelineStateVk::~ComputePipelineStateVk()
        {
            vkDestroyPipeline(mDevice.GetHandle(), mPipeline, nullptr);
            vkDestroyPipelineLayout(mDevice.GetHandle(), mPipelineLayout, nullptr);
        }
    } // namespace rapi
} // namespace cube
