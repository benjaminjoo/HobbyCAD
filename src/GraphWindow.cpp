#include "GraphWindow.h"


GraphWindow::GraphWindow()
	:
	m_hWnd(NULL),
	m_hParentWnd(NULL),
	m_Canvas(nullptr),
    m_Printer(nullptr),
    m_ViewScale(1.0f),
    m_LeftClickInProgress(false),
    m_MouseDrag(false),
    m_MouseMove(false),
    m_MouseMeasure(false),
    m_HotspotFound(false),
    m_MousePosition({ 0, 0 }),
    m_DragOrigin({ 0, 0 }),
    m_MeasureOrigin({ 0, 0 }),
    m_HotspotPosition({ 0.0f, 0.0f }),
    m_MouseBeforeZoom({ 0.0f, 0.0f }),
    m_MouseAfterZoom({ 0.0f, 0.0f }),
    m_Centre({ 0.0f, 0.0f }),
    m_BackgroundColour(0x0000003F),
    m_CanvasX(0),
    m_CanvasY(0),
    m_CanvasW(0),
    m_CanvasH(0),
    m_FrameLinesOriginal(nullptr),
    m_FrameLinesTransformed(nullptr),
    m_ReferenceLines(nullptr),
    m_GridLines(nullptr),
    m_SketchLinesOriginal(nullptr),
    m_SketchLinesTransformed(nullptr),
    m_Points(nullptr),
    m_Image(nullptr),
    m_RenderImage(true),
    m_Homography(mat3x3_t(1.0f)),
    m_InverseHomography(mat3x3_t(1.0f)),
    m_HandleRefreshAll(nullptr)
{

}


BOOL GraphWindow::Create(
	LPCSTR		lpWindowName,
	DWORD		dwStyle,
	DWORD		dwExStyle,
	int			x,
	int			y,
	int			w,
	int			h,
	HWND		hWndParent,
	HMENU		hMenu
)
{
    m_CanvasX = x;
    m_CanvasY = y;
    m_CanvasW = w;
    m_CanvasH = h;

    m_Canvas = std::make_shared<Canvas>(w, h);

    m_Printer = std::make_shared<TextRenderer>(
        m_Canvas->GetFrameBuffer(),
        m_Canvas->GetW(),
        m_Canvas->GetH()
    );

    m_hParentWnd = hWndParent;

    WNDCLASSA wc = { 0 };

    wc.lpfnWndProc = GraphWindow::WindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "Graph Window Class";
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassA(&wc);

    m_hWnd = CreateWindowExA(
        dwExStyle,
        "Graph Window Class",
        lpWindowName,
        dwStyle,
        x,
        y,
        w,
        h,
        hWndParent,
        hMenu,
        GetModuleHandle(NULL),
        this
    );
    
    SetFocus(m_hWnd);

    return (m_hWnd ? TRUE : FALSE);
}


void GraphWindow::SetBackgroundColour(uint32_t c)
{
    m_BackgroundColour = c;
}


void GraphWindow::InitialiseFrameLinesOriginal(std::vector<line2_t>* frame_lines)
{
    m_FrameLinesOriginal = frame_lines;
}


void GraphWindow::InitialiseFrameLinesTransformed(std::vector<line2_t>* frame_lines)
{
    m_FrameLinesTransformed = frame_lines;
}


void GraphWindow::InitialiseReferenceLines(std::vector<line2_t>* ref_lines)
{
    m_ReferenceLines = ref_lines;
}


void GraphWindow::InitialiseGridLines(std::vector<line2_t>* grid_lines)
{
    m_GridLines = grid_lines;
}


void GraphWindow::InitialiseSketchLinesOriginal(std::vector<line2_t>* sketch_lines)
{
    m_SketchLinesOriginal = sketch_lines;
}


void GraphWindow::InitialiseSketchLinesTransformed(std::vector<line2_t>* sketch_lines)
{
    m_SketchLinesTransformed = sketch_lines;
}


void GraphWindow::InitialisePoints(std::vector<vect2_t>* points)
{
    m_Points = points;
}


void GraphWindow::InitialiseImage(texture_t* image)
{
    m_Image = image;
}


void GraphWindow::InitialiseRefreshHandler(std::function<void()> h_refresh)
{
    m_HandleRefreshAll = h_refresh;
}


void GraphWindow::Render()
{
    if (m_Canvas == nullptr)
    {
        return;
    }

    Clear();

    DrawOrigin();

    DrawSketch();

    DrawContextSpecificStuff();

    DrawCrosshair();

    DrawHotspot();

    RenderText();

    Paint();
}


