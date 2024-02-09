#include "headers/TextRenderer.h"


TextRenderer::TextRenderer(uint32_t* screen, int w, int h)
	:
	m_FrameBuffer(screen),
	m_Width(w),
	m_Height(h)
{

}


TextRenderer::~TextRenderer()
{

}


void TextRenderer::Resize(int w, int h)
{
	m_Width = w;
	m_Height = h;
}


void TextRenderer::SetFrameBuffer(uint32_t* screen)
{
	m_FrameBuffer = screen;
}


void TextRenderer::DisplayValue(float value, int dec, int shiftH, int shiftV, uint32_t colour)
{
	bool isSigned = value >= 0.0 ? false : true;

	if (value < 0.0)
	{
		value = -value;
	}

	std::shared_ptr<int[]> fract = GetFractionals(value, dec);
	int nInt = 0;
	std::shared_ptr<int[]> dInt = GetIntegers(value, &nInt);

	int posH = 0;
	int posV = 0;

	if (shiftH >= 0)
	{
		posH = (shiftH + 2 + nInt) * PETSCII_FONT_SIZE;
	}
	else
	{
		posH = m_Width - 1 - (dec - shiftH) * PETSCII_FONT_SIZE;
	}

	if (shiftV >= 0)
	{
		posV = shiftV * PETSCII_FONT_SIZE;
	}
	else
	{
		posV = m_Height + shiftV * PETSCII_FONT_SIZE;
	}

	for (int p = 0; p < dec; p++)
	{
		bool* currentMap = GetSinglePETSCIICharacter(static_cast<char>(fract[p]) + 48);
		for (int j = 0; j < PETSCII_FONT_SIZE; j++)
		{
			for (int i = 0; i < PETSCII_FONT_SIZE; i++)
			{
				if (currentMap[j * PETSCII_FONT_SIZE + i])
				{
					m_FrameBuffer[(posV + j) * m_Width + posH + i] = colour;
				}
			}
		}
		posH += PETSCII_FONT_SIZE;
	}

	if (shiftH >= 0)
	{
		posH = (shiftH + 1) * PETSCII_FONT_SIZE;
	}
	else
	{
		posH = m_Width - 1 - (dec + nInt + 1 - shiftH) * PETSCII_FONT_SIZE;
	}

	for (int p = 0; p < nInt; p++)
	{
		bool* currentMap = GetSinglePETSCIICharacter(static_cast<char>(dInt[p]) + 48);
		for (int j = 0; j < PETSCII_FONT_SIZE; j++)
		{
			for (int i = 0; i < PETSCII_FONT_SIZE; i++)
			{
				if (currentMap[j * PETSCII_FONT_SIZE + i])
				{
					m_FrameBuffer[(posV + j) * m_Width + posH + i] = colour;
				}
			}
		}
		posH += PETSCII_FONT_SIZE;
	}

	if (shiftH >= 0)
	{
		posH = (shiftH + nInt + 1) * PETSCII_FONT_SIZE;
	}
	else
	{
		posH = m_Width - 1 - (dec + 1 - shiftH) * PETSCII_FONT_SIZE;
	}

	if (dec > 0)
	{
		bool* currentMap = GetSinglePETSCIICharacter('.');
		for (int j = 0; j < PETSCII_FONT_SIZE; j++)
		{
			for (int i = 0; i < PETSCII_FONT_SIZE; i++)
			{
				if (currentMap[j * PETSCII_FONT_SIZE + i])
				{
					m_FrameBuffer[(posV + j) * m_Width + posH + i] = colour;
				}
			}
		}
	}

	if (shiftH >= 0)
	{
		posH = (shiftH)*PETSCII_FONT_SIZE;
	}
	else
	{
		posH = m_Width - 1 - (dec + nInt + 2 - shiftH) * PETSCII_FONT_SIZE;
	}

	if (isSigned)
	{
		bool* currentMap = GetSinglePETSCIICharacter('-');
		for (int j = 0; j < PETSCII_FONT_SIZE; j++)
		{
			for (int i = 0; i < PETSCII_FONT_SIZE; i++)
			{
				if (currentMap[j * PETSCII_FONT_SIZE + i])
				{
					m_FrameBuffer[(posV + j) * m_Width + posH + i] = colour;
				}
			}
		}
	}
}


