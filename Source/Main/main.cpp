#include "Engine.h"
#include "GAPI.h"
#include "Platform.h"

#ifdef CUBE_PLATFORM_WINDOWS

#include <Windows.h>

// D3D12 Agility SDK parameters
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 616; }
extern "C" { __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\"; }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    using namespace cube;

    // TODO: argc / argv
    Engine::EngineInitializeInfo initInfo = {
        .argc = 0,
        .argv = nullptr,
        .gapi = cube::GAPIName::DX12,
    };
    Engine::Initialize(initInfo);
    Engine::StartLoop();
    Engine::Shutdown();

    return 0;
}

#elif CUBE_PLATFORM_MACOS

int main(int argc, const char* argv[])
{
    using namespace cube;

    Engine::EngineInitializeInfo initInfo = {
        .argc = argc,
        .argv = argv,
        .gapi = cube::GAPIName::Metal,
        .runInitializeAndShutdownInLoopFunction = true
    };
    platform::Platform::SetEngineInitializeFunction([initInfo](){
        Engine::EngineInitializeInfo info2 = initInfo;        
        info2.runInitializeAndShutdownInLoopFunction = false;
        Engine::Initialize(info2);
    });
    platform::Platform::SetEngineShutdownFunction(&Engine::Shutdown);

    Engine::Initialize(initInfo);
    // Remain logic will not be executed. Instead they will be executed in platform loop function.
    // Engine::StartLoop();
    // Engine::Shutdown();
    return 0;
}

#endif