LRESULT CALLBACK GraphWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    GraphWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (GraphWindow*)pCreate->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

        pThis->m_hWnd = hwnd;
    }
    else
    {
        pThis = (GraphWindow*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    }
    if (pThis)
    {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}


LRESULT GraphWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            DestroyWindow(m_hWnd);
        }
        break;
        case WM_CREATE:
        {
            //Render();
        }
        break;
        case WM_PAINT:
        {
            Render();
        }
        break;
        case WM_SIZE:
        {
            RECT rect;
            GetClientRect(m_hWnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            m_Canvas->Resize(w, h);
            if (m_Printer)
            {
                m_Printer->Resize(w, h);
                m_Printer->SetFrameBuffer(m_Canvas->GetFrameBuffer());
            }
            Render();
        }
        break;
        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                if (g_LeftClicksIn > 0)
                {
                    g_LeftClicksIn = 0;
                }
                else
                {
                    g_ActiveTool = ToolBox::Selection;
                    m_VerticesBeingMoved.clear();
                }
                for (auto& l : *m_GridLines)
                {
                    l.selected = false;
                }
                m_TempFramePoints.clear();
                Render();
            }
            if (wParam == VK_SPACE)
            {
                UndoTransformation();
                Render();
            }
            if (wParam == 0x49) // 'I'
            {
                m_RenderImage = !m_RenderImage;
                Render();
            }
            if (wParam == 0x4C) // 'L'
            {
                g_LeftClicksIn = 0;
                g_ActiveTool = ToolBox::Line;
            }
        }
        break;
        case WM_MOUSEWHEEL:
        {
            int16_t scroll = HIWORD(wParam);
            if (scroll > 0)
            {
                if (m_ViewScale <= 100.0f)
                {
                    m_ViewScale *= 1.1f;
                }
            }
            if (scroll < 0)
            {
                if (m_ViewScale >= 0.001f)
                {
                    m_ViewScale *= (1.0f / 1.1f);
                }
            }
            CompensateZoom();
            Render();
        }
        break;
        case WM_LBUTTONDOWN:
        {
            SetFocus(m_hWnd);
            g_LeftClicksIn++;

            int x = lParam & 0xFFFF;
            int y = (lParam >> 16) & 0xFFFF;

            SetMousePosition(x, y);

            if (g_ActiveTool == ToolBox::Selection)
            {
                m_MoveOrigin.x = x;
                m_MoveOrigin.y = m_Canvas->GetH() - y;

                vect2_t mouse_world = ScreenToWorld(m_MoveOrigin);

                m_TempMoveStartPoint = ScreenToWorld(m_MoveOrigin);
                m_TempMoveLine.S = m_TempMoveStartPoint;

                if (m_ReferenceLines)
                {
                    for (auto& ref_line : *m_ReferenceLines)
                    {
                        int dist_squared_s = PixelDistanceSquared(m_MousePosition, WorldToScreen(ref_line.S));
                        int dist_squared_e = PixelDistanceSquared(m_MousePosition, WorldToScreen(ref_line.E));

                        if (dist_squared_s < SNAP_RANGE)
                        {
                            m_VerticesBeingMoved.push_back(&(ref_line.S));
                        }
                        if (dist_squared_e < SNAP_RANGE)
                        {
                            m_VerticesBeingMoved.push_back(&(ref_line.E));
                        }
                    }
                }

                m_MouseMove = true;
            }
            else if (g_ActiveTool == ToolBox::Line)
            {
                if (g_LeftClicksIn == 1)
                {
                    if (m_HotspotFound)
                    {
                        m_TempStartPoint = m_HotspotPosition;
                    }
                    else
                    {
                        m_TempStartPoint = ScreenToWorld(m_MousePosition);
                    }
                    m_TempLine.S = m_TempStartPoint;
                }
                else if (g_LeftClicksIn == 2)
                {
                    if (m_HotspotFound)
                    {
                        m_TempEndPoint = m_HotspotPosition;
                    }
                    else 
                    {
                        m_TempEndPoint = ScreenToWorld(m_MousePosition);

                        if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
                        {
                            float dx = std::fabs(m_TempEndPoint.x - m_TempStartPoint.x);
                            float dy = std::fabs(m_TempEndPoint.y - m_TempStartPoint.y);
                            if (dx > dy)
                            {
                                m_TempEndPoint.y = m_TempStartPoint.y;
                            }
                            else
                            {
                                m_TempEndPoint.x = m_TempStartPoint.x;
                            }
                        }
                    }

                    line2_t temp_line = { m_TempStartPoint, m_TempEndPoint, 0x009F9F9F };
            
                    m_SketchLinesOriginal->push_back(temp_line);

                    if (m_HandleRefreshAll)
                    {
                        m_HandleRefreshAll();
                    }
                    else
                    {
                        UpdateView();
                    }

                    g_LeftClicksIn = 0;
                }
            }
            else if (g_ActiveTool == ToolBox::Frame)
            {
                if (m_TempFramePoints.size() == 0)
                {
                    m_ReferenceLines->clear();
                }

                if (m_TempFramePoints.size() < 4)
                {
                    m_TempStartPoint = ScreenToWorld(m_MousePosition);

                    m_TempFramePoints.push_back(m_TempStartPoint);

                    auto n = m_TempFramePoints.size();
                    if (n > 1)
                    {
                        m_SketchLinesOriginal->push_back({ m_TempFramePoints.at(n - 2), m_TempFramePoints.at(n - 1), 0x00FF7F00 });
                    }
                }

                if (m_TempFramePoints.size() == 4)
                {
                    m_OriginalFramePoints.clear();

                    for (const auto& p : m_TempFramePoints)
                    {
                        m_OriginalFramePoints.push_back(p);
                    }

                    m_TempFramePoints.clear();

                    m_RA = m_OriginalFramePoints.at(0);
                    m_RB = m_OriginalFramePoints.at(1);
                    m_RC = m_OriginalFramePoints.at(2);
                    m_RD = m_OriginalFramePoints.at(3);

                    m_SketchLinesOriginal->clear();

                    AddReference();
                }
            }

            Render();
        }
        break;
        case WM_MBUTTONDOWN:
        {
            int x = lParam & 0xFFFF;
            int y = (lParam >> 16) & 0xFFFF;

            m_DragOrigin.x = x;
            m_DragOrigin.y = m_Canvas->GetH() - y;

            m_MouseDrag = true;
        }
        break;
        case WM_LBUTTONUP:
        {
            if (g_ActiveTool == ToolBox::Selection)
            {
                m_MouseMove = false;
                m_VerticesBeingMoved.clear();

                if (m_HandleRefreshAll)
                {
                    m_HandleRefreshAll();
                }
                else
                {
                    UpdateView();
                }
            }
            else if (g_ActiveTool == ToolBox::Line)
            {

            }
            else if (g_ActiveTool == ToolBox::Polyline)
            {
                
            }
            else if (g_ActiveTool == ToolBox::Polygon)
            {
                
            }

            UpdateSketch();

            Render();
        }
        break;
        case WM_MBUTTONUP:
        m_MouseDrag = false;
        break;
        case WM_MOUSEMOVE:
        {
            int x = lParam & 0xFFFF;
            int y = (lParam >> 16) & 0xFFFF;

            SetMousePosition(x, y);

            if (m_ReferenceLines)
            {
                for (auto& ref_line : *m_ReferenceLines)
                {
                    int dist_squared_s = PixelDistanceSquared(m_MousePosition, WorldToScreen(ref_line.S));
                    int dist_squared_e = PixelDistanceSquared(m_MousePosition, WorldToScreen(ref_line.E));

                    if (dist_squared_s < SNAP_RANGE)
                    {
                        m_HotspotPosition = ref_line.S;
                        m_HotspotFound = true;
                        Render();
                        break;
                    }
                    else if (dist_squared_e < SNAP_RANGE)
                    {
                        m_HotspotPosition = ref_line.E;
                        m_HotspotFound = true;
                        Render();
                        break;
                    }
                    else
                    {
                        m_HotspotFound = false;
                    }
                }
            }

            m_TempEndPoint = ScreenToWorld(m_MousePosition);

            m_TempLine.E = m_TempEndPoint;
            m_TempMoveLine.E = m_TempEndPoint;

            if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
            {
                float dx = std::fabs(m_TempLine.E.x - m_TempLine.S.x);
                float dy = std::fabs(m_TempLine.E.y - m_TempLine.S.y);
                if (dx > dy)
                {
                    m_TempLine.E.y = m_TempLine.S.y;
                }
                else
                {
                    m_TempLine.E.x = m_TempLine.S.x;
                }
            }

            if (m_MouseDrag)
            {
                Drag();
                Render();
            }
            if (m_MouseMove)
            {
                Move();
                Render();
            }

            UpdateSketch();

            if (g_LeftClicksIn == 1)
            {
                Render();
            }

            if ((m_TempFramePoints.size() > 0) && (m_TempFramePoints.size() < 4))
            {
                Render();
            }
        }
        break;
    }

    return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}


