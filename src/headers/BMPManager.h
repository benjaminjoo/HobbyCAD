#pragma once


#include <fstream>


#include <Windows.h>


typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;


#pragma pack(push, 1)
struct _WinBMPFileHeader
{
	WORD   FileType;
	DWORD  FileSize;
	WORD   Reserved1;
	WORD   Reserved2;
	DWORD  BitmapOffset;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct _Win4xBitmapHeader
{
	DWORD Size;
	LONG  Width;
	LONG  Height;
	WORD  Planes;
	WORD  BitsPerPixel;
	DWORD Compression;
	DWORD SizeOfBitmap;
	LONG  HorzResolution;
	LONG  VertResolution;
	DWORD ColorsUsed;
	DWORD ColorsImportant;
};
#pragma pack(pop)


union bmp32_t
{
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };

    uint32_t colour;
};


struct texture_t
{
	uint32_t* buffer;
	int w;
	int h;

    uint32_t GetSample(float x, float y, int dx, int dy) const
    {
        if (buffer == nullptr)
        {
            return 0x00FF0000;
        }

        // If any of the sample coordinates are out of bounds, return black
        if ((x > 1.0f) || (y > 1.0f) || (x < -1.0f) || (y < -1.0f))
        {
            return 0x0000FF00;
        }

        // Convert sample coordinates to image coordinates
        int cx = static_cast<int>(static_cast<float>(w) * x);
        int cy = static_cast<int>(static_cast<float>(h) * y);

        if ((cx >= 0) && (cy >= 0) && (cx < w) && (cy < h))
        {
            return buffer[cy * w + cx];
        }
        else
        {
            return 0;
        }

        int sample_count = 0;
        uint32_t b_acc = 0;
        uint32_t g_acc = 0;
        uint32_t r_acc = 0;
        for (int j = cy - (dy / 2); j <= cy + (dy / 2); j++)
        {
            for (int i = cx - (dx / 2); i <= cx + (dx / 2); i++)
            {
                bmp32_t pixel = { 0 };

                if ((i >= 0) && (j >= 0) && (i < w) && (j < h))
                {
                    pixel.colour = buffer[j * w + i];

                    b_acc += pixel.b;
                    g_acc += pixel.g;
                    r_acc += pixel.r;

                    sample_count++;
                }
            }
        }

        if (sample_count == 0)
        {
            return 0x000000FF;
        }

        float scale = 1.0f / static_cast<float>(sample_count);

        bmp32_t result = { 0 };

        result.b = static_cast<uint8_t>(static_cast<float>(b_acc) * scale);
        result.g = static_cast<uint8_t>(static_cast<float>(g_acc) * scale);
        result.r = static_cast<uint8_t>(static_cast<float>(r_acc) * scale);

        return result.colour;
    }
};


class BMPManager
{
public:

	static bool ReadBitMapData(const std::string& filename, texture_t& txt);
	static bool WriteBitMapData(const std::string& filename, texture_t& txt);

};