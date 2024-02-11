#pragma once


#include <memory>
#include <string>
#include <vector>
#include <functional>


#include "Canvas.h"
#include "Geometry.h"
#include "TextRenderer.h"
#include "BMPManager.h"
#include "Utilities.h"
#include "Globals.h"


#define SNAP_RANGE          25


class GraphWindow
{
public:

	GraphWindow();

	BOOL Create(
		LPCSTR		lpWindowName,
		DWORD		dwStyle,
		DWORD		dwExStyle = 0,
		int			x = CW_USEDEFAULT,
		int			y = CW_USEDEFAULT,
		int			w = CW_USEDEFAULT,
		int			h = CW_USEDEFAULT,
		HWND		hWndParent = 0,
		HMENU		hMenu = 0
	);

	void SetBackgroundColour(uint32_t c);

    void InitialiseFrameLinesOriginal(std::vector<line2_t>* frame_lines);

    void InitialiseFrameLinesTransformed(std::vector<line2_t>* frame_lines);

    void InitialiseReferenceLines(std::vector<line2_t>* ref_lines);

    void InitialiseGridLines(std::vector<line2_t>* grid_lines);

    void InitialiseSketchLinesOriginal(std::vector<line2_t>* sketch_lines);

    void InitialiseSketchLinesTransformed(std::vector<line2_t>* sketch_lines);

    void InitialisePoints(std::vector<vect2_t>* points);

    void InitialiseImage(texture_t* image);

    void InitialiseRefreshHandler(std::function<void()> h_refresh);

    void CreateSketch();

    void AddReference();

    void UpdateView();

    void UpdateSketch();

	void Render();

private:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Clear();

    void DrawOrigin();

    void DrawSketch();

    void DrawContextSpecificStuff();

    void DrawCrosshair();

    void DrawHotspot();

    void RenderText();

	void Paint();

	void Drag();

    void Move();

	void SetMousePosition(int x, int y);

	void CompensateZoom();

	screen_coord_t WorldToScreen(const vect2_t& vert);

	vect2_t ScreenToWorld(const screen_coord_t& point);

    void DivideQuadrangle(
        const vect2_t& U,
        const vect2_t& V,
        const vect2_t& A,
        const vect2_t& B,
        const vect2_t& C,
        const vect2_t& D,
        int max_depth
    );

    void DivideRectangle(
        const vect2_t& A,
        const vect2_t& B,
        const vect2_t& C,
        const vect2_t& D,
        int max_depth
    );

    vect2_t AddIntersection(const line2_t& a, const line2_t& b);

    vect2_t AddHalfwayPoint(const vect2_t& A, const vect2_t& B);

    void UpdateHomography();

    void UndoTransformation();

    void DrawTexturedTriangle(
        const screen_coord_t& A,
        const screen_coord_t& B,
        const screen_coord_t& C
    );

	HWND							m_hWnd;
	HWND							m_hParentWnd;

	std::shared_ptr<Canvas>			m_Canvas;
	std::shared_ptr<TextRenderer>	m_Printer;

	float							m_ViewScale;
	bool							m_LeftClickInProgress;
	bool							m_MouseDrag;
    bool                            m_MouseMove;
	bool							m_MouseMeasure;
    bool                            m_HotspotFound;
	screen_coord_t					m_MousePosition;
	screen_coord_t					m_DragOrigin;
	screen_coord_t					m_MoveOrigin;
	screen_coord_t					m_MeasureOrigin;
    vect2_t                         m_HotspotPosition;
	vect2_t							m_MouseBeforeZoom;
	vect2_t							m_MouseAfterZoom;
	vect2_t							m_Centre;

	uint32_t						m_BackgroundColour;

    int                             m_CanvasX;
    int                             m_CanvasY;
    int                             m_CanvasW;
    int                             m_CanvasH;

    std::vector<line2_t>*           m_FrameLinesOriginal;
    std::vector<line2_t>*           m_FrameLinesTransformed;
    std::vector<line2_t>*           m_ReferenceLines;
    std::vector<line2_t>*           m_GridLines;
    std::vector<line2_t>*           m_SketchLinesOriginal;
    std::vector<line2_t>*           m_SketchLinesTransformed;
    std::vector<vect2_t>*           m_Points;

    texture_t*                      m_Image;

    bool                            m_RenderImage;

    mat3x3_t                        m_Homography;
    mat3x3_t                        m_InverseHomography;

    vect2_t                         m_TempStartPoint;
    vect2_t                         m_TempEndPoint;
    line2_t                         m_TempLine;

    std::vector<vect2_t*>           m_VerticesBeingMoved;

    vect2_t                         m_TempMoveStartPoint;
    line2_t                         m_TempMoveLine;

    std::function<void()>           m_HandleRefreshAll;

    vect2_t                         m_A;
    vect2_t                         m_B;
    vect2_t                         m_C;
    vect2_t                         m_D;

    vect2_t                         m_RA;
    vect2_t                         m_RB;
    vect2_t                         m_RC;
    vect2_t                         m_RD;

};