void GraphWindow::Clear()
{
    m_Canvas->Fill(m_BackgroundColour);
}


void GraphWindow::DrawOrigin()
{
    m_Canvas->DrawLine(WorldToScreen({ 0.0f, 0.0f }), WorldToScreen({ 12500.0f, 0.0f }), 0x00FF0000);
    m_Canvas->DrawLine(WorldToScreen({ 0.0f, 0.0f }), WorldToScreen({ 0.0f, 12500.0f }), 0x0000FF00);
}


void GraphWindow::DrawSketch()
{
    if (m_RenderImage)
    {
        auto A = WorldToScreen(m_A);
        auto B = WorldToScreen(m_B);
        auto C = WorldToScreen(m_C);
        auto D = WorldToScreen(m_D);

        if (m_FrameLinesTransformed && (m_FrameLinesTransformed->size() >= 4))
        {
            try
            {
                A = WorldToScreen(m_FrameLinesTransformed->at(0).S);
                B = WorldToScreen(m_FrameLinesTransformed->at(1).S);
                C = WorldToScreen(m_FrameLinesTransformed->at(2).S);
                D = WorldToScreen(m_FrameLinesTransformed->at(3).S);
            }
            catch (const std::exception& e)
            {
                MessageBox(
                    m_hWnd,
                    e.what(),
                    "DrawSketch()",
                    MB_OK
                );
            }

            A.u = 0.0f;     A.v = 0.0f;
            B.u = 1.0f;     B.v = 0.0f;
            C.u = 1.0f;     C.v = 1.0f;
            D.u = 0.0f;     D.v = 1.0f;

            if (m_Image)
            {
                DrawTexturedTriangle(A, B, C);
                DrawTexturedTriangle(C, D, A);
            }
        }
    }

    /*
    if (m_FrameLinesOriginal)
    {
        for (const auto& fl : *m_FrameLinesOriginal)
        {
            auto s = WorldToScreen(fl.S);
            auto e = WorldToScreen(fl.E);
            m_Canvas->DrawLine(s, e, fl.colour, 4);
        }
    }
    */

    if (m_FrameLinesTransformed)
    {
        for (const auto& fl : *m_FrameLinesTransformed)
        {
            auto s = WorldToScreen(fl.S);
            auto e = WorldToScreen(fl.E);
            m_Canvas->DrawLine(s, e, fl.colour, 4);
        }
    }

    if (m_ReferenceLines)
    {
        for (const auto& rl : *m_ReferenceLines)
        {
            auto s = WorldToScreen(rl.S);
            auto e = WorldToScreen(rl.E);
            m_Canvas->DrawLine(s, e, rl.colour, 4);
        }
    }

    if (m_GridLines)
    {
        for (const auto& gl : *m_GridLines)
        {
            auto s = WorldToScreen(gl.S);
            auto e = WorldToScreen(gl.E);
            m_Canvas->DrawLine(s, e, gl.colour);
        }
    }

    /*
    if (m_SketchLinesOriginal)
    {
        for (const auto& sl : *m_SketchLinesOriginal)
        {
            auto s = WorldToScreen(sl.S);
            auto e = WorldToScreen(sl.E);
            m_Canvas->DrawLine(s, e, sl.colour, 1);
        }
    }
    */

    if (m_SketchLinesTransformed)
    {
        for (const auto& sl : *m_SketchLinesTransformed)
        {
            auto s = WorldToScreen(sl.S);
            auto e = WorldToScreen(sl.E);
            m_Canvas->DrawLine(s, e, sl.colour, 1);
        }
    }

    if (m_Points)
    {
        for (const auto& P : *m_Points)
        {
            auto p = WorldToScreen(P);
            m_Canvas->DrawPoint(p.x, p.y, 0x00FF0000);
        }
    }
}


