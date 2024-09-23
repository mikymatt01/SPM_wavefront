#include <iostream>
#include <vector>

typedef struct
{
    int start_index;
    int height;
    int base;
    bool is_half;
} rectangle;

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

inline void iterate_on_matrix_by_triangle(std::vector<double> &M, rectangle t, int n)
{
    if (t.is_half)
    {

        for (int start_index = t.start_index, size_side = t.base; size_side > 0; start_index += n + 1, --size_side)
        {
            for (int index = start_index; index < size_side + start_index; ++index)
            {
                M[index] = m_val;
            }
        }
    }
    else
    {
        for (int h = 0; h < t.height; h++)
        {
            for (int b = 0; b < t.base; b++)
            {
                M[t.start_index + (h * n) + b] = m_val;
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

void printTriangle(rectangle t)
{
    std::cout << "start_row: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.base << std::endl;
    std::cout << "is_half: " << t.is_half << std::endl;
    std::cout << std::endl;
}

inline std::vector<rectangle *> divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw)
{
    std::vector<rectangle *> triangles;
    int d = n;
    int start_index = 0;
    int n_triangles = d > nw ? nw : d;
    int quotient = d / nw;
    int remainder = d - nw * quotient;
    for (int i_triangle = 0; i_triangle < n_triangles; i_triangle++)
    {
        rectangle *a = (rectangle *)malloc(sizeof(rectangle));
        a->start_index = start_index;
        a->base = remainder > 0 ? quotient + 1 : quotient;
        a->height = a->base;
        a->is_half = true;

        remainder--;
        start_index = a->base * n + a->base + start_index;
        triangles.push_back(a);
    }

    for (int i = 0; i + 1 < triangles.size(); i++)
    {
        if (triangles[i]->start_index < triangles[i + 1]->start_index)
        {
            rectangle *a = (rectangle *)malloc(sizeof(rectangle));
            a->start_index = triangles[i]->start_index + triangles[i]->base;
            a->height = triangles[i]->height;
            a->base = triangles[i + 1]->base;
            a->is_half = false;
            triangles.push_back(a);
        }
    }

    return triangles;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "./" << argv[0] << " <n> <nw>" << std::endl;
        return -1;
    }

    std::cout << "start execution" << std::endl;

    int n = atoi(argv[1]);
    int nw = atoi(argv[2]);
    std::vector<double> M(n * n, 1);
    std::vector<rectangle *> triangles;

    init_matrix(M, n);
    // printMatrix(M, n);
    triangles = divide_upper_matrix_into_triangles(M, n, nw);
    for (int i = 0; i < triangles.size(); i++)
    {
        // printTriangle(*triangles[i]);
        iterate_on_matrix_by_triangle(M, *triangles[i], n);
        // printMatrix(M, n);
        if (triangles[i]->is_half)
            std::cout << (triangles[i]->base * triangles[i]->height) / 2 << std::endl;
        else
            std::cout << triangles[i]->base * triangles[i]->height << std::endl;
        std::cout << std::endl;
    }
    std::cout << "triangles: " << triangles.size() << std::endl;
}