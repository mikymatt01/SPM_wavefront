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

        for (int i = 0; i < t.size_side; i++)
        {
            for (int j = t.start_index + i; j < (t.size_side * n + t.size_side) + t.start_index - (n * i); j += n + 1)
            {
                M[j] = m_val;
            }
        }
    }
    else
    {
        for (int i = 0; i < t.size_side; i++)
        {
            for (int j = t.start_index - (i * n); j <= t.start_index + i && j < std::ceil((float)t.start_index / n) * n; j += n + 1)
            {
                M[j] = m_val;
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

inline std::vector<std::vector<triangle *>>
divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw)
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

        for (int i_triangle = triangles_straight.size() - n_straight_triangles; i_triangle < triangles_straight.size() - 1; ++i_triangle)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            if (i_triangle + 1 == triangles_straight.size() - 1)
                a->size_side = max(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
            else
                a->size_side = min(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
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
    std::vector<double> M(n * n, 1);
    std::vector<std::vector<triangle *>> triangles;

    init_matrix(M, n);
    triangles = divide_upper_matrix_into_triangles(M, n, nw);
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < triangles[i].size(); j++)
        {
            printTriangle(*triangles[i][j]);
            iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
            printMatrix(M, n);
            // std::cout << (triangles[i]->size_side * triangles[i]->size_side) / 2 << std::endl;
            std::cout << std::endl;
        }
    }
    std::cout << "triangles: " << triangles.size() << std::endl;
}