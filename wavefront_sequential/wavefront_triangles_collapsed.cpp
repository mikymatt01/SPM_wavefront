#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

void printMatrix(std::vector<double> M, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

inline void init_matrix(std::vector<double> &M, int n)
{
    int value = 0;
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
        {
            M[i * n + j] = value;
            value++;
        }
    }
}

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

std::vector<std::vector<triangle *>>
divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw)
{
    std::vector<std::vector<triangle *>> triangles;
    std::vector<triangle *> triangles_straight;
    std::vector<triangle *> triangles_reversed;

    for (int i = 1; i < n;)
    {
        int d = n - i;
        int start_index = i;
        int n_straight_triangles = d > nw ? nw : d;
        int quotient = d / nw;
        int remainder = d - nw * quotient;

        for (int i_triangle = 0; i_triangle < n_straight_triangles; i_triangle++)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            a->start_index = start_index;
            a->size_side = remainder > 0 ? quotient + 1 : quotient;
            a->is_diag = true;

            remainder--;
            start_index = a->size_side * n + a->size_side + start_index;
            triangles_straight.push_back(a);

            if (i_triangle == 0)
                i += triangles_straight[0]->size_side;
            if (i_triangle == 1)
                i += triangles_straight[1]->size_side == triangles_straight[0]->size_side ? 1 : 0;
        }

        for (int i_triangle = 0; i_triangle < n_straight_triangles - 1; ++i_triangle)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            if (i_triangle == 0)
                a->size_side = min(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
            else
                a->size_side = triangles_reversed[i_triangle - 1]->size_side;

            a->start_index = triangles_straight[i_triangle + 1]->start_index - n;
            a->is_diag = false;

            triangles_reversed.push_back(a);
        }

        triangles.push_back(triangles_straight);
        triangles.push_back(triangles_reversed);

        triangles_straight.clear();
        triangles_reversed.clear();
    }

    return triangles;
}

void printTriangle(triangle t)
{
    std::cout << "start_index: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.size_side << std::endl;
    std::cout << "is_diag: " << t.is_diag << std::endl;
}

void printArray(std::vector<float> x, int n)
{
    for (int i = 0; i < n; i++)
        std::cout << x[i] << " ";
    if (n > 0)
        std::cout << std::endl;
}

void iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    int n_t = std::floor((float)((t.size_side * t.size_side) / 2)) + std::ceil((float)t.size_side / 2);
    int n_s = t.size_side;
    int n_d = t.size_side;
    int j = t.start_index;
    int step = 0;
    for (int i_t = 0; i_t < n_t; i_t++)
    {
        int row = std::floor((float)j / n);
        int col = j % n;
        double res = 0.0;
        for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col)
        {
            res += M[start_row] * M[start_col];
        }
        res = cbrt(res);
        M[j] = res;
        M[col * n + row] = res;

        step = std::floor(float(i_t + 1) / n_s) == 1 ? step + 1 : step;
        n_d = std::floor(float(i_t + 1) / n_s) == 1 ? n_d - 1 : n_d;
        j = std::floor(float(i_t + 1) / n_s) == 1 ? t.start_index + step : j + n + 1;
        n_s = std::floor(float(i_t + 1) / n_s) == 1 ? n_s + n_d : n_s;
    }
}

void iterate_on_matrix_by_reversed_triangle(std::vector<double> &M, triangle t, int n)
{
    int n_t = std::floor((float)((t.size_side * t.size_side) / 2)) + std::ceil((float)t.size_side / 2);
    int i_d = 1;
    int d = 1;
    int j = t.start_index;
    int step = 0;
    for (int i_t = 0; i_t < n_t && j < std::ceil((float)t.start_index / n) * n; i_t++)
    {
        int row = std::floor((float)j / n);
        int col = j % n;
        double res = 0.0;
        for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col)
        {
            res += M[start_row] * M[start_col];
        }
        res = cbrt(res);
        M[j] = res;
        M[col * n + row] = res;

        i_d -= 1;
        step = i_d == 0 ? step + 1 : step;
        j = i_d == 0 ? t.start_index - (n * step) : j + n + 1;
        d = i_d == 0 ? d + 1 : d;
        i_d = i_d == 0 ? d : i_d;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "./" << argv[0] << " <n> <nt>" << std::endl;
        return -1;
    }
    std::cout << "start execution" << std::endl;

    int n = atoi(argv[1]);
    std::vector<double> M(n * n, 1);
    ssize_t ntriangles = atoi(argv[2]); // ff_numCores();

    auto start_compute = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(M, n, ntriangles);

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    for (int i = 0; i < (int)triangles.size(); i++)
    {
        for (int j = 0; j < (int)triangles[i].size(); j++)
        {
            if (triangles[i][j]->is_diag) 
                iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
            else
                iterate_on_matrix_by_reversed_triangle(M, *triangles[i][j], n);
            delete triangles[i][j];
            triangles[i][j] = nullptr;
        }
    }
    auto end_compute = std::chrono::high_resolution_clock::now();
    auto duration_compute = std::chrono::duration_cast<std::chrono::milliseconds>(end_compute - start_compute);

    std::cout << "time: " << duration_compute.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << "last: " << M[n - 1] << std::endl;
    return 0;
}
