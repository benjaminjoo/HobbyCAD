#pragma once


#include <Windows.h>
#include <cstdint>
#include <cmath>


#include "BMPManager.h"


#define NODE_SIZE	    7
#define ANCHOR_SIZE     11


struct screen_coord_t
{
	int x = 0;
	int y = 0;
	float u = 0.0f;
	float v = 0.0f;
};


class Canvas
{
public:

	Canvas(int w, int h);
	~Canvas();

	int GetW()
	{
		return m_BitmapW;
	}

	int GetH()
	{
		return m_BitmapH;
	}

	uint32_t* GetFrameBuffer()
	{
		return m_FrameBuffer;
	}

	void Resize(int w, int h);
	void Clear();
	void Fill(uint32_t colour);

	void PutPixel(int x, int y, uint32_t colour);
    uint32_t GetPixel(int x, int y);

	void DrawFilledRect(int x, int y, int w, int h, uint32_t color);

    uint32_t ModifyColour(uint8_t r, uint8_t g, uint8_t b, const float illumination);

    void DrawTexturedTriangle(
        const screen_coord_t& A,
        const screen_coord_t& B,
        const screen_coord_t& C,
        const texture_t& txt
    );

	void DrawPoint(int x, int y, uint32_t colour);
    void DrawAnchor(int x, int y, uint32_t colour);

	void Line(int start_x, int start_y, int end_x, int end_y, const uint32_t& colour, int lineweight = 1);
	void Octant0(int start_x, int start_y, int DeltaX, int DeltaY, int XDirection, const uint32_t& colour, int lineweight = 1);
	void Octant1(int start_x, int start_y, int DeltaX, int DeltaY, int XDirection, const uint32_t& colour, int lineweight = 1);

	void DrawLine(const screen_coord_t& p, const screen_coord_t& q, const uint32_t& colour, int lineweight = 1);
	void DrawLine(int start_x, int start_y, int end_x, int end_y, const uint32_t& colour);

	void DrawHorizontalLine(int screen_y, const uint32_t& colour, int step = 1);
	void DrawVerticalLine(int screen_x, const uint32_t& colour, int step = 1);

	void Display(HDC hdc, RECT* rect, int x = 0, int y = 0);

private:

	static uint32_t Blend(uint32_t A, uint32_t B);

	BITMAPINFO			m_BitmapInfo;
	uint32_t*			m_FrameBuffer;
	int					m_BitmapW;
	int					m_BitmapH;

	const int			m_Node[NODE_SIZE * NODE_SIZE] = {	0, 0, 0, 1, 0, 0, 0,
															0, 1, 1, 1, 1, 1, 0,
															0, 1, 1, 1, 1, 1, 0,
															1, 1, 1, 1, 1, 1, 1,
															0, 1, 1, 1, 1, 1, 0,
															0, 1, 1, 1, 1, 0, 0,
															0, 0, 0, 1, 0, 1, 0 };

	const int			m_Anchor[ANCHOR_SIZE * ANCHOR_SIZE] = { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
                                                                0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
                                                                0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
                                                                0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
                                                                1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1,
                                                                1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1,
                                                                1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1,
                                                                0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
                                                                0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
                                                                0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
                                                                0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 };

};