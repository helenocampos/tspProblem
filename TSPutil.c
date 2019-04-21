/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdlib.h>

typedef struct NeighborDistance{
    int target;
    double distanceFromOrigin;
} neighbor_distance;

int distance_comparator(const void *v1, const void *v2)
{
    struct NeighborDistance *p1 = (struct NeighborDistance *)v1;
    struct NeighborDistance *p2 = (struct NeighborDistance *)v2;
    if (p1->distanceFromOrigin < p2->distanceFromOrigin)
        return -1;
    else if (p1->distanceFromOrigin > p2->distanceFromOrigin)
        return +1;
    else
        return 0;
}

struct NeighborDistance** sortNeighborsByDistance(struct NeighborDistance *neighbors, int amount){
    qsort(neighbors, amount,  sizeof (struct NeighborDistance), distance_comparator);
    return neighbors;
}