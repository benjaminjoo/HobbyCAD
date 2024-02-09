#include "Canvas.h"


Canvas::Canvas(int w, int h)
	:
	m_BitmapW(w),
	m_BitmapH(h),
	m_BitmapInfo({ 0 }),
	m_FrameBuffer(nullptr)
{
	memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));
	m_BitmapInfo.bmiHeader.biSize = sizeof(m_BitmapInfo.bmiHeader);
	m_BitmapInfo.bmiHeader.biWidth = w;
	m_BitmapInfo.bmiHeader.biHeight = h;
	m_BitmapInfo.bmiHeader.biPlanes = 1;
	m_BitmapInfo.bmiHeader.biBitCount = 32;
	m_BitmapInfo.bmiHeader.biCompression = BI_RGB;

	unsigned long size = m_BitmapW * m_BitmapH * 4;

	m_FrameBuffer = (uint32_t*)VirtualAlloc(
		NULL,
		size,
		MEM_COMMIT,
		PAGE_READWRITE
	);
}


Canvas::~Canvas()
{
	if (m_FrameBuffer != nullptr)
	{
		VirtualFree(m_FrameBuffer, 0, MEM_RELEASE);
	}
}


void Canvas::Resize(int w, int h)
{
	if (m_FrameBuffer != nullptr)
	{
		VirtualFree(m_FrameBuffer, 0, MEM_RELEASE);
	}
		
	m_BitmapW = w;
	m_BitmapH = h;

	memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));
	m_BitmapInfo.bmiHeader.biSize = sizeof(m_BitmapInfo.bmiHeader);
	m_BitmapInfo.bmiHeader.biWidth = m_BitmapW;
	m_BitmapInfo.bmiHeader.biHeight = m_BitmapH;
	m_BitmapInfo.bmiHeader.biPlanes = 1;
	m_BitmapInfo.bmiHeader.biBitCount = 32;
	m_BitmapInfo.bmiHeader.biCompression = BI_RGB;

	unsigned long size = w * h * 4;

	m_FrameBuffer = (uint32_t*)VirtualAlloc(
		0,
		size,
		MEM_COMMIT,
		PAGE_READWRITE
	);
}


void Canvas::Clear()
{
	memset(m_FrameBuffer, 0x7F, m_BitmapW * m_BitmapH * sizeof(uint32_t));
}


void Canvas::Fill(uint32_t colour)
{
	for (int y = 0; y < m_BitmapH; y++)
	{
		for (int x = 0; x < m_BitmapW; x++)
		{
			m_FrameBuffer[y * m_BitmapW + x] = colour;
		}
	}
}


void Canvas::PutPixel(int x, int y, uint32_t colour)
{
	if (x >= 0 && x < m_BitmapW
		&&
		y >= 0 && y < m_BitmapH)
	{
		m_FrameBuffer[y * m_BitmapW + x] = colour;
	}
}


uint32_t Canvas::GetPixel(int x, int y)
{
	if (x >= 0 && x < m_BitmapW
		&&
		y >= 0 && y < m_BitmapH)
	{
		return m_FrameBuffer[y * m_BitmapW + x];
	}
    return 0;
}


void Canvas::DrawFilledRect(int x, int y, int w, int h, uint32_t color)
{
	for (int j = y; j < y + h; j++)
	{
		for (int i = x; i < x + w; i++)
		{
			PutPixel(i, j, color);
		}
	}
}


uint32_t Canvas::ModifyColour(uint8_t r, uint8_t g, uint8_t b, const float illumination)
{
	float rgb[4];
	
	rgb[0] = (static_cast<float>(r)* illumination);
	rgb[1] = (static_cast<float>(g)* illumination);
	rgb[2] = (static_cast<float>(b)* illumination);
	
	float rgb_max = rgb[0];
	if (rgb[1] > rgb_max) rgb_max = rgb[1];
	if (rgb[2] > rgb_max) rgb_max = rgb[2];
	
	bool overexposed = rgb_max > 255.0f;
	float scale = 1.0f * !overexposed + (255.0f / rgb_max) * overexposed;
	
	rgb[0] = rgb[0] * scale;
	rgb[1] = rgb[1] * scale;
	rgb[2] = rgb[2] * scale;
	
	return static_cast<uint8_t>(rgb[0]) << 16 | static_cast<uint8_t>(rgb[1]) << 8 | static_cast<uint8_t>(rgb[2]);
}


