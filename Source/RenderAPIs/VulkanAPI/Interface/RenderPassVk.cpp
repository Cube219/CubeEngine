#include "RenderPassVk.h"

#include "../VulkanTypeConversion.h"
#include "RenderAPIs/VulkanAPI/VulkanDebug.h"
#include "RenderAPIs/VulkanAPI/VulkanDevice.h"
#include "Core/Allocator/FrameAllocator.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        VkAttachmentLoadOp ConvertToVkAttachmentLoadOp(LoadOperator loadOp)
        {
            switch(loadOp) {
                case LoadOperator::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
                case LoadOperator::Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case LoadOperator::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid LoadOperator({}).", loadOp);
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        }

        VkAttachmentStoreOp ConvertToVkAttachmentStoreOp(StoreOperator storeOp)
        {
            switch(storeOp) {
                case StoreOperator::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
                case StoreOperator::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid StoreOperator({}).", storeOp);
            return VK_ATTACHMENT_STORE_OP_STORE;
        }

        RenderPassVk::RenderPassVk(VulkanDevice& device, const RenderPassCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;

            // Attachments
            FrameVector<VkAttachmentDescription> attachments(info.numAttachments);
            for(Uint32 i = 0; i < info.numAttachments; ++i) {
                auto& attachment = info.attachments[i];

                attachments[i].flags = 0;
                attachments[i].format = TextureFormatToVkFormat(attachment.format);
                attachments[i].samples = ConvertToVkSampleCountFlagBits(attachment.sampleCount);
                attachments[i].loadOp = ConvertToVkAttachmentLoadOp(attachment.loadOp);
                attachments[i].storeOp = ConvertToVkAttachmentStoreOp(attachment.storeOp);
                attachments[i].stencilLoadOp = ConvertToVkAttachmentLoadOp(attachment.stencilLoadOp);
                attachments[i].stencilStoreOp = ConvertToVkAttachmentStoreOp(attachment.stencilStoreOp);
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: Layout 설정 추가
                attachments[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: Layout 설정 추가
            }

            // Subpasses
            struct AttachmentReferenceStorage
            {
                void Add(const Subpass& subpass)
                {
                    inputAttachments.resize(subpass.numInputAttachments);
                    for(Uint32 i = 0; i < subpass.numInputAttachments; ++i) {
                        inputAttachments[i].attachment = subpass.inputAttachmentIndices[i];
                        inputAttachments[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // TODO: Layout 설정 추가?
                    }

                    colorAttachments.resize(subpass.numColorAttachments);
                    for(Uint32 i = 0; i < subpass.numColorAttachments; ++i) {
                        inputAttachments[i].attachment = subpass.colorAttachmentIndices[i];
                        inputAttachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO: Layout 설정 추가?
                    }

                    if(subpass.depthStencilAttachmentIndex != -1) {
                        depthStencilAttachment.attachment = subpass.depthStencilAttachmentIndex;
                        depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: Layout 설정 추가?
                        pDepthStencilAttachment = &depthStencilAttachment;
                    }
                }
                FrameVector<VkAttachmentReference> inputAttachments;
                FrameVector<VkAttachmentReference> colorAttachments;
                VkAttachmentReference depthStencilAttachment;
                VkAttachmentReference* pDepthStencilAttachment = nullptr;
            };
            FrameVector<AttachmentReferenceStorage> attachmentStorages(info.numSubpasses);
            FrameVector<VkSubpassDescription> subpasses(info.numSubpasses);
            for(Uint32 i = 0; i < info.numSubpasses; ++i) {
                auto& subpass = info.subpasses[i];
                auto& storage = attachmentStorages[i];
                storage.Add(subpass);

                subpasses[i].flags = 0;
                subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpasses[i].inputAttachmentCount = subpass.numInputAttachments;
                subpasses[i].pInputAttachments = storage.inputAttachments.data();
                subpasses[i].colorAttachmentCount = subpass.numColorAttachments;
                subpasses[i].pColorAttachments = storage.colorAttachments.data();
                subpasses[i].pResolveAttachments = nullptr; // TODO: sampling 구현 때 구현(281p 참고)
                subpasses[i].pDepthStencilAttachment = storage.pDepthStencilAttachment;
                subpasses[i].preserveAttachmentCount = 0;
                subpasses[i].pPreserveAttachments = nullptr;
            }

            // Subpass dependencies
            FrameVector<VkSubpassDependency> dependencies(info.numDependencies);
            for(Uint32 i = 0; i < info.numDependencies; i++) {
                auto& dependency = info.dependencies[i];

                dependencies[i].srcSubpass = dependency.srcIndex;
                dependencies[i].dstSubpass = dependency.dstIndex;
                // TODO: 차후 커스텀 가능하게?
                dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }

            VkRenderPassCreateInfo renderPassCreateInfo = {};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.pNext = nullptr;
            renderPassCreateInfo.flags = 0;
            renderPassCreateInfo.attachmentCount = SCast(Uint32)(attachments.size());
            renderPassCreateInfo.pAttachments = attachments.data();
            renderPassCreateInfo.subpassCount = SCast(Uint32)(subpasses.size());
            renderPassCreateInfo.pSubpasses = subpasses.data();
            renderPassCreateInfo.dependencyCount = SCast(Uint32)(dependencies.size());
            renderPassCreateInfo.pDependencies = dependencies.data();

            res = vkCreateRenderPass(mDevice.GetHandle(), &renderPassCreateInfo, nullptr, &mRenderPass);
            CHECK_VK(res, "Failed to create render pass");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mRenderPass, info.debugName);
        }

        RenderPassVk::~RenderPassVk()
        {
            vkDestroyRenderPass(mDevice.GetHandle(), mRenderPass, nullptr);
        }
    } // namespace rapi
} // namespace cube
