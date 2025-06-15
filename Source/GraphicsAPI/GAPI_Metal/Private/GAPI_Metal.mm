#include "GAPI_Metal.h"

#include <memory>
#include <Metal/Metal.h>
#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"

#include "Checker.h"
#include "GAPI_MetalBuffer.h"
#include "GAPI_MetalCommandList.h"
#include "GAPI_MetalFence.h"
#include "GAPI_MetalPipeline.h"
#include "GAPI_MetalSampler.h"
#include "GAPI_MetalShader.h"
#include "GAPI_MetalShaderVariable.h"
#include "GAPI_MetalTexture.h"
#include "GAPI_MetalViewport.h"
#include "Logger.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSUtility.h"
#include "MetalDevice.h"
#include "MetalShaderCompiler.h"

@implementation CubeImGUIMTKView

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
        CUBE_LOG(Info, Metal, "Initialize GAPI_Metal.");

        mInfo = {
            .apiName = GAPIName::Metal,
            .useLeftHanded = false
        };

        mDevices.resize(1);
        mDevices[0] = new MetalDevice();
        mDevices[0]->Initialize(MTLCreateSystemDefaultDevice());
        mMainDevice = mDevices[0];

        mCommandQueue = [mMainDevice->GetDevice() newCommandQueue];

        InitializeImGUI(initInfo.imGUI);
        MetalShaderCompiler::Initialize(mMainDevice);
    }

    void GAPI_Metal::Shutdown(const ImGUIContext& imGUIInfo)
    {
        CUBE_LOG(Info, Metal, "Shutdown GAPI_Metal.");

        WaitForGPU();

        MetalShaderCompiler::Shutdown();
        ShutdownImGUI(imGUIInfo);

        for (MetalDevice* device : mDevices)
        {
            device->Shutdown();
            delete device;
        }
        mDevices.clear();
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
            // Call Metal_NewFrame at this instead of OnAfterPresent
            do
            {
                mRenderPassDescriptor = mImGUIView.currentRenderPassDescriptor;
            } while (mRenderPassDescriptor == nil);
            ImGui_ImplMetal_NewFrame(mRenderPassDescriptor);

            id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];

            ImDrawData* draw_data = ImGui::GetDrawData();

            static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            mRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:mRenderPassDescriptor];
            [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
            ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer commit];
        }
    }

    void GAPI_Metal::OnAfterPresent()
    {
        if (mImGUIContext.context)
        {
            [mImGUIView.currentDrawable present];
            [mImGUIView draw];

            // Does not call Metal_NewFrame at this point because currentRenderPassDescriptor
            // in MTKView block the process until available.
            ImGui_ImplOSX_NewFrame(mImGUIView);
        }
    }

    void GAPI_Metal::WaitForGPU()
    {
        // TODO: Use another way not using comand buffer?
        id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
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

    SharedPtr<gapi::Sampler> GAPI_Metal::CreateSampler(const gapi::SamplerCreateInfo& info)
    {
        return std::make_shared<gapi::MetalSampler>(info);
    }

    SharedPtr<gapi::Shader> GAPI_Metal::CreateShader(const gapi::ShaderCreateInfo& info)
    {
        gapi::ShaderCompileResult compileResult;

        MetalShaderCompileResult shaderResult = MetalShaderCompiler::Compile(info, compileResult);
        if (!compileResult.warning.empty())
        {
            CUBE_LOG(Warning, Metal, "{0}", compileResult.warning);
        }

        if (!compileResult.error.empty())
        {
            CUBE_LOG(Error, Metal, "{0}", compileResult.error);
        }

        if (shaderResult.function == nil)
        {
            CUBE_LOG(Error, Metal, "Failed to create the shader!");
            return nullptr;
        }

        return std::make_shared<gapi::MetalShader>(shaderResult);
    }

    SharedPtr<gapi::ShaderVariablesLayout> GAPI_Metal::CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info)
    {
        return std::make_shared<gapi::MetalShaderVariablesLayout>(info);
    }

    SharedPtr<gapi::Texture> GAPI_Metal::CreateTexture(const gapi::TextureCreateInfo& info)
    {
        return std::make_shared<gapi::MetalTexture>(info);
    }

    SharedPtr<gapi::Viewport> GAPI_Metal::CreateViewport(const gapi::ViewportCreateInfo& info)
    {
        return std::make_shared<gapi::MetalViewport>(mMainDevice->GetDevice(), mImGUIView, info);
    }

    gapi::TimestampList GAPI_Metal::GetLastTimestampList()
    {
        return {};
    }

    gapi::VRAMStatus GAPI_Metal::GetVRAMUsage()
    {
        gapi::VRAMStatus res = {
            .physicalCurrentUsage = 0, .physicalMaximumUsage = 0,
            .logicalCurrentUsage = 0, .logicalMaximumUsage = 0
        };

        res.logicalCurrentUsage = mMainDevice->GetDevice().currentAllocatedSize;
        return res;
    }

    void GAPI_Metal::InitializeImGUI(const ImGUIContext& imGUIInfo)
    {
        if (imGUIInfo.context == nullptr)
        {
            return;
        }

        CUBE_LOG(Info, Metal, "ImGUI context is set in the initialize info. Initialize ImGUI.");

        mImGUIContext = imGUIInfo;

        ImGui::SetCurrentContext((ImGuiContext*)(imGUIInfo.context));
        ImGui::SetAllocatorFunctions(
            (ImGuiMemAllocFunc)(imGUIInfo.allocFunc),
            (ImGuiMemFreeFunc)(imGUIInfo.freeFunc),
            (void*)(imGUIInfo.userData)
        );

        platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
            CubeWindow* window = platform::MacOSPlatform::GetWindow();
            NSRect windowFrame = window.contentView.bounds;
            mImGUIView = [[CubeImGUIMTKView alloc]
                initWithFrame:windowFrame
                device:mMainDevice->GetDevice()
            ];
            mImGUIView.delegate = mImGUIView;
            mImGUIView.paused = YES;
            mImGUIView.enableSetNeedsDisplay = YES;
            mImGUIView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            mImGUIView.layer.opaque = NO;
            [window.contentView addSubview:mImGUIView];
        });

        ImGui_ImplOSX_Init(mImGUIView);
        ImGui_ImplMetal_Init(mMainDevice->GetDevice());

        // Start new frame before starting ImGUI loop in Engine class
        mRenderPassDescriptor = mImGUIView.currentRenderPassDescriptor;
        ImGui_ImplMetal_NewFrame(mImGUIView.currentRenderPassDescriptor);
        ImGui_ImplOSX_NewFrame(mImGUIView);
    }

    void GAPI_Metal::ShutdownImGUI(const ImGUIContext& imGUIInfo)
    {
        if (mImGUIContext.context == nullptr)
        {
            return;
        }
        CUBE_LOG(Info, Metal, "Shtudown ImGUI.");

        // Wait until the main queue will flush.
        platform::MacOSUtility::DispatchToMainThreadAndWait([] {});

        [mImGUIView release];

        ImGui_ImplMetal_Shutdown();
        ImGui_ImplOSX_Shutdown();
    }
} // namespace cube
