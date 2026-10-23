#include "GAPI_Metal.h"

#include <memory>
#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"

#include "Checker.h"
#include "GAPI_MetalAccelerationStructure.h"
#include "GAPI_MetalBuffer.h"
#include "GAPI_MetalCommandList.h"
#include "GAPI_MetalFence.h"
#include "GAPI_MetalPipeline.h"
#include "GAPI_MetalSampler.h"
#include "GAPI_MetalShader.h"
#include "GAPI_MetalShaderParameter.h"
#include "GAPI_MetalSwapChain.h"
#include "GAPI_MetalTexture.h"
#include "Logger.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSUtility.h"
#include "MetalDevice.h"
#include "MetalShaderCompiler.h"
#include "MetalTypes.h"

@implementation CubeMetalView

- (CALayer*)makeBackingLayer
{
    return [CAMetalLayer layer];
}

- (BOOL)isOpaque
{
    return YES;
}

- (void)updateDrawableSize
{
    if (![self.layer isKindOfClass:[CAMetalLayer class]])
    {
        return;
    }

    CAMetalLayer* layer = (CAMetalLayer*)self.layer;
    CGFloat scale = self.window != nil ? self.window.backingScaleFactor : 1.0;
    CGSize drawableSize = CGSizeMake(self.bounds.size.width * scale, self.bounds.size.height * scale);
    if (drawableSize.width < 1.0 || drawableSize.height < 1.0)
    {
        return;
    }

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    layer.contentsScale = scale;
    layer.drawableSize = drawableSize;
    [CATransaction commit];
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    [self updateDrawableSize];
}

