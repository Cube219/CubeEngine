#include "GAPI_Metal.h"

#include <memory>
#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"

#include "Checker.h"
#include "GAPI_MetalBuffer.h"
#include "GAPI_MetalCommandList.h"
#include "GAPI_MetalFence.h"
#include "GAPI_MetalPipeline.h"
#include "GAPI_MetalShader.h"
#include "GAPI_MetalShaderVariable.h"
#include "GAPI_MetalViewport.h"
#include "Logger.h"
#include "MacOS/MacOSPlatform.h"

@implementation CubeMTKView

- (void) drawInMTKView:(MTKView* ) view
{
}

- (void) mtkView:(MTKView* ) view drawableSizeWillChange:(CGSize)size
{
}

@end

namespace cube
{
    GAPI* CreateGAPI()
    {
        return new GAPI_Metal();
    }

    void GAPI_Metal::Initialize(const GAPIInitInfo& initInfo)
    {
        CUBE_LOG(LogType::Info, Metal, "Initialize GAPI_Metal.");

        mInfo = {
            .apiName = GAPIName::Metal,
            .isEnabled = false,
            .useLeftHanded = false
        };

        mDevice = MTLCreateSystemDefaultDevice();

        CubeWindow* window = platform::MacOSPlatform::GetWindow();
        NSRect windowFrame = window.contentView.bounds;
        mView = [[CubeMTKView alloc]
            initWithFrame:windowFrame
            device:mDevice
        ];
        mView.delegate = mView;
        [mView setPaused:NO];
        mView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [window.contentView addSubview:mView];
        
        mCommandQueue = [mDevice newCommandQueue];
        
        InitializeImGUI(initInfo.imGUI);
    }

    void GAPI_Metal::Shutdown(const ImGUIContext& imGUIInfo)
    {
        CUBE_LOG(LogType::Info, Metal, "Shutdown GAPI_Metal.");

        ShutdownImGUI(imGUIInfo);
    }

    void GAPI_Metal::OnBeforeRender()
    {
    }

    void GAPI_Metal::OnAfterRender()
    {
    }

    void GAPI_Metal::OnBeforePresent(gapi::Viewport* viewport)
    {
        if (mImGUIContext.context)
        {
            id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];

            ImDrawData* draw_data = ImGui::GetDrawData();

            static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            mRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:mRenderPassDescriptor];
            [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
            ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            // Present
            [commandBuffer presentDrawable:mView.currentDrawable];
            [commandBuffer commit];
        }
    }

    void GAPI_Metal::OnAfterPresent()
    {
        if (mImGUIContext.context)
        {
            do
            {
                mRenderPassDescriptor = mView.currentRenderPassDescriptor;
            } while (mRenderPassDescriptor == nil);

            ImGui_ImplMetal_NewFrame(mRenderPassDescriptor);
            ImGui_ImplOSX_NewFrame(mView);
        }
    }

    void GAPI_Metal::WaitForGPU()
    {
    }

    SharedPtr<gapi::Buffer> GAPI_Metal::CreateBuffer(const gapi::BufferCreateInfo& info)
    {
        return std::make_shared<gapi::MetalBuffer>(info);
    }

    SharedPtr<gapi::CommandList> GAPI_Metal::CreateCommandList(const gapi::CommandListCreateInfo& info)
    {
        return std::make_shared<gapi::MetalCommandList>(info);
    }

    SharedPtr<gapi::Fence> GAPI_Metal::CreateFence(const gapi::FenceCreateInfo& info)
    {
        return std::make_shared<gapi::MetalFence_old>(info);
    }

    SharedPtr<gapi::Pipeline> GAPI_Metal::CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info)
    {
        return std::make_shared<gapi::MetalPipeline>(info);
    }

    SharedPtr<gapi::Pipeline> GAPI_Metal::CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Shader> GAPI_Metal::CreateShader(const gapi::ShaderCreateInfo& info)
    {
        return std::make_shared<gapi::MetalShader>(info);
    }

    SharedPtr<gapi::ShaderVariablesLayout> GAPI_Metal::CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info)
    {
        return std::make_shared<gapi::MetalShaderVariablesLayout>(info);
    }

    SharedPtr<gapi::Viewport> GAPI_Metal::CreateViewport(const gapi::ViewportCreateInfo& info)
    {
        return std::make_shared<gapi::MetalViewport>(info);
    }

    gapi::TimestampList GAPI_Metal::GetLastTimestampList()
    {
        return {};
    }

    void GAPI_Metal::InitializeImGUI(const ImGUIContext& imGUIInfo)
    {
        if (imGUIInfo.context == nullptr)
        {
            return;
        }

        CUBE_LOG(LogType::Info, DX12, "ImGUI context is set in the initialize info. Initialize ImGUI.");

        mImGUIContext = imGUIInfo;

        ImGui::SetCurrentContext((ImGuiContext*)(imGUIInfo.context));
        ImGui::SetAllocatorFunctions(
            (ImGuiMemAllocFunc)(imGUIInfo.allocFunc),
            (ImGuiMemFreeFunc)(imGUIInfo.freeFunc),
            (void*)(imGUIInfo.userData)
        );

        ImGui_ImplOSX_Init(mView);
        ImGui_ImplMetal_Init(mDevice);

        // Start new frame before starting ImGUI loop in Engine class
        mRenderPassDescriptor = mView.currentRenderPassDescriptor;
        ImGui_ImplMetal_NewFrame(mView.currentRenderPassDescriptor);
        ImGui_ImplOSX_NewFrame(mView);
    }

    void GAPI_Metal::ShutdownImGUI(const ImGUIContext& imGUIInfo)
    {
        if (mImGUIContext.context == nullptr)
        {
            return;
        }
        CUBE_LOG(LogType::Info, DX12, "Shtudown ImGUI.");

        ImGui_ImplMetal_Shutdown();
        ImGui_ImplOSX_Shutdown();
    }
} // namespace cube
