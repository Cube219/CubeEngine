#include "Engine.h"

namespace cube
{
    void MainImpl()
    {
        Engine::Initialize();

        Engine::Shutdown();
    }
} // namespace cube

#ifdef CUBE_PLATFORM_WINDOWS

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    cube::MainImpl();

    return 0;
}

#elif CUBE_PLATFORM_MACOS

int main(int argc, const char * argv[])
{
    cube::MainImpl();

    return 0;
}

#endif
