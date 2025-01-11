#include <iostream>
#include <vector>

#ifndef MATHUTILS_H
#define MATHUTILS_H

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

std::vector<triangle *> divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw);

#endif // MATHUTILS_H