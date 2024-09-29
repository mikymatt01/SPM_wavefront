#include <iostream>
#include <vector>

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

struct Cell
{
    double value;
    bool computed;

    Cell(double _v, bool _c) : value(_v), computed(_c) {}
};

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

void printMatrix(std::vector<Cell> M, uint64_t n)
{
    int total = 0;
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n - k; i++)
        {
            if (!M[i * n + i + k].computed)
            {
                total += 1;
                int j = i * n + i + k;
                int row = std::floor((float)j / n);
                int col = j % n;
                printf("index: %llu, row: %d, column: %d\n", i * n + i + k, row, col);
            }
        }
    }
    std::cout << "total: " << total << std::endl;
}

void iterate_on_matrix_by_triangle(std::vector<Cell> &M, triangle t, int n)
{
    if (t.is_diag)
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

            M[j].value = 10;
            M[j].computed = true;

            M[col * n + row].value = 10;
            M[col * n + row].computed = true;

            step = std::floor(float(i_t + 1) / n_s) == 1 ? step + 1 : step;
            n_d = std::floor(float(i_t + 1) / n_s) == 1 ? n_d - 1 : n_d;
            j = std::floor(float(i_t + 1) / n_s) == 1 ? t.start_index + step : j + n + 1;
            n_s = std::floor(float(i_t + 1) / n_s) == 1 ? n_s + n_d : n_s;
        }
    }

    if (!t.is_diag)
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

            M[j].value = 10;
            M[j].computed = true;

            M[col * n + row].value = 10;
            M[col * n + row].computed = true;

            i_d -= 1;
            step = i_d == 0 ? step + 1 : step;
            j = i_d == 0 ? t.start_index - (n * step) : j + n + 1;
            d = i_d == 0 ? d + 1 : d;
            i_d = i_d == 0 ? d : i_d;
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

std::vector<std::vector<triangle *>>
divide_upper_matrix_into_triangles(int n, int nw)
{
    std::vector<std::vector<triangle *>> triangles;
    std::vector<triangle *> triangles_straight;
    std::vector<triangle *> triangles_reversed;

    for (int i = 0; i < n;)
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
    std::vector<Cell> M(n * n, Cell(1, false));
    std::vector<std::vector<triangle *>> triangles;
    for (int m = 0; m < n; m++)
    {
        M[m * n + m].value = static_cast<double>(m + 1) / n;
        M[m * n + m].computed = true;
    }
    triangles = divide_upper_matrix_into_triangles(n, nw);
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < triangles[i].size(); j++)
        {
            iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
            // std::cout << (triangles[i]->size_side * triangles[i]->size_side) / 2 << std::endl;
        }
    }
    std::cout << "triangles: " << triangles.size() << std::endl;
    printMatrix(M, n);
}