/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TSPutil.h
 * Author: helenocampos
 *
 * Created on 12 de Abril de 2019, 12:30
 */
typedef struct NeighborDistance{
    int target;
    int distanceFromOrigin;
} neighbor_distance;
extern struct NeighborDistance* sortNeighborsByDistance(struct NeighborDistance *neighbors, int amount);

