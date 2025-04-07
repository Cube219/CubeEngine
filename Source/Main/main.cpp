#include "Engine.h"

#ifdef CUBE_PLATFORM_WINDOWS

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    Engine::Initialize(argc, argv);
    Engine::Shutdown();

    return 0;
}

#elif CUBE_PLATFORM_MACOS

int main(int argc, const char* argv[])
{
    cube::Engine::Initialize(argc, argv, true);
    // Remain logic will not be executed. So Engine::Shutdown() will be executed in Engine::OnClosing().
    return 0;
}

#endif