void TextRenderer::DisplayString(const char* text, int shiftH, int shiftV, uint32_t colour, uint32_t background)
{
	int nChar = 0;
	while (text[nChar] != '\0') { nChar++; }

	int posH = 0;
	int posV = 0;

	if (shiftH >= 0)
	{
		posH = shiftH;
	}
	else
	{
		posH = m_Width - 1 - (nChar - shiftH);
	}
	if (shiftV >= 0)
	{
		posV = shiftV;
	}
	else
	{
		posV = m_Height + shiftV;
	}

	for (int c = 0; c < nChar; c++)
	{
		bool* currentMap = GetSinglePETSCIICharacter(text[c]);
		for (int j = 0; j < PETSCII_FONT_SIZE; j++)
		{
			for (int i = 0; i < PETSCII_FONT_SIZE; i++)
			{
				if (currentMap[j * PETSCII_FONT_SIZE + i])
				{
					int x = posH + i;
					int y = m_Height - posV - j;
					if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
					{
						uint32_t existing_colour = m_FrameBuffer[y * m_Width + x];
						m_FrameBuffer[y * m_Width + x] = colour;
					}
				}
				else
				{
					if (background > 0)
					{
						int x = posH + i;
						int y = m_Height - posV - j;
						if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
						{
							m_FrameBuffer[y * m_Width + x] = background;
						}
					}
				}
			}
		}
		posH += PETSCII_FONT_SIZE;
	}
}


float TextRenderer::pow10(int n)
{
	float result = 1.0;
	if (n > 0)
		for (int i = 0; i < n; i++)
			result *= 10;
	else if (n < 0)
		for (int i = 0; i > n; i--)
			result /= 10;
	return result;
}


std::shared_ptr<int[]> TextRenderer::GetFractionals(double number, int nDecimals)
{
	int dec;
	if (number > 0)
	{
		dec = static_cast<int>((number - static_cast<int>(number))* pow10(nDecimals));
	}
	else dec = 0;
	std::shared_ptr<int[]> decimalDigits(new int[nDecimals]);
	for (int i = 0; i < nDecimals; i++)
	{
		if (i < nDecimals)
		{
			decimalDigits[i] = static_cast<int>(dec / pow10(nDecimals - 1 - i));
			int rem = dec % static_cast<int>(pow10(nDecimals - 1 - i));
			dec = rem;
		}
	}
	return decimalDigits;
}


std::shared_ptr<int[]> TextRenderer::GetIntegers(double number, int* n)
{
	int rawInt = static_cast<int>(number);
	int nDigits = 0;
	while (rawInt > 0)
	{
		rawInt /= 10;
		nDigits++;
	}
	std::shared_ptr<int[]> intDigits(new int[nDigits]);
	rawInt = static_cast<int>(number);
	for (int i = nDigits - 1; i >= 0; i--)
	{
		intDigits[i] = rawInt % 10;
		rawInt /= 10;
	}
	*n = nDigits;
	return intDigits;
	auto p = std::to_string(1);
}


uint32_t TextRenderer::Blend(uint32_t base, uint32_t overlay)
{
	uint8_t rb = (base >> 16) & 0xFF;
	uint8_t gb = (base >> 8) & 0xFF;
	uint8_t bb = base & 0xFF;

	uint8_t ro = (overlay >> 16) & 0xFF;
	uint8_t go = (overlay >> 8) & 0xFF;
	uint8_t bo = overlay & 0xFF;

	uint8_t r = (rb + ro) / 4;
	uint8_t g = (gb + go) / 4;
	uint8_t b = (bb + bo) / 4;

	return (r << 16) | (g << 8) | b;
}