- (void)viewDidChangeBackingProperties
{
    [super viewDidChangeBackingProperties];
    [self updateDrawableSize];
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

        InitializeTypes();

        mDevices.resize(1);
        mDevices[0] = new MetalDevice();
        mDevices[0]->Initialize(MTLCreateSystemDefaultDevice(), initInfo.numGPUSync);
        mMainDevice = mDevices[0];
        CHECK(mMainDevice->CheckFeatureRequirements());

        mInfo = {
            .apiName = gapi::GAPIName::Metal,
            .constantBufferAlignment = 4,
            // All non-transient Metal resources use MTLStorageModeShared (mappable).
            .supportsDirectMapInBuffer = true,
            .supportsDirectMapInTexture = true,
            .needsToOptimizeTextureContentsUsingCommandList = true,

            .supportsHWRT = mMainDevice->IsHWRTSupported(),
            .scratchBufferAlignment = 1,
        };

        InitializeMainView();
        InitializeImGUI(initInfo.imGUI);
        MetalShaderCompiler::Initialize(mMainDevice);

        mCurrentGPUFrame = 1;
        SetNumGPUSync(initInfo.numGPUSync);

        mShaderParameterHelper = std::make_unique<gapi::MetalShaderParameterHelper>();
        mShaderParameterHelper->Initialize();
    }

    void GAPI_Metal::Shutdown(const ImGUIContext& imGUIInfo)
    {
        CUBE_LOG(Info, Metal, "Shutdown GAPI_Metal.");

        WaitAllGPUSync();

        mShaderParameterHelper->Shutdown();
        mShaderParameterHelper = nullptr;

        MetalShaderCompiler::Shutdown();
        ShutdownImGUI(imGUIInfo);

        for (MetalDevice* device : mDevices)
        {
            device->Shutdown();
            delete device;
        }
        mDevices.clear();
    }

    void GAPI_Metal::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        mMainDevice->SetNumGPUSync(newNumGPUSync);
    }

    void GAPI_Metal::OnBeforeRender()
    {
    }

    void GAPI_Metal::OnAfterRender()
    {
    }

    void GAPI_Metal::OnBeforePresent(gapi::Texture* backbuffer)
    {
        if (mImGUIContext.context)
        {
            gapi::MetalTexture* metalBackbuffer = dynamic_cast<gapi::MetalTexture*>(backbuffer);
            CHECK(metalBackbuffer);

            id<MTLTexture> backbufferTexture = metalBackbuffer->GetMTLTexture();
            CHECK(backbufferTexture != nil);

            // Draw ImGui into the same drawable the scene was rendered into.
            // Load the scene color and store the composited result; the swap chain presents it afterwards.
            MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDescriptor.colorAttachments[0].texture = backbufferTexture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            ImGui_ImplMetal_NewFrame(renderPassDescriptor);

            id<MTLCommandBuffer> commandBuffer = [mMainDevice->GetMainCommandQueue() commandBuffer];
            commandBuffer.label = @"ImGui command buffer";

            ImDrawData* draw_data = ImGui::GetDrawData();

            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
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
            platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
                ImGui_ImplOSX_NewFrame(mMainView);
            });
        }
    }

    void GAPI_Metal::BeginRenderingFrame()
    {
        mMainDevice->BeginGPUFrame(mCurrentGPUFrame);
    }

    void GAPI_Metal::EndRenderingFrame()
    {
        mMainDevice->EndGPUFrame(mCurrentGPUFrame);
        mCurrentGPUFrame++;
    }

    void GAPI_Metal::WaitAllGPUSync()
    {
        mMainDevice->WaitAllGPUSync();
    }

    const gapi::ShaderParameterHelper& GAPI_Metal::GetShaderParameterHelper() const
    {
        return *mShaderParameterHelper.get();
    }

    SharedPtr<gapi::Buffer> GAPI_Metal::CreateBuffer(const gapi::BufferCreateInfo& info)
    {
        return std::make_shared<gapi::MetalBuffer>(info, *mMainDevice);
    }

    SharedPtr<gapi::CommandList> GAPI_Metal::CreateCommandList(const gapi::CommandListCreateInfo& info)
    {
        return std::make_shared<gapi::MetalCommandList>(info, *mMainDevice);
    }

    SharedPtr<gapi::Fence> GAPI_Metal::CreateFence(const gapi::FenceCreateInfo& info)
    {
        return std::make_shared<gapi::MetalFence>(info, *mMainDevice);
    }

    SharedPtr<gapi::GraphicsPipeline> GAPI_Metal::CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info)
    {
        return std::make_shared<gapi::MetalGraphicsPipeline>(info, *mMainDevice);
    }

    SharedPtr<gapi::ComputePipeline> GAPI_Metal::CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info)
    {
        return std::make_shared<gapi::MetalComputePipeline>(info, *mMainDevice);
    }

    SharedPtr<gapi::Sampler> GAPI_Metal::CreateSampler(const gapi::SamplerCreateInfo& info)
    {
        return std::make_shared<gapi::MetalSampler>(info, *mMainDevice);
    }

    SharedPtr<gapi::Shader> GAPI_Metal::CreateShader(const gapi::ShaderCreateInfo& info)
    {
        gapi::ShaderCompileResult compileResult;

        MetalShaderCompileResult shaderResult = MetalShaderCompiler::Compile(info, compileResult);

        return std::make_shared<gapi::MetalShader>(shaderResult, compileResult.warning, compileResult.error, std::move(compileResult.dependencyFilePaths));
    }

    SharedPtr<gapi::Texture> GAPI_Metal::CreateTexture(const gapi::TextureCreateInfo& info)
    {
        return std::make_shared<gapi::MetalTexture>(info, *mMainDevice);
    }

    SharedPtr<gapi::SwapChain> GAPI_Metal::CreateSwapChain(const gapi::SwapChainCreateInfo& info)
    {
        return std::make_shared<gapi::MetalSwapChain>(*mMainDevice, mMainView, info);
    }

    SharedPtr<gapi::BLAS> GAPI_Metal::CreateBLAS(const gapi::BLASCreateInfo& createInfo)
    {
        return std::make_shared<gapi::MetalBLAS>(createInfo, *mMainDevice);
    }

    SharedPtr<gapi::TLAS> GAPI_Metal::CreateTLAS(const gapi::TLASCreateInfo& createInfo)
    {
        return std::make_shared<gapi::MetalTLAS>(createInfo, *mMainDevice);
    }

    SharedPtr<gapi::RayTracingPipeline> GAPI_Metal::CreateRayTracingPipeline(const gapi::RayTracingPipelineCreateInfo& createInfo)
    {
        return std::make_shared<gapi::MetalRayTracingPipeline>(createInfo, *mMainDevice);
    }

    gapi::TimestampRangeList GAPI_Metal::GetLastTimestampRangeList()
    {
        return mMainDevice->GetTimestampManager().GetLastTimestampRangeList();
    }

    gapi::VRAMStatus GAPI_Metal::GetVRAMUsage()
    {
        gapi::VRAMStatus res = {
            .physicalCurrentUsage = 0, .physicalMaximumUsage = 0,
            .logicalCurrentUsage = 0, .logicalMaximumUsage = 0
        };

        res.logicalCurrentUsage = mMainDevice->GetMTLDevice().currentAllocatedSize;
        return res;
    }

    void GAPI_Metal::InitializeMainView()
    {
        platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
            CubeWindow* window = platform::MacOSPlatform::GetWindow();
            NSRect windowFrame = window.contentView.bounds;

            mMainView = [[CubeMetalView alloc] initWithFrame:windowFrame];
            mMainView.wantsLayer = YES;
            mMainView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

            // The swap chain configures the concrete pixel format on creation.
            [window.contentView addSubview:mMainView];
            [mMainView updateDrawableSize];
        });
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
            ImGui_ImplOSX_Init(mMainView);

            // Override ImeDataFn to ensure it runs in main thread. It is called from text input
            // and calls UI-related methods (NSTextInputContext).
            static auto defaultImeDataFn = ImGui::GetPlatformIO().Platform_SetImeDataFn;
            ImGui::GetPlatformIO().Platform_SetImeDataFn = [](ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data) -> void {
                if ([NSThread isMainThread])
                {
                    defaultImeDataFn(ctx, viewport, data);
                }
                else
                {
                    platform::MacOSUtility::DispatchToMainThread([ctx, viewport, data] {
                        defaultImeDataFn(ctx, viewport, data);
                    });
                }
            };
        });

        ImGui_ImplMetal_Init(mMainDevice->GetMTLDevice());

        // Start new frame before starting ImGUI loop in Engine class
        platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
            ImGui_ImplOSX_NewFrame(mMainView);
        });
    }

    void GAPI_Metal::ShutdownImGUI(const ImGUIContext& imGUIInfo)
    {
        // Wait until the main queue will flush.
        platform::MacOSUtility::DispatchToMainThreadAndWait([] {});

        if (mImGUIContext.context != nullptr)
        {
            CUBE_LOG(Info, Metal, "Shutdown ImGUI.");
            ImGui_ImplMetal_Shutdown();
            ImGui_ImplOSX_Shutdown();
        }

        platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
            [mMainView removeFromSuperview];
            mMainView = nil;
        });
    }
} // namespace cube
