#pragma once


#define WIN32_LEAN_AND_MEAN


#include <map>


#include <Windows.h>


#include "GraphWindow.h"
#include "Globals.h"


#define ACTIVATE_SELECTION_TOOL      0x6000
#define ACTIVATE_LINE_TOOL           0x6001
#define ACTIVATE_POLYLINE_TOOL       0x6002
#define ACTIVATE_POLYGON_TOOL       0x6003


using Window = std::shared_ptr<GraphWindow>;


class Application
{
public:

	Application(HINSTANCE hInstance);
	~Application();

	bool Initialise();
	void Run();

    void RefreshAll();

	static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	LRESULT CALLBACK HandleMainWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	void PaintMainWindow(HWND hwnd);

    void CreateButton(
        const std::string& name,
        const std::string& text,
        int x,
        int y,
        int w,
        int h,
        HMENU command
    );

    void LoadOwnCursor();

	HINSTANCE									m_hInstance;
	WNDCLASSA									m_AppWndClass;

	HDC											m_hDc;
	PAINTSTRUCT									m_Ps;

    HFONT								        m_WindowFont;

	HWND										m_hMainWnd;

	std::map<std::string, Window>				m_GraphWindows;

    std::map<std::string, HWND>                 m_Buttons;

    std::vector<line2_t>                        m_ReferenceLines;
    std::vector<line2_t>                        m_GridLines;
    std::vector<line2_t>                        m_SketchLinesOriginal;
    std::vector<line2_t>                        m_SketchLinesTransformed;
    std::vector<vect2_t>                        m_Points;

};