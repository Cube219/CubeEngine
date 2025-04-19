#include "Engine.h"
#include "GAPI.h"

#ifdef CUBE_PLATFORM_WINDOWS

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // TODO: argc / argv
    cube::Engine::EngineInitializeInfo initInfo = {
        .argc = 0,
        .argv = nullptr,
        .gapi = cube::GAPIName::DX12,
        .runShutdownInOnClosingFunc = false
    };
    cube::Engine::Initialize(initInfo);
    cube::Engine::Shutdown();

    return 0;
}

#elif CUBE_PLATFORM_MACOS

int main(int argc, const char* argv[])
{
    cube::Engine::EngineInitializeInfo initInfo = {
        .argc = argc,
        .argv = argv,
        .gapi = cube::GAPIName::Metal,
        .drawImGUI = false, // TODO: Enable while developing GAPI_Metal
        .runShutdownInOnClosingFunc = true
    };
    cube::Engine::Initialize(initInfo);
    // Remain logic will not be executed. So Engine::Shutdown() will be executed in Engine::OnClosing().
    return 0;
}

#endif
