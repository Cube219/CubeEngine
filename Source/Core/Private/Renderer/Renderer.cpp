#include "Renderer.h"

#include "Checker.h"
#include "imgui.h"

#include "Platform.h"
#include "GAPI_Viewport.h"

namespace cube
{
    void Renderer::Initialize(GAPIName gAPIName, const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(LogType::Info, Renderer, "Initialize renderer. (GAPI: {})", GAPINameToString(gAPIName));

        // GAPI init
        const Character* dLibName = CUBE_T("");
        switch (gAPIName)
        {
        case GAPIName::DX12:
            dLibName = CUBE_T("CE-GAPI_DX12");
            break;

        case GAPIName::Unknown:
        default:
            CHECK_FORMAT(false, "Invalid GAPIName: {}", (int)gAPIName);
            break;
        }

        mGAPI_DLib = platform::Platform::LoadDLib(dLibName);
        CHECK_FORMAT(mGAPI_DLib, "Cannot load GAPI library! (Name: {})", dLibName);

        using CreateGAPIFunction = GAPI* (*)();
        auto createGAPIFunc = reinterpret_cast<CreateGAPIFunction>(mGAPI_DLib->GetFunction(CUBE_T("CreateGAPI")));
        mGAPI = SharedPtr<GAPI>(createGAPIFunc());

        mGAPI->Initialize({
            .enableDebugLayer = true,
            .imGUI = imGUIContext
        });

        mViewport = mGAPI->CreateViewport({
            .width = platform::Platform::GetWindowWidth(),
            .height = platform::Platform::GetWindowHeight(),
            .vsync = true,
            .backbufferCount = 2
        });
    }

    void Renderer::Shutdown(const ImGUIContext& imGUIContext)
    {
        CUBE_LOG(LogType::Info, Renderer, "Shutdown renderer.");

        mViewport = nullptr;

        mGAPI->Shutdown(imGUIContext);
        mGAPI = nullptr;
        mGAPI_DLib = nullptr;
    }

    void Renderer::Render()
    {
        mGAPI->OnBeforeRender();
        // Render
        mGAPI->OnAfterRender();

        ImGui::Render();

        mViewport->AcquireNextImage();
        mGAPI->OnBeforePresent(mViewport.get());
        mViewport->Present();
        mGAPI->OnAfterPresent();
    }

    void Renderer::OnResize(Uint32 width, Uint32 height)
    {
        if (mViewport)
        {
            mViewport->Resize(width, height);
        }
    }
} // namespace cube
