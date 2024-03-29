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
    /*
    memset(&m_Image, 0, sizeof(texture_t));

    //if (!BMPManager::ReadBitMapData("vw_1300.bmp", m_Image))
    //if (!BMPManager::ReadBitMapData("lakotelep_2500x2500.bmp", m_Image))
    if (!BMPManager::ReadBitMapData("szamuely_2500x2500.bmp", m_Image))
    //if (!BMPManager::ReadBitMapData("chess.bmp", m_Image))
    //if (!BMPManager::ReadBitMapData("hms_daedalus.bmp", m_Image))
    {
        MessageBox(
            m_hMainWnd,
            "Could not open BMP file.",
            "Bitmap error",
            MB_OK
        );
    }
    */

    /*
    float* intensity_buffer = nullptr;
    uint8_t* threshold_buffer = nullptr;

    if (!Image::GetIntensityAndThresholdBuffer(
            m_Image,
            intensity_buffer,
            threshold_buffer,
            4,
            true
        )
    )
    {
        MessageBox(
            m_hMainWnd,
            "Failed to get intensity and threshold values.",
            "Bitmap error",
            MB_OK
        );
    }

    if (!Image::FindVerticesInsideSquares(
        m_Image,
        m_Points,
        intensity_buffer,
        threshold_buffer)
    )
    {
        MessageBox(
            m_hMainWnd,
            "Failed to calculate pixel intensities.",
            "Bitmap error",
            MB_OK
        );
    }
    */

    /*
    if (!Image::FindVerticesInsideCircles(
        m_Image,
        m_Points,
        intensity_buffer,
        threshold_buffer)
    )
    {
        MessageBox(
            m_hMainWnd,
            "Failed to calculate pixel intensities.",
            "Bitmap error",
            MB_OK
        );
    }
    */
    /*
    std::string vertices_msg = std::to_string(m_Points.size()) + " vertices found!";
    MessageBox(
        m_hMainWnd,
        vertices_msg.c_str(),
        "Corner detection",
        MB_OK
    );
    */
    /*
    std::ofstream vertex_output("vertices.txt");
    if (vertex_output.is_open())
    {
        for (const auto& p : m_Points)
        {
            vertex_output << std::to_string(p.x) << ", " << std::to_string(p.y) << "\n";
        }

        vertex_output.close();
    }
    */

    /*
    for (int i = 0; i < m_Image.w * m_Image.h; i++)
    {
        uint8_t s = threshold_buffer[i];
        m_Image.buffer[i] = (s << 16) | (s << 8) | s;
    }
    */
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

        window.second->InitialiseFrameLinesOriginal(&m_FrameLinesOriginal);
        window.second->InitialiseFrameLinesTransformed(&m_FrameLinesTransformed);
        window.second->InitialiseReferenceLines(&m_ReferenceLines);
        window.second->InitialiseGridLines(&m_GridLines);
        window.second->InitialiseSketchLinesOriginal(&m_SketchLinesOriginal);
        window.second->InitialiseSketchLinesTransformed(&m_SketchLinesTransformed);
        window.second->InitialisePoints(&m_Points);
        window.second->InitialiseImage(&m_Image);

        window.second->InitialiseRefreshHandler(
            std::bind(&Application::RefreshAll, this)
        );

        //window.second->CreateSketch();

        count++;
    }

    CreateButton("selection_tool_button",   "Selection",        0,   0, 80, 20, (HMENU)ACTIVATE_SELECTION_TOOL);
    CreateButton("line_tool_button",        "Line",             0,  20, 80, 20, (HMENU)ACTIVATE_LINE_TOOL);
    CreateButton("polyline_tool_button",    "Polyline",         0,  40, 80, 20, (HMENU)ACTIVATE_POLYLINE_TOOL);
    CreateButton("polygon_tool_button",     "Polygon",          0,  60, 80, 20, (HMENU)ACTIVATE_POLYGON_TOOL);
    CreateButton("frame_tool_button",       "Ref. Frame",       0,  80, 80, 20, (HMENU)ACTIVATE_FRAME_TOOL);
    CreateButton("open_image_button",       "Open Image",       0, 100, 80, 20, (HMENU)OPEN_IMAGE_FILE);
    CreateButton("save_image_button",       "Save Image",       0, 120, 80, 20, (HMENU)SAVE_IMAGE_FILE);

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
                case ACTIVATE_FRAME_TOOL:
                {
                    g_LeftClicksIn = 0;
                    g_ActiveTool = ToolBox::Frame;
                }
                break;
                case OPEN_IMAGE_FILE:
                {
                    OPENFILENAME ofn;
                    CHAR filename[260] = { 0 };

                    ZeroMemory(&ofn, sizeof(ofn));

                    ofn.lStructSize         = sizeof(ofn);
                    ofn.hwndOwner           = NULL;
                    ofn.lpstrFile           = filename;
                    ofn.nMaxFile            = sizeof(filename);
                    ofn.lpstrFilter         = "Bitmap Files (*.BMP)\0*.bmp\0";
                    ofn.nFilterIndex        = 1;
                    ofn.lpstrFileTitle      = NULL;
                    ofn.nMaxFileTitle       = 0;
                    ofn.lpstrInitialDir     = NULL;
                    ofn.Flags               = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileNameA(&ofn))
                    {
                        memset(&m_Image, 0, sizeof(texture_t));

                        if (!BMPManager::ReadBitMapData(filename, m_Image))
                        {
                            MessageBox(
                                m_hMainWnd,
                                "Could not open BMP file.",
                                "Bitmap file error",
                                MB_OK
                            );
                        }
                        else
                        {
                            for (auto& window : m_GraphWindows)
                            {
                                window.second->CreateSketch();
                            }
                        }

                        RefreshAll();
                    }
                }
                break;
                case SAVE_IMAGE_FILE:
                {
                    OPENFILENAMEA ofn;
                    CHAR output_filename[260] = { 0 };
                    CHAR szTitle[] = "Save Image As";

                    ZeroMemory(&ofn, sizeof(ofn));

                    ofn.lStructSize     = sizeof(ofn);
                    ofn.lpstrFile       = output_filename;
                    ofn.nMaxFile        = sizeof(output_filename);
                    ofn.lpstrFilter     = "BMP Files\0*.bmp\0All Files\0*.*\0";
                    ofn.nFilterIndex    = 1;
                    ofn.lpstrDefExt     = "bmp";
                    ofn.lpstrTitle      = szTitle;
                    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                    if (GetSaveFileNameA(&ofn) == TRUE)
                    {
                        MessageBox(
                            m_hMainWnd,
                            output_filename,
                            "Save As",
                            MB_OK
                        );

                        try
                        {
                            texture_t output_image = { 0 };

                            for (auto& window : m_GraphWindows)
                            {
                                window.second->SaveTexturedQuadrangle(output_filename, output_image);
                                break;
                            }

                            /*
                            texture_t t;
                            t.w = 3200;
                            t.h = 1600;
                            t.buffer = new uint32_t[t.w * t.h];
                            for (int i = 0; i < t.w * t.h; i++)
                            {
                                t.buffer[i] = 0x007F7FFF;
                            }
                            if (!BMPManager::WriteBitMapData(output_filename, t))
                            {
                                MessageBox(
                                    m_hMainWnd,
                                    "Could not save BMP file.",
                                    "Bitmap file error",
                                    MB_OK
                                );
                            }
                            */
                            /*
                            if (!BMPManager::WriteBitMapData(output_filename, output_image))
                            {
                                MessageBox(
                                    m_hMainWnd,
                                    "Could not save BMP file.",
                                    "Bitmap file error",
                                    MB_OK
                                );
                            }
                            */
                        }
                        catch (const std::exception& e)
                        {
                            MessageBox(
                                m_hMainWnd,
                                e.what(),
                                "Save As",
                                MB_OK
                            );
                        }
                    }
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