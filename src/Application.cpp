#include "headers/Application.h"


Application::Application(HINSTANCE hInstance)
    :
    m_hInstance(hInstance),
    m_AppWndClass({ 0 }),
    m_hDc(NULL),
    m_Ps({ 0 }),
    m_WindowFont(NULL),
    m_hMainWnd(NULL)
{

}


Application::~Application()
{

}


bool Application::Initialise()
{
    m_WindowFont = CreateFontA(
        16, 0,
        0, 0, 0,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Segoe UI"
    );

    const char MAIN_WINDOW_CLASS_NAME[] = "Main Window Class";

    m_AppWndClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    m_AppWndClass.lpfnWndProc = Application::MainWindowProc;
    m_AppWndClass.hInstance = m_hInstance;
    m_AppWndClass.hCursor = LoadCursor(m_hInstance, IDC_ARROW);
    m_AppWndClass.style = CS_HREDRAW | CS_VREDRAW;
    m_AppWndClass.hbrBackground = NULL;

    RegisterClassA(&m_AppWndClass);

    m_hMainWnd = CreateWindowExA(
        0,
        MAIN_WINDOW_CLASS_NAME,
        "HobbyCAD",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        m_hInstance,
        this
    );

    if (m_hMainWnd == NULL)
    {
        return false;
    }

    int a = 1;
    int b = 1;
    int g = 0;
    int w = 1350;
    int h = 750;

    /*
    int a = 2;
    int b = 2;
    int g = 5;
    int w = 700;
    int h = 350;
    */

    for (int i = 0; i < (a * b); i++)
    {
        std::string window_name = "graph_0" + std::to_string(i);
        m_GraphWindows.insert({ window_name, std::make_shared<GraphWindow>() });
    }
    
    int count = 0;
    for (auto& window : m_GraphWindows)
    {
        int x = g + (count % a) * (g + w);
        int y = g + (count / a) * (g + h);
        if (!window.second->Create(
            "Graph Window",
            WS_CHILDWINDOW | WS_VISIBLE,
            0,
            x + 80,
            y,
            w,
            h,
            m_hMainWnd
        )
        )
        {
            return false;
        }

        window.second->InitialiseReferenceLines(&m_ReferenceLines);
        window.second->InitialiseGridLines(&m_GridLines);
        window.second->InitialiseSketchLinesOriginal(&m_SketchLinesOriginal);
        window.second->InitialiseSketchLinesTransformed(&m_SketchLinesTransformed);
        window.second->InitialisePoints(&m_Points);

        window.second->InitialiseRefreshHandler(
            std::bind(&Application::RefreshAll, this)
        );

        window.second->CreateSketch();

        count++;
    }

    CreateButton("selection_tool_button",   "Selection",    0,  0, 80, 20, (HMENU)ACTIVATE_SELECTION_TOOL);
    CreateButton("line_tool_button",        "Line",         0, 20, 80, 20, (HMENU)ACTIVATE_LINE_TOOL);
    CreateButton("polyline_tool_button",    "Polyline",     0, 40, 80, 20, (HMENU)ACTIVATE_POLYLINE_TOOL);
    CreateButton("polygon_tool_button",     "Polygon",      0, 60, 80, 20, (HMENU)ACTIVATE_POLYGON_TOOL);

    ShowWindow(m_hMainWnd, SW_MAXIMIZE);

    return true;
}


void Application::Run()
{
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


void Application::RefreshAll()
{
    for (auto& window : m_GraphWindows)
    {
        window.second->UpdateView();
    }
}


LRESULT CALLBACK Application::MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Application* pThis = NULL;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (Application*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

        pThis->m_hMainWnd = hwnd;
    }
    else
    {
        pThis = (Application*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
    {
        return pThis->HandleMainWindowMessage(hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


LRESULT CALLBACK Application::HandleMainWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        return 0;
        case WM_CREATE:
        {
            PaintMainWindow(hwnd);
        }
        return 0;
        case WM_PAINT:
        {
            PaintMainWindow(hwnd);
        }
        return 0;
        case WM_SIZE:
        {
            PaintMainWindow(hwnd);
        }
        return 0;
        case WM_SETCURSOR:
        {
            LoadOwnCursor();
            //SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        return 0;
        case WM_COMMAND:
        {
            WORD lo_w = LOWORD(wParam);
            WORD hi_w = HIWORD(wParam);

            switch (lo_w)
            {
                case ACTIVATE_SELECTION_TOOL:
                {
                    g_LeftClicksIn = 0;
                    g_ActiveTool = ToolBox::Selection;
                }
                break;
                case ACTIVATE_LINE_TOOL:
                {
                    g_LeftClicksIn = 0;
                    g_ActiveTool = ToolBox::Line;
                }
                break;
                case ACTIVATE_POLYLINE_TOOL:
                {
                    g_LeftClicksIn = 0;
                    g_ActiveTool = ToolBox::Polyline;
                }
                break;
                case ACTIVATE_POLYGON_TOOL:
                {
                    g_LeftClicksIn = 0;
                    g_ActiveTool = ToolBox::Polygon;
                }
                break;
            }
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void Application::PaintMainWindow(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW));

    RECT rect;
    GetClientRect(hwnd, &rect);
    EndPaint(hwnd, &ps);
}


void Application::CreateButton(
    const std::string& name,
    const std::string& text,
    int x,
    int y,
    int w,
    int h,
    HMENU command
)
{
    HWND temp_button = CreateWindowA(
        "BUTTON",
        text.c_str(),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD,
        x,
        y,
        w,
        h,
        m_hMainWnd,
        command,
        m_hInstance,
        NULL
    );

    SendMessage(temp_button, WM_SETFONT, WPARAM(m_WindowFont), TRUE);
    
    m_Buttons.insert({ name, temp_button });
}


void Application::LoadOwnCursor()
{
    BYTE ANDmaskCursor[] = 
    { 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 1 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 2 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 3 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 4 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 5 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 6 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 7 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 8 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 9 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 10 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 11 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 12 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 13 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 14 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 15 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 16 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 17 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 18 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 19 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 20 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 21 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 22 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 23 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 24 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 25 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 26 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 27 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 28 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 29 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 30 
        0xFF, 0xFF, 0xFF, 0xFF,   // line 31 
        0xFF, 0xFF, 0xFF, 0xFF    // line 32 
    };
    
    BYTE XORmaskCursor[] = 
    { 
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x07, 0x70, 0x00,
        0x00, 0x04, 0x10, 0x00,
        0x7f, 0xfc, 0x1f, 0xff,
        0x00, 0x00, 0x00, 0x00,
        0x7f, 0xfc, 0x1f, 0xff,
        0x00, 0x04, 0x10, 0x00,
        0x00, 0x07, 0x70, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00,
        0x00, 0x01, 0x40, 0x00
    };

    HCURSOR own_cursor = CreateCursor(
        m_hInstance,
        16,                // horizontal position of hot spot 
        16,                // vertical position of hot spot 
        32,                // cursor width 
        32,                // cursor height 
        ANDmaskCursor,
        XORmaskCursor
    );

    SetCursor(own_cursor);
}