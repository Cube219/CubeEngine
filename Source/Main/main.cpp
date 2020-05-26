#include "Engine/Engine.h"

namespace cube
{
    void MainImpl()
    {
        EngineInitOption initOption;
        initOption.title = CUBE_T("Test Title");
        initOption.windowWidth = 1040;
        initOption.windowHeight = 807;
        initOption.isWindowMode = true;

        Engine::Initialize(initOption);

        Engine::Run();

        Engine::Shutdown();
    }
}

#ifdef _WIN32
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    cube::MainImpl();

    return 0;
}
#endif // _WIN32