void GraphWindow::DrawContextSpecificStuff()
{
    if (g_ActiveTool == ToolBox::Selection)
    {
        if (m_MouseMove)
        {
            m_Canvas->DrawLine(WorldToScreen(m_TempMoveLine.S), WorldToScreen(m_TempMoveLine.E), 0x00FFFFFF);
        }
    }
    else if (g_ActiveTool == ToolBox::Line)
    {
        if (g_LeftClicksIn == 1)
        {
            m_Canvas->DrawLine(WorldToScreen(m_TempLine.S), WorldToScreen(m_TempLine.E), 0x00FFFFFF);
        }
    }
    else if (g_ActiveTool == ToolBox::Polyline)
    {
        
    }
    else if (g_ActiveTool == ToolBox::Polygon)
    {
        
    }
    else if (g_ActiveTool == ToolBox::Frame)
    {
        if ((m_TempFramePoints.size() > 0) && (m_TempFramePoints.size() < 4))
        {
            m_Canvas->DrawLine(WorldToScreen(m_TempStartPoint), WorldToScreen(m_TempEndPoint), 0x00FFFFFF);
        }
    }
}


void GraphWindow::DrawCrosshair()
{
    /*
    auto mouse_world = ScreenToWorld(m_MousePosition);
    auto mouse_screen = WorldToScreen(mouse_world);
    m_Canvas->DrawLine({ mouse_screen.x - 10, mouse_screen.y }, { mouse_screen.x + 10, mouse_screen.y }, 0x00FF0000);
    m_Canvas->DrawLine({ mouse_screen.x, mouse_screen.y - 10 }, { mouse_screen.x, mouse_screen.y + 10 }, 0x00FF0000);
    */
}


void GraphWindow::DrawHotspot()
{
    if (m_HotspotFound)
    {
        auto hotspot_p = WorldToScreen(m_HotspotPosition);
        m_Canvas->DrawAnchor(hotspot_p.x, hotspot_p.y, 0x00FFFFFF);
    }
}


void GraphWindow::RenderText()
{
    std::string zoom_msg = Utilities::PadFront(Utilities::TruncDecimals(m_ViewScale * 100.0f, 2) + "%", 8, ' ');
    m_Printer->DisplayString(zoom_msg.c_str(), 8, 8, 0x00FF7F00, 0x0000003F);

    int matrix_pos_x = 128;
    int matrix_pos_y = 8;
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            std::string element_str = Utilities::PadFront(
                Utilities::TruncDecimals(m_Homography.elements[j * 3 + i], 4),
                8,
                ' '
            );

            m_Printer->DisplayString(
                element_str.c_str(),
                matrix_pos_x + i * 96,
                matrix_pos_y + j * 16,
                0x007F7F7F,
                0x0000003F
            );
        }
    }
}


