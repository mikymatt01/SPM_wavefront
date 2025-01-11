#include <iostream>
#include <vector>

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

int m_val = 0;
inline void init_matrix(std::vector<double> &M, int n)
{
    int value = 0;
    for (uint64_t i = 0; i < n; ++i)
    {
        for (uint64_t j = 0; j < n; j++)
        {
            M[i * n + j] = value;
            value++;
        }
    }
}

void printMatrix(std::vector<double> M, uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        for (uint64_t j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

inline void iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    if (t.is_diag)
    {

        for (int start_index = t.start_index, size_side = t.size_side; size_side > 0; start_index += n + 1, --size_side)
        {
            for (int index = start_index; index < size_side + start_index; ++index)
            {
                M[index] = m_val;
            }
        }
    }
    else
    {
        for (int start_index = t.start_index, size_side = t.size_side; size_side > 0; start_index -= n, --size_side)
        {
            for (int index = start_index; index < size_side + start_index && index < std::ceil((float)start_index / n) * n; ++index)
            {
                M[index] = m_val;
            }
        }
    }
}

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

void printTriangle(triangle t)
{
    std::cout << "start_row: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.size_side << std::endl;
    std::cout << "is_diag: " << t.is_diag << std::endl;
    std::cout << std::endl;
}

inline std::vector<triangle *> divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw)
{
    std::vector<triangle *> triangles;
    int n_triangles = 0;
    int d = n;
    for (int i = 0; i < n;)
    {
        d = n - i;
        int start_index = i;
        int n_triangles_per_d = d > nw ? nw : d;
        int quotient = d / nw;
        int remainder = d - nw * quotient;

        for (int i_triangle = 0; i_triangle < n_triangles_per_d; i_triangle++)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            a->start_index = start_index;
            a->size_side = remainder > 0 ? quotient + 1 : quotient;
            a->is_diag = true;

            remainder--;
            start_index = a->size_side * n + a->size_side + start_index;
            triangles.push_back(a);
            n_triangles++;

            if (i_triangle == 0)
                i += triangles[n_triangles - 1]->size_side;
            if (i_triangle == 1)
                i += triangles[n_triangles - 1]->size_side == triangles[n_triangles - 2]->size_side ? 1 : 0;
        }

        int n_lower_triangles = n_triangles;
        for (int i_triangle = n_lower_triangles - n_triangles_per_d; i_triangle < n_lower_triangles - 1; ++i_triangle)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            if (i_triangle + 1 == n_lower_triangles - 1)
                a->size_side = max(triangles[i_triangle]->size_side, triangles[i_triangle + 1]->size_side);
            else
                a->size_side = min(triangles[i_triangle]->size_side, triangles[i_triangle + 1]->size_side);
            a->start_index = triangles[i_triangle + 1]->start_index - n;
            a->is_diag = false;

            triangles.push_back(a);
            n_triangles++;
        }
    }

    return triangles;
}