void Canvas::DrawTexturedTriangle(
    const screen_coord_t& A,
    const screen_coord_t& B,
    const screen_coord_t& C,
    const texture_t& txt
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
	x_max = !(m_BitmapW < x_max) * x_max + (m_BitmapW < x_max) * m_BitmapW;

	y_max = !(B.y > A.y)* A.y + (B.y > A.y)* B.y;
	y_max = !(C.y > y_max)* y_max + (C.y > y_max)* C.y;
	y_max = !(m_BitmapH < y_max) * y_max + (m_BitmapH < y_max) * m_BitmapH;

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
                float u = (wA * A.u + wB * B.u + wC * C.u) / (wA + wB + wC);
                float v = (wA * A.v + wB * B.v + wC * C.v) / (wA + wB + wC);

                bmp32_t pixel = { 0 };

                pixel.colour = txt.GetSample(u, v, 4, 4);

                memcpy(m_FrameBuffer + y * m_BitmapW + x, &pixel.colour, 4);		
			}
		}
	}
}


void Canvas::DrawPoint(int x, int y, uint32_t pixel_colour)
{
	for (int j = 0; j < NODE_SIZE; j++)
	{
		for (int i = 0; i < NODE_SIZE; i++)
		{
			if (m_Node[j * NODE_SIZE + i])
			{
                int x_coord = x - NODE_SIZE / 2 + i;
                int y_coord = y - NODE_SIZE / 2 + j;
                uint32_t canvas_colour = GetPixel(x_coord, y_coord);
                uint32_t colour = Blend(canvas_colour, pixel_colour);
				PutPixel(x_coord, y_coord, colour);
			}
		}
	}
}


void Canvas::DrawAnchor(int x, int y, uint32_t pixel_colour)
{
	for (int j = 0; j < ANCHOR_SIZE; j++)
	{
		for (int i = 0; i < ANCHOR_SIZE; i++)
		{
			if (m_Anchor[j * ANCHOR_SIZE + i])
			{
                int x_coord = x - ANCHOR_SIZE / 2 + i;
                int y_coord = y - ANCHOR_SIZE / 2 + j;
                //uint32_t canvas_colour = GetPixel(x_coord, y_coord);
                //uint32_t colour = Blend(canvas_colour, pixel_colour);
				PutPixel(x_coord, y_coord, pixel_colour);
			}
		}
	}
}


void Canvas::Line(int start_x, int start_y, int end_x, int end_y, const uint32_t& colour, int lineweight)
{
	int DeltaX, DeltaY;
	int Temp;

	if (start_y > end_y)
	{
		Temp = start_y;
		start_y = end_y;
		end_y = Temp;
		Temp = start_x;
		start_x = end_x;
		end_x = Temp;
	}

	DeltaX = end_x - start_x;
	DeltaY = end_y - start_y;
	if (DeltaX > 0)
	{
		if (DeltaX > DeltaY)
		{
			Octant0(start_x, start_y, DeltaX, DeltaY, 1, colour, lineweight);
		}
		else
		{
			Octant1(start_x, start_y, DeltaX, DeltaY, 1, colour, lineweight);
		}
	}
	else
	{
		DeltaX = -DeltaX;
		if (DeltaX > DeltaY)
		{
			Octant0(start_x, start_y, DeltaX, DeltaY, -1, colour, lineweight);
		}
		else
		{
			Octant1(start_x, start_y, DeltaX, DeltaY, -1, colour, lineweight);
		}
	}
}


