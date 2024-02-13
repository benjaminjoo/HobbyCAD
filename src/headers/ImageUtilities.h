#pragma once


#include "Canvas.h"
#include "Geometry.h"
#include "BMPManager.h"


#define SAMPLE_W            5
#define SAMPLE_H            5


#define SAMPLE_SIZE         9


namespace Image
{


inline bool GetIntensityAndThresholdBuffer(
    const texture_t& txt,
    float*& i_buffer,
    uint8_t*& t_buffer,
    int threshold_step,
    bool reduce_noise = false
)
{
    if (txt.buffer == nullptr)
    {
        return false;
    }

    //float image_w = static_cast<float>(txt.w);
    //float image_h = static_cast<float>(txt.h);

    int image_sz = txt.w * txt.h;

    i_buffer = new float[image_sz];
    t_buffer = new uint8_t[image_sz];

    if ((i_buffer == nullptr) || (t_buffer == nullptr))
    {
        return false;
    }

    for (int i = 0; i < image_sz; i++)
    {
        bmp32_t temp;
        temp.colour = txt.buffer[i];

        float i_r = static_cast<float>(temp.r) / 255.0f;
        float i_g = static_cast<float>(temp.g) / 255.0f;
        float i_b = static_cast<float>(temp.b) / 255.0f;

        float shade = 0.299f * i_r + 0.587f * i_g + 0.114f * i_b;

        t_buffer[i] = static_cast<uint8_t>(255.0f * shade) / threshold_step * threshold_step;

        i_buffer[i] = shade;
    }

    if (reduce_noise)
    {
        int r_x = SAMPLE_W / 2;
        int r_y = SAMPLE_H / 2;

        for (int j = 0 + r_y; j < txt.h - r_y; j++)
        {
            for (int i = 0 + r_x; i < txt.w - r_x; i++)
            {
                float intensity_acc = 0.0f;
                int num_values = 0;
                for (int q = j - r_y; q < j + r_y; q++)
                {
                    for (int p = i - r_x; p < i + r_x; p++)
                    {
                        intensity_acc += i_buffer[q * txt.w + p];
                        num_values++;
                    }
                }
                if (num_values == 0)
                {
                    break;
                }
                float average_intensity = intensity_acc / static_cast<float>(num_values);

                int num_outliers = 0;
                for (int q = j - r_y; q < j + r_y; q++)
                {
                    for (int p = i - r_x; p < i + r_x; p++)
                    {
                        // If pixel is more than 10% different from the average, it qualifies as an outlier
                        if (std::fabs(i_buffer[q * txt.w + p] - average_intensity) > 0.1f)
                        {
                            num_outliers++;
                        }
                    }
                }

                // If there're less than 5% outliers...
                if ((static_cast<float>(num_outliers) / static_cast<float>(num_values)) < 0.05f)
                {
                    // ...then we paint over those outliers to make them the same as the rest of the pack 
                    for (int q = j - r_y; q < j + r_y; q++)
                    {
                        for (int p = i - r_x; p < i + r_x; p++)
                        {
                            t_buffer[q * txt.w + p] = static_cast<uint8_t>(255.0f * average_intensity) / threshold_step * threshold_step;
                        }
                    }
                }
            }
        }
    }

    return true;
}


inline bool FindVertices(
    const texture_t& txt,
    std::vector<vect2_t>& vertices,
    float*& i_buffer,
    uint8_t*& t_buffer
)
{
    if (txt.buffer == nullptr)
    {
        return false;
    }

    float image_w = static_cast<float>(txt.w);
    float image_h = static_cast<float>(txt.h);

    if ((i_buffer == nullptr) || (t_buffer == nullptr))
    {
        return false;
    }

    int r_x = SAMPLE_W / 2;
    int r_y = SAMPLE_H / 2;

    for (int j = 0 + r_y; j < txt.h - r_y; j++)
    //for (int j = 0 + r_y; j < txt.h - r_y; j += SAMPLE_H)
    {
        for (int i = 0 + r_x; i < txt.w - r_x; i++)
        //for (int i = 0 + r_x; i < txt.w - r_x; i += SAMPLE_W)
        {
            uint8_t candidate = t_buffer[j * txt.w + i];

            int num_less = 0;
            int num_more = 0;
            int num_same = 0;
            for (int q = j - r_y; q < j + r_y; q++)
            {
                for (int p = i - r_x; p < i + r_x; p++)
                {
                    uint8_t current = t_buffer[q * txt.w + p];
                    if (current < candidate)
                    {
                        num_less++;
                    }
                    else if (current > candidate)
                    {
                        num_more++;
                    }
                    else
                    {
                        num_same++;
                    }
                }
            }
            if (
                std::abs(num_less - num_more) == 1
            )
            {
                //float x = -250.0f + static_cast<float>(i) / static_cast<float>(txt.w) * 500.0f;
                //float y = -250.0f + static_cast<float>(j) / static_cast<float>(txt.h) * 500.0f;
                float x = -0.5f * image_w + static_cast<float>(i) / static_cast<float>(txt.w) * image_w;
                float y = -0.5f * image_h + static_cast<float>(j) / static_cast<float>(txt.h) * image_h;

                vertices.push_back({ x, y });
            }
        }
    }

    return true;
}


inline bool FindVerticesInsideSquares(
    const texture_t& txt,
    std::vector<vect2_t>& vertices,
    float*& i_buffer,
    uint8_t*& t_buffer
)
{
    if (txt.buffer == nullptr)
    {
        return false;
    }

    float image_w = static_cast<float>(txt.w);
    float image_h = static_cast<float>(txt.h);

    if ((i_buffer == nullptr) || (t_buffer == nullptr))
    {
        return false;
    }

    int r_x = SAMPLE_W / 2;
    int r_y = SAMPLE_H / 2;

    for (int j = 0 + r_y; j < txt.h - r_y; j += SAMPLE_H)
    {
        for (int i = 0 + r_x; i < txt.w - r_x; i += SAMPLE_W)
        {
            uint8_t t_min = 255;
            uint8_t t_max = 0;
            for (int q = j - r_y; q < j + r_y; q++)
            {
                for (int p = i - r_x; p < i + r_x; p++)
                {
                    uint8_t current = t_buffer[q * txt.w + p];

                    if (current < t_min)
                    {
                        t_min = current;
                    }
                    if (current > t_max)
                    {
                        t_max = current;
                    }
                }
            }
            if ((t_max - t_min) > 24)
            {
                //float x = -250.0f + static_cast<float>(i) / static_cast<float>(txt.w) * 500.0f;
                //float y = -250.0f + static_cast<float>(j) / static_cast<float>(txt.h) * 500.0f;
                float x = -0.5f * image_w + static_cast<float>(i) / static_cast<float>(txt.w) * image_w;
                float y = -0.5f * image_h + static_cast<float>(j) / static_cast<float>(txt.h) * image_h;

                vertices.push_back({ x, y });
            }
        }
    }

    return true;
}


inline bool FindVerticesInsideCircles(
    const texture_t& txt,
    std::vector<vect2_t>& vertices,
    float*& i_buffer,
    uint8_t*& t_buffer
)
{
    if (txt.buffer == nullptr)
    {
        return false;
    }

    if ((i_buffer == nullptr) || (t_buffer == nullptr))
    {
        return false;
    }

    int r_x = 8;
    int r_y = 8;

    int d_x[16] = {  0,  3,  6,  7,  8,  7,  6,  3,  0, -3, -6, -7, -8, -7, -6, -3 };
    int d_y[16] = {  8,  7,  6,  3,  0, -3, -6, -7, -8, -7, -6, -3,  0,  3,  6,  7 };

    uint8_t samples[16] = { 0 };

    for (int j = 0 + r_y; j < txt.h - r_y; j += 4)
    {
        for (int i = 0 + r_x; i < txt.w - r_x; i += 4)
        {
            uint8_t candidate = t_buffer[j * txt.w + i];

            // Fill up circular sample buffer
            for (int p = 0; p < 16; p++)
            {
                samples[p] = t_buffer[(j + d_y[p]) * txt.w + (i + d_x[p])];
            }

            // Look for SAMPLE_SIZE number of consecutive pixels lighter or darker than 'candidate'
            for (int p = 0; p < 16 - SAMPLE_SIZE; p++)
            {
                uint8_t segment[SAMPLE_SIZE] = { 0 };

                memcpy(segment, samples + p, SAMPLE_SIZE);

                int n_pixels_lighter = 0;
                int n_pixels_darker = 0;
                for (int q = 0; q < SAMPLE_SIZE; q++)
                {
                    if (segment[q] < candidate)
                    {
                        n_pixels_darker++;
                    }
                    if (segment[q] > candidate)
                    {
                        n_pixels_lighter++;
                    }
                }
                if ((n_pixels_darker == SAMPLE_SIZE) || (n_pixels_lighter == SAMPLE_SIZE))
                {
                    // We've found a possible corner
                    float x = -250.0f + static_cast<float>(i) / static_cast<float>(txt.w) * 500.0f;
                    float y = -250.0f + static_cast<float>(j) / static_cast<float>(txt.h) * 500.0f;

                    vertices.push_back({ x, y });
                }
            }
        }
    }

    return true;
}


}