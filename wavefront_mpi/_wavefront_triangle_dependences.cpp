#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

void printMatrix(std::vector<double> M, int n, int m)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

void printTriangle(triangle t)
{
    std::cout << "start_index: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.size_side << std::endl;
    std::cout << "is_diag: " << t.is_diag << std::endl;
}

void printArray(std::vector<double> x, int n)
{
    for (int i = 0; i < n; i++)
        std::cout << x[i] << " ";
    if (n > 0)
        std::cout << std::endl;
}

inline int min(int a, int b)
{
    return a > b ? b : a;
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

std::vector<std::vector<triangle *>>
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
        int remainder = d % nw;

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

void iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    if (t.is_diag)
    {
        for (int i = 0; i < t.size_side; i++)
        {
            int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
            int j = t.start_index + i;
            for (int row = j / n, col = j % n; j < end_cicle && row < col; j += n + 1, row = j / n, col = j % n)
            {
                double res = 0.0;
                for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col)
                    res += M[start_row] * M[start_col];
                res = cbrt(res);

                M[j] = 1;             // res;
                M[col * n + row] = 1; // res;
            }
        }
    }

    if (!t.is_diag)
    {
        for (int i = 0; i < t.size_side; i++)
        {
            int j = t.start_index - (i * n);
            for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
            {
                double res = 0.0;
                for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                    res += M[start_row] * M[start_col];
                res = cbrt(res);

                M[j] = res;
                M[col * n + row] = res;
            }
        }
    }
}

std::pair<std::vector<double>, int> find_triangle_dependences(std::vector<double> &M, int n, triangle t)
{
    int j = t.start_index;
    int row = j / n, col = j % n;
    int k = j - row * n + row;
    std::vector<double> dependences(k * t.size_side * 2);

    int row_el_index = 0;
    int col_el_index = k * t.size_side;

    for (int i = 0; i < t.size_side; i++, j += n + 1)
    {
        row = j / n, col = j % n;
        for (int row_el = row * n + row, col_el = n * k + j; row_el < j; row_el++, col_el--)
        {
            dependences[row_el_index] = M[row_el];
            dependences[col_el_index] = M[col_el];
            M[row_el] = 2;
            M[col_el] = 2;
            row_el_index++;
            col_el_index++;
        }
    }

    return std::make_pair(dependences, k);
}

std::pair<std::vector<double>, int> find_reversed_triangle_dependences(std::vector<double> &M, int n, triangle t)
{
    int j = t.start_index;
    int row = j / n;
    int k = (j + t.size_side * n) - row * n + row;
    std::vector<double> dependences(k * t.size_side * 2);

    int row_el_index = 0;
    int col_el_index = k * t.size_side;

    for (int i = 0, up_side = j, right_side = j; i < t.size_side; i++, up_side -= n, right_side += 1)
    {
        int row_up_side = up_side / n;
        for (int row_el = row_up_side * n + row_up_side; row_el < up_side; row_el++)
        {
            dependences[row_el_index] = M[row_el];
            M[row_el] = 2;
            row_el_index++;
        }

        int row_right_side = right_side / n, col_right_side = right_side % n;
        int opposite_right_side = col_right_side * n + row_right_side;
        for (int col_el = col_right_side * n + col_right_side; col_el > opposite_right_side; col_el--)
        {
            dependences[col_el_index] = M[col_el];
            M[col_el] = 2;
            col_el_index++;
        }
    }

    return std::make_pair(dependences, k);
}

