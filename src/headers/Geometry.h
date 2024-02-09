#pragma once


#include <cstdint>
#include <cmath>
#include <vector>


#define EPSILON     1e-7f


struct vect2_t
{
	float x = 0.0f;
	float y = 0.0f;
};


struct vect3_t
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};


struct anchor2_t
{
    void move(const vect2_t& m)
    {
        for (auto& d : dependants)
        {
            d->x += m.x;
            d->y += m.y;
        }
    }

    vect2_t                 position;

    std::vector<vect2_t*>   dependants;
};


struct line2_t
{
    line2_t()
        :
        S({ 0.0f, 0.0f }),
        E({ 0.0f, 0.0f }),
        colour(0x00FFFFFF),
        selected(false)
    {
    }

    line2_t(const vect2_t& s, const vect2_t& e)
        :
        S(s),
        E(e),
        colour(0x00FFFFFF),
        selected(false)
    {
    }

    line2_t(const vect2_t& s, const vect2_t& e, uint32_t col)
        :
        S(s),
        E(e),
        colour(col),
        selected(false)
    {
    }

    line2_t(const line2_t& other)
        :
        S(other.S),
        E(other.E),
        colour(other.colour),
        selected(false)
    {
    }

    line2_t& operator=(const line2_t& other)
    {
        S = other.S;
        E = other.E;
        colour = other.colour;
        selected = false;

        return *this;
    }

    vect2_t S = { 0.0f, 0.0f };
    vect2_t E = { 0.0f, 0.0f };

    uint32_t colour = 0x00FFFFFF;

    bool selected = false;
};


inline float LengthSquared(const line2_t& l)
{
    return (l.S.x - l.E.x) * (l.S.x - l.E.x) + (l.S.y - l.E.y) * (l.S.y - l.E.y);
}


inline float Length(const line2_t& l)
{
    return sqrtf(LengthSquared(l));
}


inline float DistanceSquared(const vect2_t& A, const vect2_t& B)
{
    return (A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y);
}


inline int PixelDistanceSquared(const screen_coord_t& A, const screen_coord_t& B)
{
    return (A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y);
}


inline float Distance(const vect2_t& A, const vect2_t& B)
{
    return sqrtf(DistanceSquared(A, B));
}


inline bool Intersect(const line2_t& p, const line2_t& q, vect2_t& x)
{
    float a_p = p.E.y - p.S.y;
    float b_p = p.S.x - p.E.x;
    float c_p = a_p * p.S.x + b_p * p.S.y;

    float a_q = q.E.y - q.S.y;
    float b_q = q.S.x - q.E.x;
    float c_q = a_q * q.S.x + b_q * q.S.y;

    float determinant = a_p * b_q  - a_q * b_p;

    if (fabs(determinant) < EPSILON)
    {
        return false;
    }

    x.x = (b_q * c_p - b_p * c_q) / determinant;
    x.y = (a_p * c_q - a_q * c_p) / determinant;

    return true;
}


inline vect2_t HalfwayPoint(const vect2_t& A, const vect2_t& B)
{
    return { (A.x + B.x) * 0.5f, (A.y + B.y) * 0.5f };
}


union mat3x3_t
{
    struct
    {
        float _00;
        float _01;
        float _02;
        
        float _10;
        float _11;
        float _12;
        
        float _20;
        float _21;
        float _22;
    };

    float elements[9];

    float coeff[3][3];

    mat3x3_t() {}

    mat3x3_t(float scalar)
        :
        _00(scalar),
        _01(0.0f),
        _02(0.0f),
        _10(0.0f),
        _11(scalar),
        _12(0.0f),
        _20(0.0f),
        _21(0.0f),
        _22(scalar)
    {
    }

    mat3x3_t(const mat3x3_t& other)
    {
        for (int i = 0; i < 9; ++i)
        {
            elements[i] = other.elements[i];
        }
    }

    mat3x3_t& operator = (const mat3x3_t& other)
    {
        for (int i = 0; i < 9; ++i)
        {
            elements[i] = other.elements[i];
        }

        return *this;
    }

	vect2_t operator * (const vect2_t& v) const
	{
		vect3_t t;

		t.x = v.x * _00 + v.y * _01 + _02;
		t.y = v.x * _10 + v.y * _11 + _12;
		t.z = v.x * _20 + v.y * _21 + _22;

        if (std::fabs(t.z) < EPSILON)
        {
            return { 0.0f, 0.0f };
        }

        t.x /= t.z;
        t.y /= t.z;

		return { t.x, t.y };
	}

	vect3_t operator * (const vect3_t& v) const
	{
		vect3_t t;

		t.x = v.x * _00 + v.y * _01 + v.z * _02;
		t.y = v.x * _10 + v.y * _11 + v.z * _12;
		t.z = v.x * _20 + v.y * _21 + v.z * _22;

		return t;
	}