void GraphWindow::Paint()
{
    RECT client_rect;
    GetClientRect(m_hWnd, &client_rect);
    PAINTSTRUCT ps;
    HDC hdc;
    hdc = BeginPaint(m_hWnd, &ps);
    int w = m_Canvas->GetW();
    int h = m_Canvas->GetH();
    HRGN dataRegion = CreateRectRgn(0, 0, w, h);
    RedrawWindow(m_hWnd, NULL, dataRegion, RDW_ERASE | RDW_INVALIDATE);
    DeleteObject(dataRegion);
    m_Canvas->Display(hdc, &client_rect);
    DeleteDC(hdc);
    EndPaint(m_hWnd, &ps);
}


void GraphWindow::Drag()
{
    m_Centre.x -= (float)(m_MousePosition.x - m_DragOrigin.x) / m_ViewScale;
    m_Centre.y -= (float)(m_MousePosition.y - m_DragOrigin.y) / m_ViewScale;

    m_DragOrigin.x = m_MousePosition.x;
    m_DragOrigin.y = m_MousePosition.y;
}


void GraphWindow::Move()
{
    auto mouse_world = ScreenToWorld(m_MousePosition);
    for (auto pV : m_VerticesBeingMoved)
    {
        *pV = mouse_world;
    }
}


void GraphWindow::SetMousePosition(int x, int y)
{
    m_MousePosition.x = x;
    m_MousePosition.y = m_Canvas->GetH() - y;

    m_MouseBeforeZoom = ScreenToWorld(m_MousePosition);
}


void GraphWindow::CompensateZoom()
{
    m_MouseAfterZoom = ScreenToWorld(m_MousePosition);

    m_Centre.x += (m_MouseBeforeZoom.x - m_MouseAfterZoom.x);
    m_Centre.y += (-m_MouseBeforeZoom.y + m_MouseAfterZoom.y);
}


screen_coord_t GraphWindow::WorldToScreen(const vect2_t& vert)
{
    screen_coord_t temp;

    temp.x = (int)(((vert.x) - m_Centre.x) * m_ViewScale);
    temp.y = -(int)(((vert.y) + m_Centre.y) * m_ViewScale);

    return temp;
}


vect2_t GraphWindow::ScreenToWorld(const screen_coord_t& point)
{
    vect2_t temp = { 0.0f, 0.0f };

    temp.x = ((float)(point.x) / m_ViewScale) + m_Centre.x;
    temp.y = -((float)(point.y) / m_ViewScale) - m_Centre.y;

    return temp;
}


void GraphWindow::CreateSketch()
{
    if (m_Image == nullptr)
    {
        return;
    }

    m_A = { -0.5f * static_cast<float>(m_Image->w),  0.5f * static_cast<float>(m_Image->h) };
    m_B = {  0.5f * static_cast<float>(m_Image->w),  0.5f * static_cast<float>(m_Image->h) };
    m_C = {  0.5f * static_cast<float>(m_Image->w), -0.5f * static_cast<float>(m_Image->h) };
    m_D = { -0.5f * static_cast<float>(m_Image->w), -0.5f * static_cast<float>(m_Image->h) };

    m_FrameLinesOriginal->clear();
    m_FrameLinesTransformed->clear();

    if (m_FrameLinesOriginal)
    {
        m_FrameLinesOriginal->push_back({ m_A, m_B, 0x00FF0000 });
        m_FrameLinesOriginal->push_back({ m_B, m_C, 0x00FF0000 });
        m_FrameLinesOriginal->push_back({ m_C, m_D, 0x00FF0000 });
        m_FrameLinesOriginal->push_back({ m_D, m_A, 0x00FF0000 });

        m_FrameLinesTransformed->push_back({ m_A, m_B, 0x00FF0000 });
        m_FrameLinesTransformed->push_back({ m_B, m_C, 0x00FF0000 });
        m_FrameLinesTransformed->push_back({ m_C, m_D, 0x00FF0000 });
        m_FrameLinesTransformed->push_back({ m_D, m_A, 0x00FF0000 });
    }
}


void GraphWindow::AddReference()
{
    if (m_ReferenceLines)
    {
        m_ReferenceLines->push_back({ m_RA, m_RB, 0x00FF7F00 });
        m_ReferenceLines->push_back({ m_RB, m_RC, 0x00FF7F00 });
        m_ReferenceLines->push_back({ m_RC, m_RD, 0x00FF7F00 });
        m_ReferenceLines->push_back({ m_RD, m_RA, 0x00FF7F00 });
    }
}


void GraphWindow::UpdateView()
{
    UpdateHomography();
    Render();
}


