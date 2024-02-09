#pragma once


#include <string>
#include <memory>


#include "CharacterSet.h"


class TextRenderer
{
public:

	TextRenderer(uint32_t* screen, int w, int h);
	~TextRenderer();

	void Resize(int w, int h);
	void SetFrameBuffer(uint32_t* screen);

	void DisplayValue(float value, int dec, int shiftH, int shiftV, uint32_t colour);
	void DisplayString(const char* text, int shiftH, int shiftV, uint32_t colour, uint32_t background = 0x00000000);

private:

	float pow10(int n);
	std::shared_ptr<int[]> GetFractionals(double number, int nDecimals);
	std::shared_ptr<int[]> GetIntegers(double number, int* n);
	bool* GetSinglePETSCIICharacter(char char_no);
	uint32_t Blend(uint32_t base, uint32_t overlay);

	int m_Width;
	int m_Height;

	uint32_t* m_FrameBuffer;

};