	mat3x3_t operator * (const mat3x3_t& m) const
	{
		mat3x3_t tmp;

		tmp._00 = _00 * m._00 + _01 * m._10 + _02 * m._20;
		tmp._01 = _00 * m._01 + _01 * m._11 + _02 * m._21;
		tmp._02 = _00 * m._02 + _01 * m._12 + _02 * m._22;

		tmp._10 = _10 * m._00 + _11 * m._10 + _12 * m._20;
		tmp._11 = _10 * m._01 + _11 * m._11 + _12 * m._21;
		tmp._12 = _10 * m._02 + _11 * m._12 + _12 * m._22;

		tmp._20 = _20 * m._00 + _21 * m._10 + _22 * m._20;
		tmp._21 = _20 * m._01 + _21 * m._11 + _22 * m._21;
		tmp._22 = _20 * m._02 + _21 * m._12 + _22 * m._22;

		return tmp;
	}
};


inline bool MultiplyMatrices(
    const float* A,
    const float* B,
    int num_rows_a,
    int num_rows_b,
    int num_cols_a,
    int num_cols_b,
    float* R
)
{
    if (num_cols_a != num_rows_b)
    {
        return false;
    }

    for (int i = 0; i < num_rows_a; i++)
    {
        for (int j = 0; j < num_cols_b; j++)
        {
            R[i * num_cols_b + j] = 0.0f;

            for (int k = 0; k < num_cols_a; k++)
            {
                R[i * num_cols_b + j] += A[i * num_cols_a + k] * B[k * num_cols_b + j];
            }
        }
    }

    return true;
}


inline bool FindInverseMatrix(
    const float in[],
    float out[],
    int num_rows,
    int num_columns
)
{
    float* augmented = new float[num_rows * 2 * num_columns];
    if (!augmented)
    {
        return false;
    }

    memset(augmented, 0, num_rows * 2 * num_columns * sizeof(float));

    // Initialising elemets of the augmented matrix with identity on the right
    for (int j = 0; j < num_rows; j++)
    {
        for (int i = 0; i < num_columns; i++)
        {
            augmented[j * num_columns * 2 + i] = in[j * num_columns + i];

            if (i == j)
            {
                augmented[j * num_columns * 2 + i + num_columns] = 1.0f;
            }
        }
    }

    // Initialising the order of rows
    std::vector<int> row_order;
    for (int i = 0; i < num_rows; i++)
    {
        row_order.push_back(i);
    }

    for (int r = 0; r < num_rows - 1; r++)
    {
        // We don't want to divide by zero in the next step!
        if (std::fabs(augmented[row_order[r] * num_columns * 2 + r]) < EPSILON)
        {
            int i = r + 1;
            // We need to swap out our current row with
            // one where the pivot element is not zero
            for (; i <= num_rows; i++)
            {
                // We haven't found a suitable row to swap
                // our current row with, we have to bail!
                if (i == num_rows)
                {
                    delete[] augmented;

                    return false;
                }

                if (std::fabs(augmented[row_order[i] * num_columns * 2 + r]) > EPSILON) // We found a suitable one
                {
                    break;
                }
            }
            // Do the swap
            int temp = row_order[r];        // This is the number of the bad row
            row_order[r] = row_order[i];    // This is the number of the good row, our new current row
            row_order[i] = temp;            // We store the bad one here, hopefully it will be useful later
        }

        for (int j = r + 1; j < num_rows; j++)
        {
            float scalar = -1.0f * augmented[row_order[j] * num_columns * 2 + r] / augmented[row_order[r] * num_columns * 2 + r];
            for (int c = r; c < num_columns * 2; c++)
            {
                // Add row 'r' multiplied by 'scalar' to the row below
                augmented[row_order[j] * num_columns * 2 + c] += (augmented[row_order[r] * num_columns * 2 + c] * scalar);
            }
        }
    }

    // No we do the same thing again but this time from the bottom up
    for (int r = num_rows - 1; r > 0; r--)
    {
        // Swapping is not needed at this point, our diagonal is already sorted out

        for (int j = r - 1; j >= 0; j--)
        {
            float scalar = -1.0f * augmented[row_order[j] * num_columns * 2 + r] / augmented[row_order[r] * num_columns * 2 + r];
            for (int c = r; c < num_columns * 2; c++)
            {
                // Add row 'r' multiplied by 'scalar' to the row above
                augmented[row_order[j] * num_columns * 2 + c] += (augmented[row_order[r] * num_columns * 2 + c] * scalar);
            }
        }
    }

    // All we need to do no is to normalise the result
    for (int j = 0; j < num_rows; j++)
    {
        float scalar = augmented[j * num_columns * 2 + j];

        if (std::fabs(scalar) < EPSILON)
        {
            // This should never happen!
            delete[] augmented;

            return false;
        }

        for (int i = 0; i < num_columns * 2; i++)
        {
            augmented[j * num_columns * 2 + i] /= scalar;
        }
    }

    for (int j = 0; j < num_rows; j++)
    {
        for (int i = 0; i < num_columns; i++)
        {
            out[j * num_columns + i] = augmented[j * num_columns * 2 + i + num_columns];
        }
    }

    delete[] augmented;

    return true;
}