void GraphWindow::UpdateSketch()
{
    UpdateHomography();

    if (m_SketchLinesTransformed)
    {
        m_SketchLinesTransformed->clear();
    }
    if (m_SketchLinesOriginal)
    {
        for (auto& sl : *m_SketchLinesOriginal)
        {
            line2_t temp = { m_Homography * sl.S, m_Homography * sl.E, 0x007F7FFF };

            m_SketchLinesTransformed->push_back(temp);
        }
    }

    if (m_FrameLinesTransformed)
    {
        m_FrameLinesTransformed->clear();
    }
    if (m_FrameLinesOriginal && m_FrameLinesOriginal->size())
    {
        for (auto& fl : *m_FrameLinesOriginal)
        {
            line2_t temp = { m_Homography * fl.S, m_Homography * fl.E, 0x00FF0000 };

            m_FrameLinesTransformed->push_back(temp);
        }
    }

    /*
    m_GridLines->clear();
    m_Points->clear();

    if (m_FrameLinesTransformed && m_FrameLinesTransformed->size())
    {
        try
        {
            line2_t a = m_FrameLinesTransformed->at(0);
            line2_t b = m_FrameLinesTransformed->at(1);
            line2_t c = m_FrameLinesTransformed->at(2);
            line2_t d = m_FrameLinesTransformed->at(3);

            vect2_t A_ = a.S;
            vect2_t B_ = b.S;
            vect2_t C_ = c.S;
            vect2_t D_ = d.S;

            vect2_t dummy_x;
            vect2_t dummy_y;
            if (Intersect(d, b, dummy_x) && Intersect(a, c, dummy_y))
            {
                vect2_t U = AddIntersection(d, b);
                vect2_t V = AddIntersection(a, c);
        
                DivideQuadrangle(U, V, a.S, b.S, c.S, d.S, 5);
            }
            else
            {
                DivideRectangle(a.S, b.S, c.S, d.S, 5);
            }
        }
        catch (const std::exception& e)
        {
            MessageBox(
                m_hWnd,
                e.what(),
                "UpdateSketch()",
                MB_OK
            );
        }
    }
    */
}


void GraphWindow::DivideQuadrangle(
    const vect2_t& U,
    const vect2_t& V,
    const vect2_t& A,
    const vect2_t& B,
    const vect2_t& C,
    const vect2_t& D,
    int max_depth
)
{
    vect2_t T = AddIntersection({ A, C }, { B, D });

    vect2_t P = AddIntersection({ U, T }, { A, B });
    vect2_t Q = AddIntersection({ V, T }, { B, C });
    vect2_t R = AddIntersection({ U, T }, { D, C });
    vect2_t S = AddIntersection({ V, T }, { A, D });

    max_depth--;

    if (max_depth > 0)
    {
        DivideQuadrangle(U, V, S, T, R, D, max_depth);
        DivideQuadrangle(U, V, T, Q, C, R, max_depth);
        DivideQuadrangle(U, V, A, P, T, S, max_depth);
        DivideQuadrangle(U, V, P, B, Q, T, max_depth);
    }
    else
    {
        m_GridLines->push_back({ T, P, 0x003F3FFF });
        m_GridLines->push_back({ T, Q, 0x003F3FFF });
        m_GridLines->push_back({ T, R, 0x003F3FFF });
        m_GridLines->push_back({ T, S, 0x003F3FFF });
    }
}


void GraphWindow::DivideRectangle(
    const vect2_t& A,
    const vect2_t& B,
    const vect2_t& C,
    const vect2_t& D,
    int max_depth
)
{
    vect2_t P = AddHalfwayPoint(A, B);
    vect2_t Q = AddHalfwayPoint(B, C);
    vect2_t R = AddHalfwayPoint(C, D);
    vect2_t S = AddHalfwayPoint(D, A);
    vect2_t T = AddHalfwayPoint(P, R);

    max_depth--;

    if (max_depth > 0)
    {
        DivideRectangle(S, T, R, D, max_depth);
        DivideRectangle(T, Q, C, R, max_depth);
        DivideRectangle(A, P, T, S, max_depth);
        DivideRectangle(P, B, Q, T, max_depth);
    }
    else
    {
        m_GridLines->push_back({ T, P, 0x003F3FFF });
        m_GridLines->push_back({ T, Q, 0x003F3FFF });
        m_GridLines->push_back({ T, R, 0x003F3FFF });
        m_GridLines->push_back({ T, S, 0x003F3FFF });
    }
}


vect2_t GraphWindow::AddIntersection(const line2_t& a, const line2_t& b)
{
    vect2_t temp_intersection = { 0.0f, 0.0f };
    if (Intersect(a, b, temp_intersection))
    {
        m_Points->push_back(temp_intersection);
    }
    return temp_intersection;
}


vect2_t GraphWindow::AddHalfwayPoint(const vect2_t& A, const vect2_t& B)
{
    vect2_t temp_halfway_point = HalfwayPoint(A, B);
    m_Points->push_back(temp_halfway_point);
    return temp_halfway_point;
}


void GraphWindow::UpdateHomography()
{
    if (m_ReferenceLines && (m_ReferenceLines->size() == 4))
    {
        try
        {
            line2_t a = m_ReferenceLines->at(0);
            line2_t b = m_ReferenceLines->at(1);
            line2_t c = m_ReferenceLines->at(2);
            line2_t d = m_ReferenceLines->at(3);

            vect2_t A_ = a.S;
            vect2_t B_ = b.S;
            vect2_t C_ = c.S;
            vect2_t D_ = d.S;

            std::vector<vect2_t> original_points = { m_RA, m_RB, m_RC, m_RD };
            std::vector<vect2_t> transformed_points = { A_, B_, C_, D_ };

            mat3x3_t temp(1.0f);
            if (CalculateHomography(original_points, transformed_points, temp))
            {
                m_Homography = temp;

                mat3x3_t temp_inverse(1.0f);
                if (FindInverseMatrix(m_Homography.elements, temp_inverse.elements, 3, 3))
                {
                    m_InverseHomography = temp_inverse;
                }
            }
        }
        catch (const std::exception& e)
        {
            MessageBox(
                m_hWnd,
                e.what(),
                "UpdateHomography()",
                MB_OK
            );
        }
    }
}


