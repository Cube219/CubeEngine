namespace cube
{
    void MainImpl()
    {}
}

#ifdef _WIN32
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    cube::MainImpl();

    return 0;
}
#endif // _WIN32