void Canvas::Octant0(int start_x, int start_y, int DeltaX, int DeltaY, int XDirection, const uint32_t& colour, int lineweight)
{
	int DeltaYx2;
	int DeltaYx2MinusDeltaXx2;
	int ErrorTerm;

	DeltaYx2 = DeltaY * 2;
	DeltaYx2MinusDeltaXx2 = DeltaYx2 - (int)(DeltaX * 2);
	ErrorTerm = DeltaYx2 - (int)DeltaX;

	if (lineweight == 1)
	{
		PutPixel(start_x, start_y, colour);
	}
	else
	{
		for (int j = -lineweight / 2; j < lineweight / 2; j++)
		{
			for (int i = -lineweight / 2; i < lineweight / 2; i++)
			{
				PutPixel(start_x + i, start_y + j, colour);
			}
		}
	}
	while (DeltaX--)
	{
		if (ErrorTerm >= 0)
		{
			start_y++;
			ErrorTerm += DeltaYx2MinusDeltaXx2;
		}
		else
		{
			ErrorTerm += DeltaYx2;
		}
		start_x += XDirection;
		if (lineweight == 1)
		{
			PutPixel(start_x, start_y, colour);
		}
		else
		{
			for (int j = -lineweight / 2; j < lineweight / 2; j++)
			{
				for (int i = -lineweight / 2; i < lineweight / 2; i++)
				{
					PutPixel(start_x + i, start_y + j, colour);
				}
			}
		}
	}
}


void Canvas::Octant1(int start_x, int start_y, int DeltaX, int DeltaY, int XDirection, const uint32_t& colour, int lineweight)
{
	int DeltaXx2;
	int DeltaXx2MinusDeltaYx2;
	int ErrorTerm;

	DeltaXx2 = DeltaX * 2;
	DeltaXx2MinusDeltaYx2 = DeltaXx2 - (int)(DeltaY * 2);
	ErrorTerm = DeltaXx2 - (int)DeltaY;

	if (lineweight == 1)
	{
		PutPixel(start_x, start_y, colour);
	}
	else
	{
		for (int j = -lineweight / 2; j < lineweight / 2; j++)
		{
			for (int i = -lineweight / 2; i < lineweight / 2; i++)
			{
				PutPixel(start_x + i, start_y + j, colour);
			}
		}
	}
	while (DeltaY--)
	{
		if (ErrorTerm >= 0)
		{
			start_x += XDirection;
			ErrorTerm += DeltaXx2MinusDeltaYx2;
		}
		else
		{
			ErrorTerm += DeltaXx2;
		}
		start_y++;
		if (lineweight == 1)
		{
			PutPixel(start_x, start_y, colour);
		}
		else
		{
			for (int j = -lineweight / 2; j < lineweight / 2; j++)
			{
				for (int i = -lineweight / 2; i < lineweight / 2; i++)
				{
					PutPixel(start_x + i, start_y + j, colour);
				}
			}
		}
	}
}


void Canvas::DrawLine(const screen_coord_t& p, const screen_coord_t& q, const uint32_t& colour, int lineweight)
{
	Line(p.x, p.y, q.x, q.y, colour, lineweight);
}