void GraphWindow::UndoTransformation()
{
    if (m_ReferenceLines && m_ReferenceLines->size())
    {
        /*
        m_RA = { -150.0f,  150.0f };
        m_RB = {  150.0f,  150.0f };
        m_RC = {  150.0f, -150.0f };
        m_RD = { -150.0f, -150.0f };
        */

        m_RA = m_OriginalFramePoints.at(0);
        m_RB = m_OriginalFramePoints.at(1);
        m_RC = m_OriginalFramePoints.at(2);
        m_RD = m_OriginalFramePoints.at(3);

        try
        {
            m_ReferenceLines->at(0).S = m_RA;
            m_ReferenceLines->at(0).E = m_RB;
            m_ReferenceLines->at(1).S = m_RB;
            m_ReferenceLines->at(1).E = m_RC;
            m_ReferenceLines->at(2).S = m_RC;
            m_ReferenceLines->at(2).E = m_RD;
            m_ReferenceLines->at(3).S = m_RD;
            m_ReferenceLines->at(3).E = m_RA;
        }
        catch (const std::exception& e)
        {
            MessageBox(
                m_hWnd,
                e.what(),
                "UndoTransformation()",
                MB_OK
            );
        }

        UpdateSketch();

        m_Homography =          mat3x3_t(1.0f);
        m_InverseHomography =   mat3x3_t(1.0f);
    }
}


void GraphWindow::DrawTexturedTriangle(
    const screen_coord_t& A,
    const screen_coord_t& B,
    const screen_coord_t& C
)
{
	int x_min, y_min, x_max, y_max;

	x_min = !(B.x < A.x) * A.x + (B.x < A.x) * B.x;
	x_min = !(C.x < x_min) * x_min + (C.x < x_min) * C.x;
	x_min = !(0 >= x_min) * x_min;

	y_min = !(B.y < A.y) * A.y + (B.y < A.y) * B.y;
	y_min = !(C.y < y_min) * y_min + (C.y < y_min) * C.y;
	y_min = !(0 >= y_min) * y_min;

	x_max = !(B.x > A.x)* A.x + (B.x > A.x)* B.x;
	x_max = !(C.x > x_max)* x_max + (C.x > x_max)* C.x;
	x_max = !(m_CanvasW < x_max) * x_max + (m_CanvasW < x_max) * m_CanvasW;

	y_max = !(B.y > A.y)* A.y + (B.y > A.y)* B.y;
	y_max = !(C.y > y_max)* y_max + (C.y > y_max)* C.y;
	y_max = !(m_CanvasH < y_max) * y_max + (m_CanvasH < y_max) * m_CanvasH;

	for (int y = y_min; y < y_max; y++)
	{
		int tA = (C.x - B.x) * (y - B.y);
		int tB = (A.x - C.x) * (y - C.y);
		int tC = (B.x - A.x) * (y - A.y);
		for (int x = x_min; x < x_max; x++)
		{
			int wA = tA - (C.y - B.y) * (x - B.x);
			int wB = tB - (A.y - C.y) * (x - C.x);
			int wC = tC - (B.y - A.y) * (x - A.x);

			if (wA >= 0 && wB >= 0 && wC >= 0)
			{
                auto p_world = ScreenToWorld({ x, y });
                auto p_original = m_InverseHomography * p_world;

                float u = (p_original.x + 0.5f * static_cast<float>(m_Image->w)) / static_cast<float>(m_Image->w);
                float v = (p_original.y + 0.5f * static_cast<float>(m_Image->h)) / static_cast<float>(m_Image->h);

                bmp32_t pixel = { 0 };

                pixel.colour = m_Image->GetSample(u, v, 1, 1);

                m_Canvas->PutPixel(x, y, pixel.colour);
			}
		}
	}
}