std::vector<double> compute_triangle_using_dependences(triangle t, int n_matrix, std::vector<double> dependences)
{
    std::vector<double> M(t.size_side * t.size_side);
    int row_matrix = t.start_index / n_matrix, col_matrix;
    int k = t.start_index - row_matrix * n_matrix + row_matrix;
    int row_count = 0;
    int col = t.size_side * k;
    for (int diag_cursor = 0, row_count = 1; diag_cursor < t.size_side * t.size_side; diag_cursor += t.size_side + 1, row_count += 1)
    {
        col = t.size_side * k;
        for (int row_cursor = diag_cursor; row_cursor < t.size_side * row_count; row_cursor++)
        {
                double res = 0.0;
                for (int row = 0; row < row_count * t.size_side; row++, col++)
                    res += dependences[row] * dependences[col];
                res = cbrt(res);
                M[row_cursor] = res;
                row_matrix = row_cursor / t.size_side;
                col_matrix = row_cursor % t.size_side;
                M[col_matrix * t.size_side + row_matrix] = res;
        }
    }

    for (int row_cursor = 1; row_cursor < t.size_side; row_cursor++)
    {
        for (int col_cursor = 0; col_cursor < t.size_side - row_cursor; col_cursor++)
        {
            double res = 0.0;
            for (int t = 0; t < row_cursor; ++t)
            {
                res += M[col_cursor * t.size_side + col_cursor + t] * M[(col_cursor + row_cursor) * t.size_side + (col_cursor + row_cursor) - t];
            }
            res = cbrt(res);

            M[col_cursor * t.size_side + col_cursor + row_cursor] = res;
            M[(col_cursor + row_cursor) * t.size_side + col_cursor] = res;
        }
    }

    return M;
}

std::vector<double> compute_reversed_triangle_using_dependences(triangle t, int n_matrix, std::vector<double> dependences)
{
    std::vector<double> M(t.size_side * t.size_side);
    int row_matrix = t.start_index / n_matrix, col_matrix;
    int max_k = (t.start_index + t.size_side * n_matrix) - row_matrix * n_matrix + row_matrix;
    int k = t.start_index - row_matrix * n_matrix + row_matrix;
    int row_count = 0;
    int row_dependence = 0;
    int col_dependence = t.size_side * max_k;
    for (int col_cursor = t.size_side * (t.size_side - 1) + 1; col_cursor >= 0; col_cursor -= t.size_side, row_count += 1, k += 1)
    {
        int diag_count = 0;
        for (int diag_cursor = col_cursor; diag_cursor < t.size_side * (t.size_side - 1) + 1 + row_count; diag_cursor += t.size_side + 1)
        {
            double res = 0.0;
            for (int t = 0; t < k; ++t)
                res += dependences[row_dependence + t] * M[col_dependence + t];
            res = cbrt(res);

            dependences[row_dependence + t] = res;
            dependences[col_dependence + t] = res;

            M[diag_cursor] = res;
            row_matrix = diag_cursor / t.size_side;
            col_matrix = diag_cursor % t.size_side;
            M[col_matrix * t.size_side + row_matrix] = res;

            diag_count++;
            // handle row_dependence and col_dependence
        }
        row_dependence = row_count * max_k;
        col_dependence = t.size_side * max_k;
    }
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
    ssize_t ntriangles = atoi(argv[2]);
    std::vector<double> M(n * n, 0);

    auto start_compute = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(M, n, ntriangles);

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    /*for (int i = 0; i < (int)triangles.size(); i++)
    {
        for (int j = 0; j < (int)triangles[i].size(); j++)
        {*/
    triangle t = *triangles[2][0];
    std::pair<std::vector<double>, int> dependences;
    iterate_on_matrix_by_triangle(M, t, n);
    if (t.is_diag)
    {
        dependences = find_triangle_dependences(M, n, t);
    }
    else
    {
        dependences = find_reversed_triangle_dependences(M, n, t);
    }
    printMatrix(M, n, n);
    std::cout << std::endl;
    printMatrix(dependences.first, dependences.first.size() / dependences.second, dependences.second);
    /*}
}*/
    auto end_compute = std::chrono::high_resolution_clock::now();
    auto duration_compute = std::chrono::duration_cast<std::chrono::milliseconds>(end_compute - start_compute);

    std::cout << "time: " << duration_compute.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << "last: " << M[n - 1] << std::endl;
    return 0;
}