void Canvas::DrawLine(int start_x, int start_y, int end_x, int end_y, const uint32_t& colour)
{
	if (start_x == end_x &&
		start_y == end_y &&
		start_x >= 0 && start_x < m_BitmapW &&
		start_y >= 0 && start_y < m_BitmapH)
	{
		memcpy(&m_FrameBuffer[start_y * m_BitmapW + start_x], &colour, sizeof(uint32_t));
	}
	else
	{
		float deltaX = static_cast<float>(end_x - start_x);
		float deltaY = static_cast<float>(end_y - start_y);

		if (fabs(end_x - start_x) >= fabs(end_y - start_y))
		{
			/*Line is 'X'-major*/

			bool leftFirst = start_x < end_x;
			bool lowerFirst = start_y < end_y;

			int x1 = (leftFirst) ? start_x : end_x;
			int y1 = (leftFirst) ? start_y : end_y;
			int x2 = (leftFirst) ? end_x : start_x;
			int y2 = (leftFirst) ? end_y : start_y;
			int stepY = (leftFirst) ? ((lowerFirst) ? 1 : -1) : ((lowerFirst) ? -1 : 1);
			int xCount = 0;
			int yCount = 0;

			for (int xCurrent = x1; xCurrent <= x2; xCurrent++, xCount++)
			{
				int yCurrent = y1 + yCount;
				float yIdeal = (deltaY / deltaX) * xCount;
				yCount += (stepY * (abs(yIdeal - yCount) > 0.5f));
				if ((xCurrent >= 0 && xCurrent < m_BitmapW) && (yCurrent >= 0 && yCurrent < m_BitmapH))
				{
					m_FrameBuffer[yCurrent * m_BitmapW + xCurrent] = colour;
				}
			}
		}
		else
		{
			/*Line is 'Y'-major*/

			bool leftFirst = start_x < end_x;
			bool lowerFirst = start_y < end_y;

			int x1 = (lowerFirst) ? start_x : end_x;
			int y1 = (lowerFirst) ? start_y : end_y;
			int x2 = (lowerFirst) ? end_x : start_x;
			int y2 = (lowerFirst) ? end_y : start_y;
			int stepX = (lowerFirst) ? ((leftFirst) ? 1 : -1) : ((leftFirst) ? -1 : 1);
			int xCount = 0;
			int yCount = 0;

			for (int yCurrent = y1; yCurrent <= y2; yCurrent++, yCount++)
			{
				int xCurrent = x1 + xCount;
				float xIdeal = (deltaX / deltaY) * yCount;
				xCount += (stepX * (abs(xIdeal - xCount) > 0.5f));
				if ((xCurrent >= 0 && xCurrent < m_BitmapW) && (yCurrent >= 0 && yCurrent < m_BitmapH))
				{
					m_FrameBuffer[yCurrent * m_BitmapW + xCurrent] = colour;
				}
			}
		}
	}
}


void Canvas::DrawHorizontalLine(int screen_y, const uint32_t& colour, int step)
{
	if ((screen_y < 0) || (screen_y >= m_BitmapH))
	{
		return;
	}
	if (step < 1)
	{
		step = 1;
	}
	for (int x = 0; x < m_BitmapW; x += step)
	{
		m_FrameBuffer[screen_y * m_BitmapW + x] = colour;
	}
}


void Canvas::DrawVerticalLine(int screen_x, const uint32_t& colour, int step)
{
	if ((screen_x < 0) || (screen_x >= m_BitmapW))
	{
		return;
	}
	if (step < 1)
	{
		step = 1;
	}
	for (int y = 0; y < m_BitmapH; y += step)
	{
		m_FrameBuffer[y * m_BitmapW + screen_x] = colour;
	}
}


void Canvas::Display(HDC hdc, RECT* rect, int x, int y)
{
	int windowW = rect->right - rect->left;
	int windowH = rect->bottom - rect->top - y;

	StretchDIBits(
		hdc,                // hdc
		x,                  // xDest
		y,                  // yDest
		windowW,			// DestWidth
		windowH,			// DestHeight
		0,                  // xSrc
		0,                  // ySrc
		m_BitmapW,          // SrcWidth
		m_BitmapH,          // SrcHeight
		m_FrameBuffer,      // lpBits
		&m_BitmapInfo,      // lpbmi
		DIB_RGB_COLORS,     // iUsage
		SRCCOPY             // rop
	);
}


uint32_t Canvas::Blend(uint32_t A, uint32_t B)
{
	uint16_t rA = static_cast<uint16_t>((A >> 16) & 0xFF);
	uint16_t gA = static_cast<uint16_t>((A >> 8) & 0xFF);
	uint16_t bA = static_cast<uint16_t>((A >> 0) & 0xFF);

	uint16_t rB = static_cast<uint16_t>((B >> 16) & 0xFF);
	uint16_t gB = static_cast<uint16_t>((B >> 8) & 0xFF);
	uint16_t bB = static_cast<uint16_t>((B >> 0) & 0xFF);

	uint8_t rC = static_cast<uint8_t>((rA + rB) / 2);
	uint8_t gC = static_cast<uint8_t>((gA + gB) / 2);
	uint8_t bC = static_cast<uint8_t>((bA + bB) / 2);

	return (rC << 16) | (gC << 8) | bC;
}