bool* TextRenderer::GetSinglePETSCIICharacter(char char_no)
{
	bool* current;

	switch (char_no)
	{
	case 33:
		current = petscii_1;
		break;
	case 34:
		current = petscii_2;
		break;
	case 35:
		current = petscii_3;
		break;
	case 36:
		current = petscii_4;
		break;
	case 37:
		current = petscii_5;
		break;
	case 38:
		current = petscii_6;
		break;
	case 39:
		current = petscii_7;
		break;
	case 40:
		current = petscii_8;
		break;
	case 41:
		current = petscii_9;
		break;
	case 42:
		current = petscii_10;
		break;
	case 43:
		current = petscii_11;
		break;
	case 44:
		current = petscii_12;
		break;
	case 45:
		current = petscii_13;
		break;
	case 46:
		current = petscii_14;
		break;
	case 47:
		current = petscii_15;
		break;
	case 48:
		current = petscii_16;
		break;
	case 49:
		current = petscii_17;
		break;
	case 50:
		current = petscii_18;
		break;
	case 51:
		current = petscii_19;
		break;
	case 52:
		current = petscii_20;
		break;
	case 53:
		current = petscii_21;
		break;
	case 54:
		current = petscii_22;
		break;
	case 55:
		current = petscii_23;
		break;
	case 56:
		current = petscii_24;
		break;
	case 57:
		current = petscii_25;
		break;
	case 58:
		current = petscii_26;
		break;
	case 59:
		current = petscii_27;
		break;
	case 60:
		current = petscii_28;
		break;
	case 61:
		current = petscii_29;
		break;
	case 62:
		current = petscii_30;
		break;
	case 63:
		current = petscii_31;
		break;
	case 64:
		current = petscii_32;
		break;
	case 65:
		current = petscii_65;
		break;
	case 66:
		current = petscii_66;
		break;
	case 67:
		current = petscii_67;
		break;
	case 68:
		current = petscii_68;
		break;
	case 69:
		current = petscii_69;
		break;
	case 70:
		current = petscii_70;
		break;
	case 71:
		current = petscii_71;
		break;
	case 72:
		current = petscii_72;
		break;
	case 73:
		current = petscii_73;
		break;
	case 74:
		current = petscii_74;
		break;
	case 75:
		current = petscii_75;
		break;
	case 76:
		current = petscii_76;
		break;
	case 77:
		current = petscii_77;
		break;
	case 78:
		current = petscii_78;
		break;
	case 79:
		current = petscii_79;
		break;
	case 80:
		current = petscii_80;
		break;
	case 81:
		current = petscii_81;
		break;
	case 82:
		current = petscii_82;
		break;
	case 83:
		current = petscii_83;
		break;
	case 84:
		current = petscii_84;
		break;
	case 85:
		current = petscii_85;
		break;
	case 86:
		current = petscii_86;
		break;
	case 87:
		current = petscii_87;
		break;
	case 88:
		current = petscii_88;
		break;
	case 89:
		current = petscii_89;
		break;
	case 90:
		current = petscii_90;
		break;
	case 97:
		current = petscii_33;
		break;
	case 98:
		current = petscii_34;
		break;
	case 99:
		current = petscii_35;
		break;
	case 100:
		current = petscii_36;
		break;
	case 101:
		current = petscii_37;
		break;
	case 102:
		current = petscii_38;
		break;
	case 103:
		current = petscii_39;
		break;
	case 104:
		current = petscii_40;
		break;
	case 105:
		current = petscii_41;
		break;
	case 106:
		current = petscii_42;
		break;
	case 107:
		current = petscii_43;
		break;
	case 108:
		current = petscii_44;
		break;
	case 109:
		current = petscii_45;
		break;
	case 110:
		current = petscii_46;
		break;
	case 111:
		current = petscii_47;
		break;
	case 112:
		current = petscii_48;
		break;
	case 113:
		current = petscii_49;
		break;
	case 114:
		current = petscii_50;
		break;
	case 115:
		current = petscii_51;
		break;
	case 116:
		current = petscii_52;
		break;
	case 117:
		current = petscii_53;
		break;
	case 118:
		current = petscii_54;
		break;
	case 119:
		current = petscii_55;
		break;
	case 120:
		current = petscii_56;
		break;
	case 121:
		current = petscii_57;
		break;
	case 122:
		current = petscii_58;
		break;
	case 91:
		current = petscii_59;
		break;
	case 93:
		current = petscii_61;
		break;
	case'_':
		current = petscii_111;
		break;
	default:
		current = petscii_0;
		break;
	}

	return current;
}