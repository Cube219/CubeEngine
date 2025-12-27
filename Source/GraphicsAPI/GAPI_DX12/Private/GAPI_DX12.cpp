#include "GAPI_DX12.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "DX12Debug.h"
#include "DX12ShaderCompiler.h"
#include "DX12Types.h"
#include "DX12Utility.h"
#include "GAPI_DX12Buffer.h"
#include "GAPI_DX12CommandList.h"
#include "GAPI_DX12Fence.h"
#include "GAPI_DX12Pipeline.h"
#include "GAPI_DX12Sampler.h"
#include "GAPI_DX12Shader.h"
#include "GAPI_DX12ShaderParameter.h"
#include "GAPI_DX12SwapChain.h"
#include "GAPI_DX12Texture.h"
#include "Logger.h"
#include "Windows/WindowsPlatform.h"

// D3D12 Agility SDK parameters are set in CE-Main/main.cpp.

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cube
{
    // TODO: merge srv heap
    struct ImGUISRVDescHeap
    {
        void Initialize(ID3D12Device* device)
        {
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.NumDescriptors = 4;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            CHECK_HR(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mHeap)));

            mBeginCPU = mHeap->GetCPUDescriptorHandleForHeapStart();
            mBeginGPU = mHeap->GetGPUDescriptorHandleForHeapStart();
            mNumDescriptors = srvHeapDesc.NumDescriptors;
            mDescriptorSize = device->GetDescriptorHandleIncrementSize(srvHeapDesc.Type);

            mFreeIndices.resize(mNumDescriptors);
            for (int i = 0; i < mNumDescriptors; ++i)
            {
                mFreeIndices[i] = mNumDescriptors - i - 1;
            }
        }

        void Shutdown()
        {
            CHECK_FORMAT(mFreeIndices.size() == mNumDescriptors, "Not all descriptors free while shutdown.");

            mFreeIndices.clear();
            mHeap = nullptr;
        }

        void Allocate(D3D12_CPU_DESCRIPTOR_HANDLE* outCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* outGPUDescriptor)
        {
            CHECK(mFreeIndices.size() > 0);

            int index = mFreeIndices.back();
            mFreeIndices.pop_back();

            outCPUDescriptor->ptr = mBeginCPU.ptr + mDescriptorSize * index;
            outGPUDescriptor->ptr = mBeginGPU.ptr + mDescriptorSize * index;
        }

        void Free(D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor)
        {
            int CPUIndex = static_cast<int>(CPUDescriptor.ptr - mBeginCPU.ptr) / mDescriptorSize;
            int GPUIndex = static_cast<int>(GPUDescriptor.ptr - mBeginGPU.ptr) / mDescriptorSize;
            CHECK(CPUIndex == GPUIndex);
            CHECK_FORMAT(std::ranges::find(mFreeIndices, CPUIndex) == mFreeIndices.end(), "Try to free the descriptor that already freed.");

            mFreeIndices.push_back(CPUIndex);
        }

        ComPtr<ID3D12DescriptorHeap> mHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE mBeginCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE mBeginGPU;
        Uint32 mNumDescriptors;
        Uint64 mDescriptorSize;
        Vector<int> mFreeIndices;
    };
    ImGUISRVDescHeap gImGUISRVHeap;

    GAPI* CreateGAPI()
    {
        return new GAPI_DX12();
    }

    void GAPI_DX12::Initialize(const GAPIInitInfo& initInfo)
    {
        CUBE_LOG(Info, DX12, "Initialize GAPI_DX12.");

        mInfo = {
            .apiName = GAPIName::DX12,
            .useLeftHanded = true
        };

        InitializeTypes();

        Uint32 dxgiFactoryFlags = 0;
        if (initInfo.enableDebugLayer)
        {
            DX12Debug::Initialize(dxgiFactoryFlags);
        }

        CHECK_HR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));
        mFactory->QueryInterface(IID_PPV_ARGS(&mFactory6));

        ComPtr<IDXGIAdapter1> adapter;
        auto CheckAndCreateDevice = [this, &initInfo](IDXGIAdapter1* adapter)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            // Check if the adapter support DX12
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                // TODO: Move to after checking feature requirements
                DX12Device* device = new DX12Device();
                device->Initialize(adapter, initInfo.numGPUSync);

                CUBE_LOG(Info, DX12, "Found device: {0}", WindowsStringView(desc.Description));

                if (device->CheckFeatureRequirements())
                {
                    mDevices.push_back(device);
                }
                else
                {
                    CUBE_LOG(Info, DX12, "Device {0} does not meet the feature requirements. It will not be used.", WindowsStringView(desc.Description));

                    device->Shutdown();
                    delete device;
                }
            }
        };
        if (mFactory6)
        {
            for (Uint32 index = 0; DXGI_ERROR_NOT_FOUND != mFactory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)); ++index)
            {
                CheckAndCreateDevice(adapter.Get());
            }
        }
        else
        {
            for (Uint32 index = 0; DXGI_ERROR_NOT_FOUND != mFactory->EnumAdapters1(index, &adapter); ++index)
            {
                CheckAndCreateDevice(adapter.Get());
            }
        }
        CHECK_FORMAT(mDevices.size() > 0, "No device found that can run this engine!");
        mMainDevice = mDevices[0];

        CUBE_LOG(Info, DX12, "Found {0} supported devices.", mDevices.size());
        CUBE_LOG(Info, DX12, "Use the first device: {0}", WindowsStringView(mMainDevice->GetAdapterDesc().Description));

        if (initInfo.enableDebugLayer)
        {
            DX12Debug::InitializeD3DExceptionHandler(*mMainDevice);
        }

        InitializeImGUI(initInfo.imGUI);
        DX12ShaderCompiler::Initialize();

        mCurrentGPUFrame = 0;
        SetNumGPUSync(initInfo.numGPUSync);
    }

    void GAPI_DX12::Shutdown(const ImGUIContext& imGUIInfo)
    {
        CUBE_LOG(Info, DX12, "Shutdown GAPI_DX12.");

        DX12ShaderCompiler::Shutdown();

        ShutdownImGUI(imGUIInfo);

        WaitAllGPUSync();

        DX12Debug::ShutdownD3DExceptionHandler();

        for (DX12Device* device : mDevices)
        {
            device->Shutdown();
            delete device;
        }
        mDevices.clear();

        mFactory6 = nullptr;
        mFactory = nullptr;

        DX12Debug::Shutdown();
    }

    void GAPI_DX12::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        mMainDevice->SetNumGPUSync(newNumGPUSync);
    }

    void GAPI_DX12::OnBeforeRender()
    {
        
    }

    void GAPI_DX12::OnAfterRender()
    {
    }

    void GAPI_DX12::OnBeforePresent(gapi::TextureRTV* backbufferRTV)
    {
        if (mImGUIContext.context)
        {
            mImGUIRenderCommandList->Reset(mMainDevice->GetCommandListManager().GetCurrentAllocator(), nullptr);

            gapi::DX12TextureRTV* dx12BackbufferRTV = dynamic_cast<gapi::DX12TextureRTV*>(backbufferRTV);
            CHECK(dx12BackbufferRTV);
            D3D12_CPU_DESCRIPTOR_HANDLE currentRTVDescriptor = dx12BackbufferRTV->GetDescriptorHandle();
            
            // Render Dear ImGui graphics
            // const float clear_color_with_alpha[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
            // mImGUIRenderCommandList->ClearRenderTargetView(currentRTVDescriptor, clear_color_with_alpha, 0, nullptr);
            mImGUIRenderCommandList->OMSetRenderTargets(1, &currentRTVDescriptor, FALSE, nullptr);
            ID3D12DescriptorHeap* heap = gImGUISRVHeap.mHeap.Get();
            mImGUIRenderCommandList->SetDescriptorHeaps(1, &heap);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mImGUIRenderCommandList.Get());
            mImGUIRenderCommandList->Close();

            ID3D12CommandList* ppCommandLists[] = { mImGUIRenderCommandList.Get() };
            mMainDevice->GetQueueManager().GetMainQueue()->ExecuteCommandLists(1, ppCommandLists);
        }
    }

    void GAPI_DX12::OnAfterPresent()
    {
        if (mImGUIContext.context)
        {
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
        }
    }

    void GAPI_DX12::BeginRenderingFrame()
    {
        mMainDevice->BeginGPUFrame(mCurrentGPUFrame);
    }

    void GAPI_DX12::EndRenderingFrame()
    {
        mMainDevice->EndGPUFrame(mCurrentGPUFrame);
        mCurrentGPUFrame++;
    }

    void GAPI_DX12::WaitAllGPUSync()
    {
        mMainDevice->WaitAllGPUSync();
    }

    const gapi::ShaderParameterHelper& GAPI_DX12::GetShaderParameterHelper() const
    {
        return mMainDevice->GetShaderParameterHelper();
    }

    SharedPtr<gapi::Buffer> GAPI_DX12::CreateBuffer(const gapi::BufferCreateInfo& info)
    {
        return std::make_shared<gapi::DX12Buffer>(info, *mMainDevice);
    }

    SharedPtr<gapi::CommandList> GAPI_DX12::CreateCommandList(const gapi::CommandListCreateInfo& info)
    {
        return std::make_shared<gapi::DX12CommandList>(*mMainDevice, info);
    }

    SharedPtr<gapi::Fence> GAPI_DX12::CreateFence(const gapi::FenceCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
        // return std::make_shared<gapi::DX12Fence_old>(info);
    }

    SharedPtr<gapi::GraphicsPipeline> GAPI_DX12::CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info)
    {
        return std::make_shared<gapi::DX12GraphicsPipeline>(*mMainDevice, info);
    }

    SharedPtr<gapi::ComputePipeline> GAPI_DX12::CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info)
    {
        return std::make_shared<gapi::DX12ComputePipeline>(*mMainDevice, info);
    }

    SharedPtr<gapi::Sampler> GAPI_DX12::CreateSampler(const gapi::SamplerCreateInfo& info)
    {
        return std::make_shared<gapi::DX12Sampler>(*mMainDevice, info);
    }

    SharedPtr<gapi::Shader> GAPI_DX12::CreateShader(const gapi::ShaderCreateInfo& info)
    {
        gapi::ShaderCompileResult compileResult;

        Blob shader = DX12ShaderCompiler::Compile(info, compileResult);

        return std::make_shared<gapi::DX12Shader>(std::move(shader), compileResult.warning, compileResult.error);
    }

    SharedPtr<gapi::Texture> GAPI_DX12::CreateTexture(const gapi::TextureCreateInfo& info)
    {
        return std::make_shared<gapi::DX12Texture>(info, *mMainDevice);
    }

    SharedPtr<gapi::SwapChain> GAPI_DX12::CreateSwapChain(const gapi::SwapChainCreateInfo& info)
    {
        return std::make_shared<gapi::DX12SwapChain>(mFactory.Get(), *mMainDevice, info);
    }

    gapi::TimestampList GAPI_DX12::GetLastTimestampList()
    {
        return mMainDevice->GetQueryManager().GetLastTimestampList();
    }

    gapi::VRAMStatus GAPI_DX12::GetVRAMUsage()
    {
        gapi::VRAMStatus res = {
            .physicalCurrentUsage = 0, .physicalMaximumUsage = 0,
            .logicalCurrentUsage = 0, .logicalMaximumUsage = 0
        };

        IDXGIAdapter3* adapter3 = mMainDevice->GetAdapter3();
        if (adapter3)
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo;
            if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo)))
            {
                res.physicalCurrentUsage = memoryInfo.CurrentUsage;
                res.physicalMaximumUsage = memoryInfo.Budget;
            }

            if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &memoryInfo)))
            {
                res.logicalCurrentUsage = memoryInfo.CurrentUsage;
                res.logicalMaximumUsage = memoryInfo.Budget;
            }
        }

        return res;
    }

    void GAPI_DX12::InitializeImGUI(const ImGUIContext& imGUIInfo)
    {
        if (imGUIInfo.context == nullptr)
        {
            return;
        }

        CUBE_LOG(Info, DX12, "ImGUI context is set in the initialize info. Initialize ImGUI.");

        mImGUIContext = imGUIInfo;

        gImGUISRVHeap.Initialize(mMainDevice->GetDevice());
        CHECK_HR(mMainDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mMainDevice->GetCommandListManager().GetCurrentAllocator(), nullptr, IID_PPV_ARGS(&mImGUIRenderCommandList)));
        mImGUIRenderCommandList->Close();

        ImGui::SetCurrentContext((ImGuiContext*)(imGUIInfo.context));
        ImGui::SetAllocatorFunctions(
            (ImGuiMemAllocFunc)(imGUIInfo.allocFunc),
            (ImGuiMemFreeFunc)(imGUIInfo.freeFunc),
            (void*)(imGUIInfo.userData)
        );

        ImGui_ImplWin32_Init(platform::WindowsPlatform::GetWindow());
        platform::WindowsPlatform::SetImGUIWndProcFunction(ImGui_ImplWin32_WndProcHandler);

        ImGui_ImplDX12_InitInfo initInfo = {};
        initInfo.Device = mMainDevice->GetDevice();
        initInfo.CommandQueue = mMainDevice->GetQueueManager().GetMainQueue();
        initInfo.NumFramesInFlight = 2;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.SrvDescriptorHeap = gImGUISRVHeap.mHeap.Get();
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* outCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* outGPUDescriptor)
        {
            gImGUISRVHeap.Allocate(outCPUDescriptor, outGPUDescriptor);
        };
        initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor)
        {
            gImGUISRVHeap.Free(CPUDescriptor, GPUDescriptor);
        };
        ImGui_ImplDX12_Init(&initInfo);

        // Start new frame before starting ImGUI loop in Engine class
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
    }

    void GAPI_DX12::ShutdownImGUI(const ImGUIContext& imGUIInfo)
    {
        if (mImGUIContext.context == nullptr)
        {
            return;
        }
        CUBE_LOG(Info, DX12, "Shtudown ImGUI.");

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        platform::WindowsPlatform::SetImGUIWndProcFunction(nullptr);

        mImGUIRenderCommandList = nullptr;
        gImGUISRVHeap.Shutdown();
    }
} // namespace cube