void GraphWindow::SaveTexturedQuadrangle(const std::string& filename, texture_t& txt)
{
    try
    {
        auto A = WorldToScreen(m_FrameLinesTransformed->at(0).S);
        auto B = WorldToScreen(m_FrameLinesTransformed->at(1).S);
        auto C = WorldToScreen(m_FrameLinesTransformed->at(2).S);
        auto D = WorldToScreen(m_FrameLinesTransformed->at(3).S);
    
        A.u = 0.0f;     A.v = 0.0f;
        B.u = 1.0f;     B.v = 0.0f;
        C.u = 1.0f;     C.v = 1.0f;
        D.u = 0.0f;     D.v = 1.0f;
    
        int x_min, y_min, x_max, y_max;
    
        x_min = !(B.x < A.x) * A.x + (B.x < A.x) * B.x;
        x_min = !(C.x < x_min) * x_min + (C.x < x_min) * C.x;
        x_min = !(D.x < x_min) * x_min + (D.x < x_min) * D.x;
    
        y_min = !(B.y < A.y) * A.y + (B.y < A.y) * B.y;
        y_min = !(C.y < y_min) * y_min + (C.y < y_min) * C.y;
        y_min = !(D.y < y_min) * y_min + (D.y < y_min) * D.y;
    
        x_max = !(B.x > A.x)* A.x + (B.x > A.x)* B.x;
        x_max = !(C.x > x_max)* x_max + (C.x > x_max)* C.x;
        x_max = !(D.x > x_max)* x_max + (D.x > x_max)* D.x;
    
        y_max = !(B.y > A.y)* A.y + (B.y > A.y)* B.y;
        y_max = !(C.y > y_max)* y_max + (C.y > y_max)* C.y;
        y_max = !(D.y > y_max)* y_max + (D.y > y_max)* D.y;
    
        txt.w = std::abs(x_max - x_min);
        txt.h = std::abs(y_max - y_min);
    
        if ((txt.w <= 0) || (txt.h <= 0))
        {
            return;
        }

        std::string d = "";
        d += "A.x: ";
        d += std::to_string(A.x);
        d += " - ";
        d += "A.y: ";
        d += std::to_string(A.y);
        d += " - ";
        d += "B.x: ";
        d += std::to_string(B.x);
        d += " - ";
        d += "B.y: ";
        d += std::to_string(B.y);
        d += " - ";
        d += "C.x: ";
        d += std::to_string(C.x);
        d += " - ";
        d += "C.y: ";
        d += std::to_string(C.y);
        d += " - ";
        d += "D.x: ";
        d += std::to_string(D.x);
        d += " - ";
        d += "D.y: ";
        d += std::to_string(D.y);

        MessageBox(
            m_hWnd,
            d.c_str(),
            "Corner Points",
            MB_OK
        );

        std::string m = "";
        m += "x_min: ";
        m += std::to_string(x_min);
        m += " - ";
        m += "y_min: ";
        m += std::to_string(y_min);
        m += " - ";
        m += "x_max: ";
        m += std::to_string(x_max);
        m += " - ";
        m += "y_max: ";
        m += std::to_string(y_max);

        MessageBox(
            m_hWnd,
            m.c_str(),
            "Limits",
            MB_OK
        );
    
        std::string size_msg = "Image size: " + std::to_string(txt.w) + " * " + std::to_string(txt.h);
    
        MessageBox(
            m_hWnd,
            size_msg.c_str(),
            "Saving Image",
            MB_OK
        );

        txt.buffer = new uint32_t[txt.w * txt.h];

        if (txt.buffer == nullptr)
        {
            return;
        }

        memset(&txt.buffer[0], 0, txt.w * txt.h * sizeof(uint32_t));

        for (int y = y_min, j = 0; y < y_max; y++, j++)
        {
            int tA = (C.x - B.x) * (y - B.y);
            int tB = (A.x - C.x) * (y - C.y);
            int tC = (B.x - A.x) * (y - A.y);
            for (int x = x_min, i = 0; x < x_max; x++, i++)
            {
                int wA = tA - (C.y - B.y) * (x - B.x);
                int wB = tB - (A.y - C.y) * (x - C.x);
                int wC = tC - (B.y - A.y) * (x - A.x);

                if (wA >= 0 && wB >= 0 && wC >= 0)
                {
                    auto p_world = ScreenToWorld({ x, y });
                    auto p_original = m_InverseHomography * p_world;

                    float u = (p_original.x + 0.5f * static_cast<float>(m_Image->w)) / static_cast<float>(m_Image->w);
                    float v = (p_original.y + 0.5f * static_cast<float>(m_Image->h)) / static_cast<float>(m_Image->h);

                    txt.buffer[j * txt.w + i] = m_Image->GetSample(u, v, 1, 1);
                }
            }
        }

        for (int y = y_min, j = 0; y < y_max; y++, j++)
        {
            int tA = (A.x - D.x) * (y - D.y);
            int tB = (C.x - A.x) * (y - A.y);
            int tC = (D.x - C.x) * (y - C.y);
            for (int x = x_min, i = 0; x < x_max; x++, i++)
            {
                int wA = tA - (A.y - D.y) * (x - D.x);
                int wB = tB - (C.y - A.y) * (x - A.x);
                int wC = tC - (D.y - C.y) * (x - C.x);

                if (wA >= 0 && wB >= 0 && wC >= 0)
                {
                    auto p_world = ScreenToWorld({ x, y });
                    auto p_original = m_InverseHomography * p_world;

                    float u = (p_original.x + 0.5f * static_cast<float>(m_Image->w)) / static_cast<float>(m_Image->w);
                    float v = (p_original.y + 0.5f * static_cast<float>(m_Image->h)) / static_cast<float>(m_Image->h);

                    txt.buffer[j * txt.w + i] = m_Image->GetSample(u, v, 1, 1);
                }
            }
        }

        if (!BMPManager::WriteBitMapData(filename, txt))
        {
            MessageBox(
                m_hWnd,
                "Could not save BMP file.",
                "Bitmap file error",
                MB_OK
            );
        }


    }
    catch (const std::exception& e)
    {
        MessageBox(
            m_hWnd,
            e.what(),
            "SaveTexturedQuadrangle()",
            MB_OK
        );
    }
}