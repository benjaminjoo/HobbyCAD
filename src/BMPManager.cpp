#include "BMPManager.h"


bool BMPManager::ReadBitMapData(const std::string& filename, texture_t& txt)
{
	std::ifstream picture(filename, std::ifstream::in | std::ifstream::binary);

	if (picture.is_open())
	{
		char rawheader1[sizeof(_WinBMPFileHeader)];
		picture.read(rawheader1, sizeof(_WinBMPFileHeader));
		_WinBMPFileHeader header_1 = *((_WinBMPFileHeader*)&rawheader1);

		char rawheader2[sizeof(_Win4xBitmapHeader)];
		picture.read(rawheader2, sizeof(_Win4xBitmapHeader));
		_Win4xBitmapHeader header_2 = *((_Win4xBitmapHeader*)&rawheader2);

		int width = (int)header_2.Width;
		int height = (int)header_2.Height;
		int offset = header_1.BitmapOffset;

		bmp32_t* bitmap = new bmp32_t[width * height];

		picture.seekg(offset);

		for (auto i = 0; i < width * height; ++i)
		{
			char temp = 0;

			picture.read(&temp, 1);
			uint8_t r = temp;
			bitmap[i].b = temp;

			picture.read(&temp, 1);
			uint8_t g = temp;
			bitmap[i].g = temp;

			picture.read(&temp, 1);
			uint8_t b = temp;
			bitmap[i].r = temp;

			int gr = ((int)r + (int)g + (int)b) / 3;

		}

		picture.close();

		txt.w = width;
		txt.h = height;
		txt.buffer = new uint32_t[width * height];

		for (int i = 0; i < width * height; i++)
		{
			bmp32_t pixel = bitmap[i];
			uint32_t col = pixel.a << 24 | pixel.r << 16 | pixel.g << 8 | pixel.b;
			txt.buffer[i] = col;
		}

		return true;
	}
	else
	{
		std::string error_msg = "Failed to read file: " + filename + "\n";
		OutputDebugStringA(error_msg.c_str());
		return false;
	}

	return false;
}


bool BMPManager::WriteBitMapData(const std::string& filename, const texture_t& txt)
{
	std::ofstream picture(filename, std::ifstream::out | std::ifstream::binary);

	if (picture.is_open())
	{
        
        DWORD row_size = (txt.w * 3) / 4 * 4;
        if (row_size < (txt.w * 3))
        {
            row_size += 4;
        }
        

		_WinBMPFileHeader header_1;
		memset(&header_1, 0, sizeof(_WinBMPFileHeader));

		header_1.FileType = 0x4D42;
		header_1.FileSize = sizeof(_WinBMPFileHeader) + sizeof(_Win4xBitmapHeader) + txt.h * row_size;
		header_1.BitmapOffset = 0x36;

		picture.write((const char*)(&header_1), sizeof(_WinBMPFileHeader));

		_Win4xBitmapHeader header_2;
		memset(&header_2, 0, sizeof(_Win4xBitmapHeader));

		header_2.Size = sizeof(_Win4xBitmapHeader);
		header_2.Width = txt.w;
		header_2.Height = txt.h;
		header_2.Planes = 1;
		header_2.BitsPerPixel = 24;
		header_2.SizeOfBitmap = txt.h * row_size;
        header_2.HorzResolution = 4724;
        header_2.VertResolution = 4724;

		picture.write((const char*)(&header_2), sizeof(_Win4xBitmapHeader));

        for (int j = 0; j < txt.h; j++)
        {
            int bytes_written = 0;
            for (int i = 0; i < txt.w; i++)
            {
                uint32_t pixel = txt.buffer[j * txt.w + i];
                picture.write((const char*)(&pixel), 3);
                bytes_written += 3;
            }
            int padding = row_size - bytes_written;
            for (int p = 0; p < padding; p++)
            {
                char xxx = 0;
                picture.write((const char*)(&xxx), 1);
            }
        }

		picture.close();

		return true;
	}
	else
	{
		return false;
	}
	return false;
}