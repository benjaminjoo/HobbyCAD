#pragma once


#include <cstring>


namespace Utilities
{


inline const std::string TruncDecimals(const float& value, int decimals)
{
	std::string result = "";
	std::string raw_str = "";
	try
	{
		raw_str = std::to_string(value);
	}
	catch (const std::exception & e)
	{
		raw_str = "0.00";
	}

	int decimal_count = 0;
	bool decimal_point_found = false;
	for (const auto& ch : raw_str)
	{
		if (decimal_point_found)
		{
			if (decimal_count < decimals)
			{
				result += ch;
				decimal_count++;
			}
		}
		else
		{
			if (ch == '.')
			{
				decimal_point_found = true;
			}
			result += ch;
		}
	}

	return result;
}


inline const std::string PadFront(
	const std::string& raw_str,
	std::size_t desired_sz,
	char padding_ch
)
{
	std::string result = "";

	if (raw_str.length() >= desired_sz)
	{
		return raw_str;
	}

	std::size_t padding_required = desired_sz - raw_str.length();
	for (auto i = 0ULL; i < padding_required; i++)
	{
		result += padding_ch;
	}

	result += raw_str;

	return result;
}


};