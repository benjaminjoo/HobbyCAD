#include <memory>


#include "headers/Application.h"


int APIENTRY wWinMain(
    _In_        HINSTANCE   hInstance,
    _In_opt_    HINSTANCE   hPrevInstance,
    _In_        LPWSTR      lpCmdLine,
    _In_        int         nCmdShow
)
{
    auto App = std::make_shared<Application>(hInstance);

    if (!App->Initialise())
    {
        return 1;
    }

    App->Run();

    return 0;
}