/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: helenocampos
 *
 * Created on 28 de Março de 2019, 09:32
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/*
 * 
 */

#define BIG_DISTANCE 999999

struct TSPLibData {
    int citiesAmount;
    double *x;
    double *y;
};

struct TSPInstance {
    int citiesAmount;
    double** graphMatrix;
};

int nint(int x) {
    return (int) (x + 0.5);
}

int getEuclidianDistance(int i, int j, struct TSPLibData *dat) {
    if (i < dat->citiesAmount && j < dat->citiesAmount) {
        double xd = dat->x[i] - dat->x[j];
        double yd = dat->y[i] - dat->y[j];
        return nint(sqrt(xd * xd + yd * yd));
    } else {
        return -1;
    }
}

//returns 1 if valid, 0 otherwise

int isValidEuclidian2Dfile(char *filePath) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filePath, "r");
    if (fp) {
        while ((read = getline(&line, &len, fp)) != -1) {
            char delim[] = " ";
            char *ptr = strtok(line, delim);
            while (ptr != NULL) {
                if (strcmp(ptr, "EDGE_WEIGHT_TYPE") == 0) {
                    ptr = strtok(NULL, delim);
                    ptr = strtok(NULL, delim);
                    if (strcmp(ptr, "EUC_2D\n") == 0) {
                        fclose(fp);
                        return 1;
                    }
                }
                ptr = strtok(NULL, delim);
            }
        }
        fclose(fp);
    }
    return 0;
}

void allocateTSPInstanceEuclidian2D(struct TSPLibData *data, struct TSPInstance *instance) {
    instance->citiesAmount = data->citiesAmount;
    instance->graphMatrix = malloc(instance->citiesAmount * sizeof (double*));
    for (int i = 0; i < instance->citiesAmount; i++) {
        instance->graphMatrix[i] = malloc(instance->citiesAmount * sizeof (double));
    }

    //    for (int i = 0; i < instance->citiesAmount; i++) {
    //        free(instance->graphMatrix[i]);
    //    }
    //    free(instance->graphMatrix);
    for (int i = 0; i < instance->citiesAmount; i++) {
        for (int j = 0; j < instance->citiesAmount; j++) {
            instance->graphMatrix[i][j] = getEuclidianDistance(i, j, data);
        }
    }

}

void parseTSPLibFileEuclidian2D(char *filePath, struct TSPLibData *data, struct TSPInstance *instance) { //only valid for Euclidian2D instances
    if (isValidEuclidian2Dfile(filePath)) {
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        int dataSection = 0;
        int vertexAmount = 0;
        int vertexCounter = 0;
        fp = fopen(filePath, "r");
        if (fp) {
            while ((read = getline(&line, &len, fp)) != -1) {
                //printf("Retrieved line of length %zu:\n", read);
                //printf("\n %s tokens: ", line);
                if (dataSection == 0) {
                    if (vertexAmount == 0) {
                        char delim[] = " ";
                        char *ptr = strtok(line, delim);
                        
                        if (strcmp(ptr, "DIMENSION") == 0) {
                            ptr = strtok(NULL, delim);
                            if(strcmp(ptr, ":")==0){
                                ptr = strtok(NULL, delim);
                            }
                            vertexAmount = atoi(ptr);
                            data->citiesAmount = vertexAmount;
//                            printf("%d",data->citiesAmount);
                            data->x = malloc(vertexAmount * sizeof *data->x);
                            data->y = malloc(vertexAmount * sizeof *data->y);
                        }
                    }
                    if (strcmp(line, "NODE_COORD_SECTION\n") == 0) {
                        dataSection = 1;
                    }
                } else {
                    if (strcmp(line, "EOF\n") != 0) {
                        char delim[] = " ";
                        char *ptr = strtok(line, delim);
                        ptr = strtok(NULL, delim);
                        float x = strtod(ptr, NULL);
                        ptr = strtok(NULL, delim);
                        float y = strtod(ptr, NULL);
//                        printf("\nvertexCounter: %d, cities: %d", vertexCounter, data->citiesAmount);
                        data->x[vertexCounter] = x;
                        data->y[vertexCounter] = y;
                        vertexCounter++;
                    } else {
                        continue;
                    }

                }
            }
            fclose(fp);

        }
    }

}

void printTSPLibData(struct TSPLibData *data) {
    printf("\nVertex amount: %d", data->citiesAmount);
    for (int i = 0; i < data->citiesAmount; i++) {
        printf("\n %f %f", data->x[i], data->y[i]);
    }
}

void printInstanceData(struct TSPInstance *instance) {
    for (int i = 1; i <= instance->citiesAmount; i++) {
        for (int j = 1; j <= instance->citiesAmount; j++) {
            if (j == instance->citiesAmount) {
                printf("\n");
            }
            printf("\nmatrix[%d][%d]=%.2f", i, j, instance->graphMatrix[i][j]);

        }
    }
}

void printRoute(int route[], int routeSize, double length) {
    printf("Route: ");
    for (int i = 0; i < routeSize; i++) {
        printf(" %d ", route[i]);
    }
    printf("\nTotal length: %f", length);
}

int getNearestVertexNotVisited(double distances[], int vertexAmount, int origin, int visitedVertexes[]) {
    double minDistance = BIG_DISTANCE;
    int minDistanceVertex = -1;
    for (int j = 0; j < vertexAmount; j++) {
//        printf("\n %d to %d = %f. visited: %d", origin, j, distances[j], visitedVertexes[j]);
        if (distances[j] < minDistance && visitedVertexes[j] == 0 && origin != j) {
            minDistance = distances[j];
            minDistanceVertex = j;
        }
    }
    return minDistanceVertex;
}

void initializeArray(int array[], int size, int value) {
    for (int i = 0; i < size; i++) {
        array[i] = value;
    }
}

void symetricGreedyTSP(int startingNode, struct TSPInstance *instance) { //using a greedy approach 

    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        double totalDistance = 0;
        int routeOrder[instance->citiesAmount + 1];
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int lastVisited = startingNode;
        for (int visitNumber = 0; visitNumber < instance->citiesAmount; visitNumber++) {
            //            //find the edge with the minimum distance among the not visited vertexes
            int nearestNotVisited = getNearestVertexNotVisited(instance->graphMatrix[visitNumber],
                    instance->citiesAmount, visitNumber, visitedVertexes);

            visitedVertexes[nearestNotVisited] = 1;
            totalDistance += instance->graphMatrix[lastVisited][nearestNotVisited];
            routeOrder[visitNumber + 1] = nearestNotVisited;
//            printf("\n nearest: %d visitnumber: %d", nearestNotVisited, visitNumber);

            lastVisited = nearestNotVisited;
        }
        printRoute(routeOrder, instance->citiesAmount, totalDistance);
    }
}

int main(int argc, char** argv) {

    struct TSPLibData tspLibData;
    struct TSPInstance tspInstance;

    //printf("Initializing\n\n");
    //printf("%d", isValidEuclidian2Dfile("instances/a280.tsp.txt"));
//    parseTSPLibFileEuclidian2D("instances/a280.tsp.txt", &tspLibData, &tspInstance);
    parseTSPLibFileEuclidian2D("instances/d18512.tsp", &tspLibData, &tspInstance);
//    printTSPLibData(&tspLibData);

    allocateTSPInstanceEuclidian2D(&tspLibData, &tspInstance);
    //    printInstanceData(&tspInstance);
    symetricGreedyTSP(1, &tspInstance);
    return (EXIT_SUCCESS);
}