inline bool SolveEquations(
    float augmented_matrix[],
    float solution[],
    int num_rows,
    int num_columns
)
{
    std::vector<int> row_order;
    for (int i = 0; i < num_rows; i++)
    {
        row_order.push_back(i);
    }

    for (int r = 0; r < num_rows - 1; r++)
    {
        // We don't want to divide by zero in the next step!
        if (std::fabs(augmented_matrix[row_order[r] * num_columns + r]) < EPSILON)
        {
            int i = r + 1;
            // We need to swap out our current row with
            // one where the pivot element is not zero
            for (; i <= num_rows; i++)
            {
                // We haven't found a suitable row to swap
                // our current row with, we have to bail!
                if (i == num_rows)
                {
                    return false;
                }

                if (std::fabs(augmented_matrix[row_order[i] * num_columns + r]) > EPSILON) // We found a suitable one
                {
                    break;
                }
            }
            // Do the swap
            int temp = row_order[r];        // This is the number of the bad row
            row_order[r] = row_order[i];    // This is the number of the good row, our new current row
            row_order[i] = temp;            // We store the bad one here, hopefully it will be useful later
        }

        for (int j = r + 1; j < num_rows; j++)
        {
            float scalar = -1.0f * augmented_matrix[row_order[j] * num_columns + r] / augmented_matrix[row_order[r] * num_columns + r];
            for (int c = r; c < num_columns; c++)
            {
                // Add row 'r' multiplied by 'scalar' to the row below
                augmented_matrix[row_order[j] * num_columns + c] += (augmented_matrix[row_order[r] * num_columns + c] * scalar);
            }
        }
    }

    // Now we work out the solution from the bottom row up
    for (int s = num_rows - 1; s >= 0; s--)
    {
        float acc = augmented_matrix[row_order[s] * num_columns + num_columns - 1];
        for (int c = num_columns - 2; c > s; c--)
        {
            acc -= (augmented_matrix[row_order[s] * num_columns + c] * solution[c]);
        }
        solution[s] = acc / (augmented_matrix[row_order[s] * num_columns + s]);
    }

    return true;
}


inline bool CalculateHomography(
        const std::vector<vect2_t>& original_points,
        const std::vector<vect2_t>& transformed_points,
        mat3x3_t& homography
)
{
    homography = mat3x3_t(1.0f);

    float augmented[9 * 10] = { 0.0f };

    for (int i = 0; i < 4; i++)
    {
        augmented[(i * 2 + 0) * 10 + 0] = -1.0f * original_points[i].x;
        augmented[(i * 2 + 0) * 10 + 1] = -1.0f * original_points[i].y;
        augmented[(i * 2 + 0) * 10 + 2] = -1.0f;
        augmented[(i * 2 + 0) * 10 + 3] =  0.0f;
        augmented[(i * 2 + 0) * 10 + 4] =  0.0f;
        augmented[(i * 2 + 0) * 10 + 5] =  0.0f;
        augmented[(i * 2 + 0) * 10 + 6] =  transformed_points[i].x * original_points[i].x;
        augmented[(i * 2 + 0) * 10 + 7] =  transformed_points[i].x * original_points[i].y;
        augmented[(i * 2 + 0) * 10 + 8] =  transformed_points[i].x;
        augmented[(i * 2 + 0) * 10 + 9] =  0.0f;

        augmented[(i * 2 + 1) * 10 + 0] =  0.0f;
        augmented[(i * 2 + 1) * 10 + 1] =  0.0f;
        augmented[(i * 2 + 1) * 10 + 2] =  0.0f;
        augmented[(i * 2 + 1) * 10 + 3] = -1.0f * original_points[i].x;
        augmented[(i * 2 + 1) * 10 + 4] = -1.0f * original_points[i].y;
        augmented[(i * 2 + 1) * 10 + 5] = -1.0f;
        augmented[(i * 2 + 1) * 10 + 6] =  transformed_points[i].y * original_points[i].x;
        augmented[(i * 2 + 1) * 10 + 7] =  transformed_points[i].y * original_points[i].y;
        augmented[(i * 2 + 1) * 10 + 8] =  transformed_points[i].y;
        augmented[(i * 2 + 1) * 10 + 9] =  0.0f;
    }

    augmented[8 * 10 + 0] = 0.0f;
    augmented[8 * 10 + 1] = 0.0f;
    augmented[8 * 10 + 2] = 0.0f;
    augmented[8 * 10 + 3] = 0.0f;
    augmented[8 * 10 + 4] = 0.0f;
    augmented[8 * 10 + 5] = 0.0f;
    augmented[8 * 10 + 6] = 0.0f;
    augmented[8 * 10 + 7] = 0.0f;
    augmented[8 * 10 + 8] = 1.0f;
    augmented[8 * 10 + 9] = 1.0f;

    float solution[9] = { 0.0f };

    if (!SolveEquations(augmented, solution, 9, 10))
    {
        return false;
    }

    for (int i = 0; i < 9; i++)
    {
        homography.elements[i] = solution[i];
    }

    return true;
}