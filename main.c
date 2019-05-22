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
#include <time.h>
#include <dirent.h> 
#include <stddef.h>
#include "TSPutil.h"
#include "mtwister.h"
#include <limits.h>
/*
 * 
 */

#define CONSTRUCTIVE_APPROACHES_AMOUNT 4

struct TSPLibData {
    int citiesAmount;
    char* instanceName;
    double *x;
    double *y;
};

struct TSPInstance {
    int citiesAmount;
    char* instanceName;
    int** graphMatrix;
};

struct solution {
    int* constructive_route;
    int* local_search_route;
    int constructive_distance;
    int local_search_distance;
    double constructiveTime;
    double localSearchTime;
    double GRASPTime;
    double timeToBestSolution;
    int iterationsToBestSolution;
    double totalTime;
};

struct Config {
    char* constructiveMethod;
    char* alphaType;
    int alphaTypeIndex;
    int constructiveMethodIndex;
    char* localSearchMethod;
    int localSearchMethodIndex;
    int repeatTimes;
    char* mode;
    char* path;
    int alpha;
    int alphaUB;
    int testAlpha;
    char* logName;
    int initialSeed;
    int alphaStep;
    int GRASP_criterion_parameter;
    int GRASP_criterion_type;
};

struct neighbor {
    int distance;
    int target;
    struct neighbor* next;
};

struct pair {
    int x;
    int y;
};

struct Config config;

struct TSPLibData *tspLibData = 0;
struct TSPInstance *tspInstance = 0;
char previousInstance[4096] = "";
int originalAlpha = 0;
int originalUBAlpha = 0;
int originalConstructiveMethodIndex = 0;
int originalLocalSearchMethodIndex = 0;
int randomSeed = 2;
FILE *resultsFile;
MTRand randomizerConstructive;
MTRand randomizerLocalSearch;
int** pairsMap;

int nint(double x) {
    return (int) (x + 0.5);
}

char* getConstructiveMethodName(int methodIndex) { //index 0 = NN
    switch (methodIndex) {
        default: return "Nearest neighbor (NN)";
        case 1: return "Double-sided nearest neighbor (DSNN)";
        case 2: return "Random Nearest Neighbor (RNN)";
        case 3: return "Random Double-sided nearest neighbor (RDSNN)";
    }
}

char* getLocalSearchMethodName(int methodIndex) {
    switch (methodIndex) {
        default: return "None";
        case 1: return "1st improv 2opt";
        case 2: return "Best improv 2opt";
        case 3: return "1st improv modified-2opt";
        case 4: return "Best improv modified-2opt";
    }
}

int getRandomInt(int lb, int ub, int randomizer) {
    double randN = 0;
    if (randomizer == 1) {
        randN = genRand(&randomizerConstructive);
    } else {
        randN = genRand(&randomizerLocalSearch);
    }
    int generatedInt = randN * (ub - lb + 1) + lb;
    return generatedInt;
}

double getRouteDistance(int route[], int routeSize) {
    double distance = 0;
    for (int i = 1; i < routeSize; i++) {
        //        printf("\nroute[%d]=%d -> route[%d]=%d", i - 1, route[i - 1], i, route[i]);
        distance += tspInstance->graphMatrix[route[i - 1]][route[i]];
    }
    return distance;
}

void printNd(struct NeighborDistance *nd, int size) {
    for (int i = 0; i < size; i++) {
        printf("\n to %d: %d",
                nd[i].target,
                nd[i].distanceFromOrigin);
    }
}

//void verifyCanonicalDistance(){
//    double distance = 0;
//    for(int i=1; i<442; i++){
//        printf("\ndistance(%f)+=graphMatrix[%d][%d] (%d)", distance, i-1, i, tspInstance->graphMatrix[i-1][i]);
//        distance+=tspInstance->graphMatrix[i-1][i];
//    }
//    distance+=tspInstance->graphMatrix[441][0];
//    printf("\nCanonical distance: %f",distance);
//}

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
    FILE * fp = NULL;
    char * originalLine = NULL;
    size_t len = 0;
    int read = 0;
    fp = fopen(filePath, "r");
    if (fp) {
        while ((read = getline(&originalLine, &len, fp)) != -1) {
            char * line = originalLine;
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
                } else if (strcmp(ptr, "EDGE_WEIGHT_TYPE:") == 0) {
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

struct TSPInstance* allocateTSPInstanceEuclidian2D(struct TSPLibData *data) {
    struct TSPInstance *instance = malloc(sizeof (struct TSPInstance));
    if (instance) {
        if (data->citiesAmount) {
            instance->citiesAmount = data->citiesAmount;
            instance->graphMatrix = calloc(instance->citiesAmount, sizeof *(instance->graphMatrix));
            instance->instanceName = data->instanceName;
            //            instance->nodes = calloc(instance->citiesAmount, sizeof *(instance->nodes));
            for (int i = 0; i < instance->citiesAmount; i++) {
                instance->graphMatrix[i] = calloc(instance->citiesAmount, sizeof *(instance->graphMatrix));
                //                instance->nodes[i] = createNode(i);
            }
            for (int i = 0; i < instance->citiesAmount; i++) {
                for (int j = 0; j < instance->citiesAmount; j++) {
                    instance->graphMatrix[i][j] = getEuclidianDistance(i, j, data);
                    //                    struct ReachableNode* newNode = malloc(sizeof *(newNode));
                    //                    newNode->distance = getEuclidianDistance(i, j, data);
                    //                    newNode->id = j;
                    //                    newNode->next = NULL;
                    //                    addVertex(instance->nodes[i], newNode);
                }
            }
        }
    }
    return instance;
}

struct TSPLibData* parseTSPLibFileEuclidian2D(char *filePath) { //only valid for Euclidian2D instances
    struct TSPLibData *data = 0;
    if (isValidEuclidian2Dfile(filePath)) {
        FILE * fp = NULL;
        char * originalLine = NULL;
        size_t len = 0;
        int read = 0;
        int dataSection = 0;
        int vertexAmount = 0;
        int vertexCounter = 0;
        fp = fopen(filePath, "r");
        if (fp) {
            while ((read = getline(&originalLine, &len, fp)) != -1) {
                char *line = originalLine;
                if (dataSection == 0) {
                    if (vertexAmount == 0) {
                        char delim[] = " ";
                        char *ptr = strtok(line, delim);
                        if (strcmp(ptr, "DIMENSION") == 0) {
                            ptr = strtok(NULL, delim);
                            if (strcmp(ptr, ":") == 0) {
                                ptr = strtok(NULL, delim);
                            }
                            vertexAmount = atoi(ptr);
                            data = malloc(sizeof *data);
                            if (data) {
                                data->x = calloc(vertexAmount, sizeof *(data->x));
                                data->y = calloc(vertexAmount, sizeof *(data->y));
                            }
                            data->citiesAmount = vertexAmount;
                        } else if (strcmp(ptr, "DIMENSION:") == 0) {
                            ptr = strtok(NULL, delim);
                            vertexAmount = atoi(ptr);
                            data = malloc(sizeof (*data));

                            if (data) {
                                data->x = calloc(vertexAmount, sizeof *(data->x));
                                data->y = calloc(vertexAmount, sizeof *(data->y));
                            }
                            data->citiesAmount = vertexAmount;
                        }
                    }
                    if (strcmp(line, "NODE_COORD_SECTION\n") == 0) {
                        dataSection = 1;
                    }
                } else {
                    if (vertexCounter < vertexAmount) {
                        char delim[] = " ";
                        char *ptr = strtok(line, delim);
                        ptr = strtok(NULL, delim);
                        float x = strtod(ptr, NULL);
                        ptr = strtok(NULL, delim);
                        float y = strtod(ptr, NULL);
                        data->x[vertexCounter] = x;
                        data->y[vertexCounter] = y;
                        data->instanceName = filePath;
                        vertexCounter++;
                    } else {
                        continue;
                    }

                }
            }
            fclose(fp);

            free(originalLine);


        }
    } else {
        printf("\nThe file you supplied is invalid or cannot be read. File: %s \n", filePath);
    }
    return data;
}

void printTSPLibData(struct TSPLibData *data) {
    printf("\nVertex amount: %d", data->citiesAmount);
    for (int i = 0; i < data->citiesAmount; i++) {
        printf("\nvertex %d: %f %f", i, data->x[i], data->y[i]);
    }
}

void printInstanceData(struct TSPInstance *instance) {
    for (int i = 1; i < instance->citiesAmount; i++) {
        for (int j = 1; j < instance->citiesAmount; j++) {
            if (j == instance->citiesAmount) {
                printf("\n");
            }
            printf("\nmatrix[%d][%d]=%d", i, j, instance->graphMatrix[i][j]);

        }
    }
    //    for (int i = 0; i < instance->citiesAmount; i++) {
    //        printInstance(instance->nodes[i]);
    //    }
}

void printRoute(int route[], int routeSize, double length) {
    printf("\nRoute: ");
    for (int i = 0; i < routeSize; i++) {
        printf(" %d ", route[i]);
    }
    printf("\nTotal length (provided): %f, total length (calculated): %f, total cities: %d", length, getRouteDistance(route, routeSize), routeSize - 1);
}

int getNearestVertexNotVisited(int distances[], int vertexAmount, int origin, int visitedVertexes[]) {
    double minDistance = INT_MAX;
    int minDistanceVertex = -1;
    for (int j = 0; j < vertexAmount; j++) {
        //                printf("\n %d to %d = %f. visited: %d", origin, j, distances[j], visitedVertexes[j]);
        if (distances[j] < minDistance && visitedVertexes[j] == 0 && origin != j) {
            minDistance = distances[j];
            minDistanceVertex = j;
        }
    }
    //        printf("\n min distance = %f  from %d to %d", minDistance, origin, minDistanceVertex);
    return minDistanceVertex;
}

void initializeArray(int array[], int size, int value) {
    for (int i = 0; i < size; i++) {
        array[i] = value;
    }
}

int* twoOptSwap(int* route, int routeSize, int i, int j) {
    //    printf("\n Swapping %d and %d, index %d, %d", route[i], route[j], i, j);
    if (i > j) {
        int aux = i;
        i = j;
        j = aux;
    }
    int* newRoute = malloc(routeSize * sizeof (int));
    memcpy(newRoute, route, routeSize * sizeof (int));
    if (j < routeSize && i >= 0) {
        for (int x = j, y = i; x >= i; x--, y++) {
            newRoute[y] = route[x];
            //            printf("\nnewRoute[%d] = %d", y, route[x]);
        }
    }
    return newRoute;
}

int calculateNewDistance(int oldDistance, int i, int j, int* oldRoute) {
    int pair1_old_x = oldRoute[i - 1];
    int pair1_old_y = oldRoute[i];
    int pair2_old_x = oldRoute[j];
    int pair2_old_y = oldRoute[j + 1];

    int pair1_new_x = oldRoute[i - 1];
    int pair1_new_y = oldRoute[j];
    int pair2_new_x = oldRoute[i];
    int pair2_new_y = oldRoute[j + 1];

    int subtract1 = tspInstance->graphMatrix[pair1_old_x][pair1_old_y];
    int subtract2 = tspInstance->graphMatrix[pair2_old_x][pair2_old_y];
    int add1 = tspInstance->graphMatrix[pair1_new_x][pair1_new_y];
    int add2 = tspInstance->graphMatrix[pair2_new_x][pair2_new_y];
    //    printf("\n neighborDistance = oldDistance - d(%d,%d) - d(%d,%d) + d(%d,%d) + d(%d,%d)", pair1_old_x, pair1_old_y, pair2_old_x, pair2_old_y, pair1_new_x, pair1_new_y,
    //            pair2_new_x, pair2_new_y);
    int newDistance = oldDistance - subtract1 - subtract2 + add1 + add2;
    //        printf("\n %d = %d - %d - %d + %d + %d", newDistance, oldDistance, subtract1, subtract2, add1, add2);
    return newDistance;
}

void printMatrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        printf("\n");
        for (int j = 0; j < size; j++) {
            printf("%d\t", matrix[i][j]);
        }
    }
}

struct pair getUniqueNewPair(int pairsAmount, int size) {
    if (pairsAmount < ((size - 3)*(size - 2)) / 2) {
        int randomX = -1;
        int randomY = -1;

        do {
            randomX = getRandomInt(1, size - 3, 0);
            randomY = getRandomInt(randomX + 1, size - 2, 0);
        } while (pairsMap[randomX][randomY] != 0);
        struct pair newPair;
        newPair.x = randomX;
        newPair.y = randomY;
        pairsMap[newPair.x][newPair.y] = 1;
        return newPair;

    } else {
        struct pair newPair = {-1, -1};
        return newPair;
    }
}

void allocatePairsMap(int size) {
    pairsMap = calloc(size, sizeof *(pairsMap));
    for (int i = 0; i < size; i++) {
        pairsMap[i] = calloc(size, sizeof *(pairsMap));
        //        for (int j = 0; j < size; j++) {
        //            pairsMap[i][j] = 0;
        //        }
    }
}

void freePairsMap(int** pairs, int size) {
    if (pairs != NULL) {
        for (int i = 0; i < size; i++) {
            free(pairs[i]);
        }
        free(pairs);
    }
}

void initializePairsMap() {
    if (config.localSearchMethodIndex == 3 || config.localSearchMethodIndex == 4) {
        allocatePairsMap(tspInstance->citiesAmount + 1);
    }
}

void resetPairsMap(int size) {
    //    printf("\n\nResetting pairs map");
    if (pairsMap != NULL && pairsMap != 0) {
        //        printf("\n\nResetting pairs map2");
        //        printf("\n\nResetting pairs map2");
        for (int i = 0; i < size; i++) {
            //            printf("\n\n i: %d", i);
            //            printf("\n\n i: %d", i);
            for (int j = 0; j < size; j++) {
                pairsMap[i][j] = 0;
            }
        }
        //        printf("\nleaving reset");
        //        printf("\nleaving reset");
    }
}

int searchFirstImprovementNeighbor(int solutionSize, struct solution* currentSolution, int* dontLook) {
    for (int i = 1; i < solutionSize - 1; i++) {
        if (dontLook[currentSolution->local_search_route[i]] != 1) {
            for (int j = i + 1; j < solutionSize - 1; j++) {
                int neighborDistance = calculateNewDistance(currentSolution->local_search_distance, i, j, currentSolution->local_search_route);
                //            printf("\n Neighbor: ");
                //            printRoute(neighbor, solutionSize, neighborDistance);
                if (neighborDistance < currentSolution->local_search_distance) {
                    int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, i, j);
                    currentSolution->local_search_distance = neighborDistance;
                    memcpy(currentSolution->local_search_route, neighbor, solutionSize * sizeof (int));
                    free(neighbor);
                    dontLook[currentSolution->local_search_route[i]] = 0;
                    dontLook[currentSolution->local_search_route[i - 1]] = 0;
                    dontLook[currentSolution->local_search_route[j]] = 0;
                    dontLook[currentSolution->local_search_route[i + 1]] = 0;
                    return 1;
                }
            }
            dontLook[currentSolution->local_search_route[i]] = 1;
        }
    }
    return 0;
}

int searchFirstImprovementNeighbor_2(int solutionSize, struct solution* currentSolution, int alphaLocalSearch) {
    int iterations = ((double) alphaLocalSearch / (double) 100) * (((solutionSize - 3)*(solutionSize - 2)) / 2);
    //    printf("\n Local search with %d iterations: ", iterations);
    //    int** pairsMap = getPairsMap(solutionSize);
    resetPairsMap(solutionSize);
    int pairsAmount = 0;
    for (int x = 0; x < iterations; x++) {
        struct pair newPair = getUniqueNewPair(pairsAmount++, solutionSize);
        //        printf("\n%d",currentSolution->route[solutionSize-1]);
        //        int i = getRandomInt(1, solutionSize - 3, 0);
        //        int j = getRandomInt(i + 1, solutionSize - 2, 0);
        int randomI = newPair.x;
        int randomJ = newPair.y;
        //        printf("\n random pair: %d %d", randomI, randomJ);
        //        printf("\n random pair: %d %d", randomI, randomJ);
        int neighborDistance = calculateNewDistance(currentSolution->local_search_distance, randomI, randomJ, currentSolution->local_search_route);
        if (neighborDistance < currentSolution->local_search_distance) {
            int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, randomI, randomJ);
            currentSolution->local_search_distance = neighborDistance;
            memcpy(currentSolution->local_search_route, neighbor, solutionSize * sizeof (int));
            free(neighbor);
            //            freePairsMap(pairsMap, solutionSize);
            return 1;
        }
    }
    //    freePairsMap(pairsMap, solutionSize);
    return 0;
}

int searchBestImprovementNeighbor_2(int solutionSize, struct solution* currentSolution, int alphaLocalSearch) {
    int improvementFound = 0;
    //    struct solution* bestKnownSolution = malloc(sizeof *(bestKnownSolution));
    //    bestKnownSolution->local_search_route = calloc(solutionSize, sizeof (int));
    //    memcpy(bestKnownSolution->local_search_route, currentSolution->local_search_route, solutionSize * sizeof (int));
    //    bestKnownSolution->local_search_distance = currentSolution->local_search_distance;
    int iterations = ((double) alphaLocalSearch / (double) 100) * (((solutionSize - 3)*(solutionSize - 2)) / 2);
    //    int iterations = ((double) alphaLocalSearch / (double) 100) * solutionSize;
    //    printf("\n iterations: %d", iterations);
    //        printf("\nStarting best improvement neighbor search. Current solution:");
    //        printRoute(bestKnownSolution->route, solutionSize, bestKnownSolution->distance);
    resetPairsMap(solutionSize);

    int pairsAmount = 0;
    //    printf("\n\n");
    int bestI = -1, bestJ = -1, bestDistance = currentSolution->local_search_distance;
    for (int x = 0; x < iterations; x++) {
        struct pair newPair = getUniqueNewPair(pairsAmount++, solutionSize);
        //        printf("\n%d",currentSolution->route[solutionSize-1]);
        //        int i = getRandomInt(1, solutionSize - 3, 0);
        //        int j = getRandomInt(i + 1, solutionSize - 2, 0);
        int i = newPair.x;
        int j = newPair.y;
        //        printf("\n%d,%d", i, j);
        int neighborDistance = calculateNewDistance(currentSolution->local_search_distance, i, j, currentSolution->local_search_route);
        //                    printf("\n Neighbor: ");
        //                    printRoute(neighbor, solutionSize, neighborDistance);
        if (neighborDistance < bestDistance) {
            //                        int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, i, j);
            //                        bestKnownSolution->local_search_distance = neighborDistance;
            //                        memcpy(bestKnownSolution->local_search_route, neighbor, solutionSize * sizeof (int));
            //                        free(neighbor);
            bestI = i;
            bestJ = j;
            bestDistance = neighborDistance;
            //                bestKnownSolution->route = neighbor;
        }

        //            getchar();
    }
    //    if (bestI != -1 && bestJ != -1) {
    //        int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, bestI, bestJ);
    //        bestKnownSolution->local_search_distance = bestDistance;
    //        memcpy(bestKnownSolution->local_search_route, neighbor, solutionSize * sizeof (int));
    //        free(neighbor);
    //    }
    //    freePairsMap(pairsMap, solutionSize);
    //    printf("\n Best neighbor:");
    //    printRoute(bestKnownSolution->route, solutionSize, bestKnownSolution->distance);
    if (bestDistance < currentSolution->local_search_distance) {
        currentSolution->local_search_distance = bestDistance;
        int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, bestI, bestJ);
        memcpy(currentSolution->local_search_route, neighbor, solutionSize * sizeof (int));
        free(neighbor);
        improvementFound = 1;
    }
    //    free(bestKnownSolution->local_search_route);
    //    free(bestKnownSolution);
    return improvementFound;
}

int searchBestImprovementNeighbor(int solutionSize, struct solution* currentSolution) {
    int improvementFound = 0;
    struct solution* bestKnownSolution = malloc(sizeof *(bestKnownSolution));
    bestKnownSolution->local_search_route = calloc(solutionSize, sizeof (int));
    memcpy(bestKnownSolution->local_search_route, currentSolution->local_search_route, solutionSize * sizeof (int));
    bestKnownSolution->local_search_distance = currentSolution->local_search_distance;
    //        printf("\nStarting best improvement neighbor search. Current solution:");
    //        printRoute(currentSolution->local_search_route, solutionSize, bestKnownSolution->local_search_distance);
    int bestI = -1, bestJ = -1, bestDistance = currentSolution->local_search_distance;
    for (int i = 1; i < solutionSize - 1; i++) {

        for (int j = i + 1; j < solutionSize - 1; j++) {
            //            printf("\n%d,%d", i, j);
            int neighborDistance = calculateNewDistance(currentSolution->local_search_distance, i, j, currentSolution->local_search_route);
            //            printf("\n Neighbor distance: %d . Best neighbor: %d  i=%d, j=%d", neighborDistance, bestKnownSolution->local_search_distance,i,j);
            if (neighborDistance < bestDistance) {
                bestI = i;
                bestJ = j;
                bestDistance = neighborDistance;
            }
        }
    }
    //    printf("\n Best neighbor:");
    if (bestDistance < currentSolution->local_search_distance) {
        currentSolution->local_search_distance = bestDistance;
        int* neighbor = twoOptSwap(currentSolution->local_search_route, solutionSize, bestI, bestJ);
        memcpy(currentSolution->local_search_route, neighbor, solutionSize * sizeof (int));
        free(neighbor);
        improvementFound = 1;
    }
    return improvementFound;
}

//void testRoute(){
//    int route[52] = {1 , 22 , 8 , 26 , 31 , 28 , 3 , 36 , 35 , 20 , 2 , 29 , 21 , 16 , 50 , 34 , 30 
//            , 9 , 49 , 10 , 39 , 33 , 45 , 15 , 44 , 42 , 40 , 19 , 41 , 13 , 25 , 14 , 24 , 43 , 7 , 
//    23 , 48 , 6 , 27 , 51 , 46 , 12 , 47 , 18 , 4 , 17 , 37 , 5 , 38 , 11 , 32, 1};
//    int routeSize = 52;
//    
//    for(int i=0; i<routeSize; i++){
//        route[i]=route[i]-1;
////        printf("\n %d", route[i]);
//        
//    }
//    printf("\n Test route eil51");
//    printRoute(route, routeSize, 426);
//}

struct solution* localSearch(struct solution* currentSolution, int solutionSize) {
    if (currentSolution != NULL) {
        currentSolution->local_search_distance = currentSolution->constructive_distance;
        memcpy(currentSolution->local_search_route, currentSolution->constructive_route, solutionSize * sizeof (int));
        int keepSearching = 1;
        int* dontLook = calloc(solutionSize, sizeof *(dontLook));
        for (int i = 0; i < solutionSize; i++) {
            dontLook[i] = 0;
        }
        do {
            //            printf("\n Current solution distance: %d ", currentSolution->local_search_distance);
            //                                                        printRoute(currentSolution->local_search_route, solutionSize, currentSolution->local_search_distance);
            int newSolution = 0;
            switch (config.localSearchMethodIndex) {
                case 1:
                    newSolution = searchFirstImprovementNeighbor(solutionSize, currentSolution, dontLook);
                    break;
                case 2:
                    newSolution = searchBestImprovementNeighbor(solutionSize, currentSolution);
                    break;
                case 3:
                    newSolution = searchFirstImprovementNeighbor_2(solutionSize, currentSolution, config.alpha);
                    break;
                case 4:
                    newSolution = searchBestImprovementNeighbor_2(solutionSize, currentSolution, config.alpha);
                    break;
            }
            if (newSolution == 0) {
                //                printf("\nLocal optimum found. Stopping.");
                keepSearching = 0;
            } else {
                //                printf("\n Improvement found. Moving to neighbor of distance %d", currentSolution->local_search_distance);
                //                                                                            printf("\n Current solution: ");
                //                                                                            printRoute(currentSolution->local_search_route, solutionSize, currentSolution->local_search_distance);
            }
            //                        printf("\n\n\n");
            //                    getchar();
        } while (keepSearching == 1);
        //    printRoute(currentSolution->route, constructiveSolutionSize, currentSolution->distance);
        //    testRoute();    
        free(dontLook);
    }

    return currentSolution;
}

int* getNNearestVertexNotVisited(int origin, int visitedVertexes[], int N) {
    struct NeighborDistance *nd = calloc(tspInstance->citiesAmount, sizeof (*nd));
    int notVisitedNds = 0;
    //    struct neighbor* notVisitedNeighbors = NULL;
    //    struct neighbor* newNeighbor = NULL;
    for (int i = 0; i < tspInstance->citiesAmount; i++) {
        //        printf("\norigin: %d  i: %d", origin, i);
        if (visitedVertexes[i] == 0 && i != origin) {
            nd[notVisitedNds].distanceFromOrigin = tspInstance->graphMatrix[origin][i];
            nd[notVisitedNds].target = i;
            notVisitedNds++;
        }
    }
    sortNeighborsByDistance(nd, notVisitedNds);
    //    printNd(nd, notVisitedNds);
    //    printf("\n\n");
    //    printList(notVisitedNeighbors);
    int* NNearestNotVisited = (int*) calloc(N, sizeof (int));
    initializeArray(NNearestNotVisited, N, -1);
    //    newNeighbor = notVisitedNeighbors;
    //    int notVisitedAmount = 0;
    //    for (int i = 0; i < neighborsAmount && i < N; i++, notVisitedAmount++) {
    //        NNearestNotVisited[i] = newNeighbor->target;
    //        newNeighbor = newNeighbor->next;
    //    }
    //    for (int i = 0; i < notVisitedNds; i++) {
    //        printf("\ndistance to %d=%d", nd[i].target, nd[i].distanceFromOrigin);
    //    }

    int notVisitedIndex = 0;
    for (int j = 0; j < notVisitedNds; j++) {
        int current = nd[j].target;
        if (notVisitedIndex < N) {
            NNearestNotVisited[notVisitedIndex++] = current;
        }
        if (notVisitedIndex >= N) {
            break;
        }
    }
    //    printf("\nN nearest not yet visited: ");
    //    for (int i = 0; i < notVisitedAmount; i++) {
    //        printf("\n %d", NNearestNotVisited[i]);
    //    }

    if (nd) {
        free(nd);
    }

    return NNearestNotVisited;
}

//int* getNNearestVertexNotVisited(int origin, int visitedVertexes[], int N) {
//    int* NNearestNotVisited = (int*) calloc(N, sizeof (int));
//    initializeArray(NNearestNotVisited, N, -1);
//    //    printInstance(tspInstance->nodes[origin]);
//    struct ReachableNode* current = tspInstance->nodes[origin]->head;
//    int NNearestNotVisitedIndex = 0;
//    while (current != NULL && NNearestNotVisitedIndex < N) {
//        if (visitedVertexes[current->id] == 0) {
//            NNearestNotVisited[NNearestNotVisitedIndex++] = current->id;
//        }
//        current = current->next;
//    }
//    return NNearestNotVisited;
//}

double finishCycle(int startingNode, double totalDistance, int* route, int routeSize) {
    //    printf("\nFinishing cycle by connecting %d and %d with distance of %d", route[routeSize - 2], startingNode, tspInstance->graphMatrix[route[routeSize - 2]][startingNode]);
    route[routeSize - 1] = startingNode;
    totalDistance += tspInstance->graphMatrix[route[routeSize - 2]][startingNode];
    return totalDistance;
}

struct solution* symmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;
    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);

        int routeOrder[instance->citiesAmount + 1];
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int lastVisited = startingNode;
        int visitNumber = 1;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            //            //find the edge with the minimum distance among the not visited vertexes
            int nearestNotVisited = getNearestVertexNotVisited(instance->graphMatrix[lastVisited],
                    instance->citiesAmount, lastVisited, visitedVertexes);
            if (nearestNotVisited != -1) {
                visitedVertexes[nearestNotVisited] = 1;
                totalDistance += instance->graphMatrix[lastVisited][nearestNotVisited];
                routeOrder[visitNumber] = nearestNotVisited;
                //                            printf("\n nearest: %d visitnumber: %d", nearestNotVisited, visitNumber);

                lastVisited = nearestNotVisited;
            }
        }
        totalDistance = finishCycle(startingNode, totalDistance, routeOrder, instance->citiesAmount + 1);
        struct solution* solution = malloc(sizeof *(solution));
        solution->constructive_route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->constructive_distance = totalDistance;
        memcpy(solution->constructive_route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        //        testRoute();
        return solution;
    }
    return NULL;
}

int getCandidatesSetSize(int neighborsAmount) {
    if (config.alpha == 0) {
        return 1;
    }
    double sizeD = neighborsAmount * ((double) config.alpha / 100);
    return ceil(sizeD);
}

//quantity-based RCL

int getRandomNearestNotVisited(int origin,
        int visitedVertexes[], int candidateSetSize) {
    clock_t start = 0, end = 0;
    double functionTime = 0;
    start = clock();
    int randomNearestNotVisited = -1;
    if (candidateSetSize != 0) {
        int* NNearestNotVisited = getNNearestVertexNotVisited(origin, visitedVertexes, candidateSetSize);
        //                        printf("\n origin: %d", origin);
        //                        printf("\nN nearest not yet visited: ");
        //                        for (int i = 0; i < candidateSetSize; i++) {
        //                            printf("\n %d", NNearestNotVisited[i]);
        //                        }
        int randomIndex = -1;
        do {
            randomIndex = getRandomInt(0, candidateSetSize - 1, 1);
            //            if(randomIndex==-1){
            //                printf("\n Searching another one");
            //            }
        } while (NNearestNotVisited[randomIndex] == -1);
        //                        printf("\n candidateSize: %d random choosen: %d\n", candidateSetSize, randomIndex);
        randomNearestNotVisited = NNearestNotVisited[randomIndex];
        if (NNearestNotVisited != NULL) {
            free(NNearestNotVisited);
        }
    }
    end = clock();
    functionTime = ((double) (end - start)) / CLOCKS_PER_SEC;
    //    printf("\ngetRandomNearestNotVisited: %f seconds", functionTime);
    return randomNearestNotVisited;
}

int getMinDistanceIndexNotVisited(int* visitedVertexes, int vertexesAmount, int origin) {
    int minDistance = INT_MAX;
    int minDistanceIndex = 0;
    for (int i = 0; i < vertexesAmount; i++) {
        if (visitedVertexes[i] == 0 &&
                tspInstance->graphMatrix[origin][i] < minDistance) {
            minDistance = tspInstance->graphMatrix[origin][i];
            minDistanceIndex = i;
        }
    }
    return minDistanceIndex;
}

int getMaxDistanceIndexNotVisited(int* visitedVertexes, int vertexesAmount, int origin) {
    int maxDistance = INT_MIN;
    int maxDistanceIndex = 0;
    for (int i = 0; i < vertexesAmount; i++) {
        if (visitedVertexes[i] == 0 &&
                tspInstance->graphMatrix[origin][i] > maxDistance) {
            maxDistance = tspInstance->graphMatrix[origin][i];
            maxDistanceIndex = i;
        }
    }
    return maxDistanceIndex;
}

//quality-based RCL

int getRandomNearestNotVisited_2(int origin,
        int visitedVertexes[]) {
    int minDistanceIndex = getMinDistanceIndexNotVisited(visitedVertexes, tspInstance->citiesAmount, origin);
    int maxDistanceIndex = getMaxDistanceIndexNotVisited(visitedVertexes, tspInstance->citiesAmount, origin);
    int minCandidateDistance = tspInstance->graphMatrix[origin][minDistanceIndex];
    int maxCandidateDistance = tspInstance->graphMatrix[origin][maxDistanceIndex];
    int alphaOperand = nint(((double) config.alpha / 100) * (maxCandidateDistance - minCandidateDistance));
    int threshold = minCandidateDistance + alphaOperand;
    int randomIndex = -1;
    do {
        randomIndex = getRandomInt(0, tspInstance->citiesAmount - 1, 1);
        //        printf("\nRandom index: %d. Visited: %d  Distance: %d   Threshold: %d  Origin: %d",
        //                randomIndex, visitedVertexes[randomIndex], tspInstance->graphMatrix[origin][randomIndex], 
        //                threshold, origin);
    } while (visitedVertexes[randomIndex] == 1
            || tspInstance->graphMatrix[origin][randomIndex] > threshold
            || randomIndex == origin);
    //    printf("\nChosen RandomIndex = %d", randomIndex);
    return randomIndex;
}

struct solution* randomSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;
    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        int routeOrder[instance->citiesAmount + 1];
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int lastVisited = startingNode;
        int candidateSetSize = getCandidatesSetSize(instance->citiesAmount - 1);
        printf("\n candidateSetSize: %d\n", candidateSetSize);
        int visitNumber = 1;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            int randomNearestNotVisited = -1;
            if (config.alphaTypeIndex == 0) {
                randomNearestNotVisited = getRandomNearestNotVisited(lastVisited, visitedVertexes, candidateSetSize);
            } else {
                randomNearestNotVisited = getRandomNearestNotVisited_2(lastVisited, visitedVertexes);
            }
            //            printf("\n rnn n%d", randomNearestNotVisited);
            if (randomNearestNotVisited != -1) {
                visitedVertexes[randomNearestNotVisited] = 1;
                totalDistance += instance->graphMatrix[lastVisited][randomNearestNotVisited];
                routeOrder[visitNumber] = randomNearestNotVisited;
                //                printf("\n random nearest: %d  totalDistance: %f visitnumber: %d", randomNearestNotVisited, totalDistance, visitNumber);
                lastVisited = randomNearestNotVisited;
            }
        }
        totalDistance = finishCycle(startingNode, totalDistance, routeOrder, instance->citiesAmount + 1);
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        struct solution* solution = malloc(sizeof *(solution));
        solution->constructive_route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->constructive_distance = totalDistance;
        memcpy(solution->constructive_route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
        return solution;
    }
    return NULL;
}

struct solution* randomDoubleSidedSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;

    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        int routeOrder[instance->citiesAmount + 1];

        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int candidateSetSize = getCandidatesSetSize(instance->citiesAmount - 1);
        int visitNumber = 1;
        int head1VisitNumber = 1;
        int head2VisitNumber = instance->citiesAmount - 1;
        int lastVisitedNode = startingNode;
        int lastVisitedDistance = 0;
        int head1 = lastVisitedNode;
        int head2 = lastVisitedNode;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            //            printf("\nHead1: %d   || Head2: %d", head1, head2);
            int randomNearestNotVisitedHead1 = -1;
            int randomNearestNotVisitedHead2 = -1;
            if (config.alphaTypeIndex == 0) {
                randomNearestNotVisitedHead1 = getRandomNearestNotVisited(head1, visitedVertexes, candidateSetSize);
                randomNearestNotVisitedHead2 = getRandomNearestNotVisited(head2, visitedVertexes, candidateSetSize);
            } else {
                randomNearestNotVisitedHead1 = getRandomNearestNotVisited_2(head1, visitedVertexes);
                randomNearestNotVisitedHead2 = getRandomNearestNotVisited_2(head2, visitedVertexes);
            }
            if (randomNearestNotVisitedHead1 != -1 && randomNearestNotVisitedHead2 != -1) {
                double nearestNotVisitedHead1Distance = instance->graphMatrix[head1][randomNearestNotVisitedHead1];
                double nearestNotVisitedHead2Distance = instance->graphMatrix[head2][randomNearestNotVisitedHead2];
                //                printf("\n nearestHead1 (%d to %d): distance: %f || nearestHead2 (%d to %d): distance: %f",
                //                        head1, randomNearestNotVisitedHead1, nearestNotVisitedHead1Distance, head2,
                //                        randomNearestNotVisitedHead2, nearestNotVisitedHead2Distance);
                if (nearestNotVisitedHead1Distance <= nearestNotVisitedHead2Distance) {
                    head1 = randomNearestNotVisitedHead1;
                    lastVisitedNode = head1;
                    lastVisitedDistance = nearestNotVisitedHead1Distance;
                    //                    printf("\n Andei com Head1. Move para %d adiciona na posicao %d da rota", head1, head1VisitNumber);
                    routeOrder[head1VisitNumber++] = lastVisitedNode;
                } else {
                    head2 = randomNearestNotVisitedHead2;
                    lastVisitedNode = head2;
                    lastVisitedDistance = nearestNotVisitedHead2Distance;
                    //                    printf("\n Andei com Head2. Move para %d adiciona na posicao %d da rota", head2, head2VisitNumber);
                    routeOrder[head2VisitNumber--] = lastVisitedNode;
                }
                //                printf("\n head1VisitNumber = %d  head2VisitNumber = %d", head1VisitNumber, head2VisitNumber);
                visitedVertexes[lastVisitedNode] = 1;
                //                printf("\nDistance = %f + %d = %f", totalDistance, lastVisitedDistance, totalDistance + lastVisitedDistance);
                totalDistance += lastVisitedDistance;
                if (head1VisitNumber > head2VisitNumber) {
                    totalDistance += instance->graphMatrix[head1][head2];
                    //                    printf("\n Connecting head1(%d) with head2(%d). Distance = %d", head1, head2, instance->graphMatrix[head1][head2]);
                    routeOrder[instance->citiesAmount] = startingNode;
                }
            }
        }
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        struct solution* solution = malloc(sizeof *(solution));
        solution->constructive_route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->constructive_distance = totalDistance;
        memcpy(solution->constructive_route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        //        testRoute();
        return solution;
    }
    return NULL;
}

struct solution* doubleSidedSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;

    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        int routeOrder[instance->citiesAmount + 1];

        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int visitNumber = 1;
        int head1VisitNumber = 1;
        int head2VisitNumber = instance->citiesAmount - 1;
        int lastVisitedNode = startingNode;
        int lastVisitedDistance = 0;
        int head1 = lastVisitedNode;
        int head2 = lastVisitedNode;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            //            printf("\nHead1: %d   || Head2: %d", head1, head2);
            int nearestNotVisitedHead1 = getNearestVertexNotVisited(instance->graphMatrix[head1],
                    instance->citiesAmount, head1, visitedVertexes);
            int nearestNotVisitedHead2 = getNearestVertexNotVisited(instance->graphMatrix[head2],
                    instance->citiesAmount, head2, visitedVertexes);
            if (nearestNotVisitedHead1 != -1 && nearestNotVisitedHead2 != -1) {
                double nearestNotVisitedHead1Distance = instance->graphMatrix[head1][nearestNotVisitedHead1];
                double nearestNotVisitedHead2Distance = instance->graphMatrix[head2][nearestNotVisitedHead2];
                //                printf("\n nearestHead1 (%d to %d): distance: %f || nearestHead2 (%d to %d): distance: %f",
                //                        head1, randomNearestNotVisitedHead1, nearestNotVisitedHead1Distance, head2,
                //                        randomNearestNotVisitedHead2, nearestNotVisitedHead2Distance);
                if (nearestNotVisitedHead1Distance <= nearestNotVisitedHead2Distance) {
                    head1 = nearestNotVisitedHead1;
                    lastVisitedNode = head1;
                    lastVisitedDistance = nearestNotVisitedHead1Distance;
                    //                    printf("\n Andei com Head1. Move para %d adiciona na posicao %d da rota", head1, head1VisitNumber);
                    routeOrder[head1VisitNumber++] = lastVisitedNode;
                } else {
                    head2 = nearestNotVisitedHead2;
                    lastVisitedNode = head2;
                    lastVisitedDistance = nearestNotVisitedHead2Distance;
                    //                    printf("\n Andei com Head2. Move para %d adiciona na posicao %d da rota", head2, head2VisitNumber);
                    routeOrder[head2VisitNumber--] = lastVisitedNode;
                }
                //                printf("\n head1VisitNumber = %d  head2VisitNumber = %d", head1VisitNumber, head2VisitNumber);
                visitedVertexes[lastVisitedNode] = 1;
                //                printf("\nDistance = %f + %d = %f", totalDistance, lastVisitedDistance, totalDistance + lastVisitedDistance);
                totalDistance += lastVisitedDistance;
                if (head1VisitNumber > head2VisitNumber) {
                    totalDistance += instance->graphMatrix[head1][head2];
                    //                    printf("\n Connecting head1(%d) with head2(%d). Distance = %d", head1, head2, instance->graphMatrix[head1][head2]);
                    routeOrder[instance->citiesAmount] = startingNode;
                }
            }
        }
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        struct solution* solution = malloc(sizeof *(solution));
        solution->constructive_route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->constructive_distance = totalDistance;
        memcpy(solution->constructive_route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        //        testRoute();
        return solution;
    }

    return NULL;
}

struct solution* constructive_controller() {
    clock_t start = 0, end = 0;
    struct solution* constructiveSolution = NULL;
    start = clock();
    switch (config.constructiveMethodIndex) {
        default:
            constructiveSolution = symmetricGreedyTSP(0, tspInstance);
            break;
        case 1:
            constructiveSolution = doubleSidedSymmetricGreedyTSP(0, tspInstance);
            break;
        case 2:
            constructiveSolution = randomSymmetricGreedyTSP(0, tspInstance);
            break;
        case 3:
            constructiveSolution = randomDoubleSidedSymmetricGreedyTSP(0, tspInstance);
            break;
    }
    end = clock();
    if (constructiveSolution != NULL) {
        constructiveSolution->constructiveTime = ((double) (end - start)) / CLOCKS_PER_SEC;
    }
    printf("\nGot constructive Solution using %s in %.6f seconds. Distance: %d",
            getConstructiveMethodName(config.constructiveMethodIndex), constructiveSolution->constructiveTime,
            constructiveSolution->constructive_distance);
    //    printRoute(constructiveSolution->constructive_route, tspInstance->citiesAmount + 1, constructiveSolution->constructive_distance);
    return constructiveSolution;
}

struct solution* local_search_controller(struct solution* currentSolution) {
    clock_t start = 0, end = 0;
    start = clock();
    if (currentSolution != NULL) {
        currentSolution->local_search_route = calloc(tspInstance->citiesAmount + 1, sizeof *(currentSolution->local_search_route));
        if (config.localSearchMethodIndex != 0) {
            currentSolution = localSearch(currentSolution, tspInstance->citiesAmount + 1);
            end = clock();
            currentSolution->localSearchTime = ((double) (end - start)) / CLOCKS_PER_SEC;
        } else {
            currentSolution->local_search_distance = 0;
            currentSolution->localSearchTime = 0;
            currentSolution->local_search_route = malloc(0);
        }
    }
    printf("\n Local search Solution using %s in %.6f seconds. Distance:  %d",
            getLocalSearchMethodName(config.localSearchMethodIndex), currentSolution->localSearchTime,
            currentSolution->local_search_distance);
    //    printRoute(currentSolution->constructive_route, tspInstance->citiesAmount + 1, currentSolution->constructive_distance);
    return currentSolution;
}

struct solution* GRASP() {
    struct solution* solution = constructive_controller();
    solution = local_search_controller(solution);
    return solution;
}

void freeSolution(struct solution* solution) {
    if (solution != NULL) {
        printf("\nfreeing local search route...");
        if (solution->local_search_route != NULL) {
            free(solution->local_search_route);
            solution->local_search_route = NULL;
        }
        printf("\nfreeing constructive search route...");
        if (solution->constructive_route != NULL) {
            free(solution->constructive_route);
            solution->constructive_route = NULL;
        }
        printf("\nfreeing solution... %d", solution->constructive_distance);
        printf("\nfreeing solution... %d\n", solution->constructive_distance);
        free(solution);
        printf("\n done;");
    }
    solution = NULL;
}

struct solution* GRASP_controller() {
    double timeElapsed = 0;
    clock_t start = 0, end = 0;
    int totalIterations = 1;
    int iterationsToBest = 0;
    double graspTime = 0, timeToBest = 0;
    clock_t GRASPstart = clock(), timeToBestStart = clock(), GRASPend = 0;
    struct solution* bestSolution = NULL;
    time_t ltime;
    printf("\n Starting GRASP with criterion type %d and parameter %d", config.GRASP_criterion_type, config.GRASP_criterion_parameter);
    if (config.GRASP_criterion_type == 1) {
        start = clock();
        while (timeElapsed < config.GRASP_criterion_parameter) {
            randomSeed++;
            randomizerConstructive = seedRand(randomSeed);
            randomizerLocalSearch = seedRand(randomSeed);
            struct solution* currentSolution = GRASP();
            if (currentSolution != NULL) {
                if (bestSolution != NULL) {
                    if (currentSolution->local_search_distance < bestSolution->local_search_distance) {
                        bestSolution = currentSolution;
                        timeToBest = ((double) (clock() - timeToBestStart)) / CLOCKS_PER_SEC;
                        iterationsToBest = totalIterations;
                    }
                } else {
                    bestSolution = currentSolution;
                    iterationsToBest = totalIterations;
                    timeToBest = ((double) (clock() - timeToBestStart)) / CLOCKS_PER_SEC;
                }
                printf("\n freeing solution from GRASP timebased");
                freeSolution(currentSolution);
            }
            end = clock();
            timeElapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
            ++totalIterations;
        }
    } else {
        int iteration = 1;
        while (iteration <= config.GRASP_criterion_parameter) {
            ltime = time(NULL);
            printf("\n %s \n iteration %d of GRASP", asctime(localtime(&ltime)), iteration);
            struct solution* currentSolution = GRASP();
            if (currentSolution != NULL) {
                if (bestSolution != NULL) {
                    if (currentSolution->local_search_distance < bestSolution->local_search_distance) {
                        bestSolution = currentSolution;
                        timeToBest = ((double) (clock() - timeToBestStart)) / CLOCKS_PER_SEC;
                        iterationsToBest = iteration;
                    }
                } else {
                    bestSolution = currentSolution;
                    timeToBest = ((double) (clock() - timeToBestStart)) / CLOCKS_PER_SEC;
                    iterationsToBest = iteration;
                }
            }
            printf("\n freeing solution from GRASP iteration");
            freeSolution(currentSolution);
            iteration++;
        }
    }
    GRASPend = clock();
    graspTime = ((double) (GRASPend - GRASPstart)) / CLOCKS_PER_SEC;
    bestSolution->GRASPTime = graspTime;
    bestSolution->timeToBestSolution = timeToBest;
    bestSolution->iterationsToBestSolution = iterationsToBest;
    return bestSolution;
}

void freeInstancesMemory(struct TSPLibData *tspLibData,
        struct TSPInstance *tspInstance) {
    if (tspInstance) {
        if (tspInstance->graphMatrix) {
            for (int i = 0; i < tspInstance->citiesAmount; i++) {
                if (tspInstance->graphMatrix[i]) {
                    free(tspInstance->graphMatrix[i]);
                }
            }
            free(tspInstance->graphMatrix);
        }
        //        if (tspInstance->nodes) {
        //            for (int i = 0; i < tspInstance->citiesAmount; i++) {
        //                freeList(tspInstance->nodes[i]->head);
        //                free(tspInstance->nodes[i]);
        //            }
        //            free(tspInstance->nodes);
        //        }
        free(tspInstance);
    }
    if (tspLibData) {
        if (tspLibData->x) {
            free(tspLibData->x);
        }
        if (tspLibData->y) {
            free(tspLibData->y);
        }
        free(tspLibData);
    }
}

char* concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void writeResultToFile(char* result) {
    fputs(result, resultsFile);
}

void printHeader() {
    writeResultToFile("name,n,constructive method, constructive distance, constructive calc time, local search "
            "method, localsearch distance, localsearch time, alpha, random seed, total time, alpha type, grasp time, timeToBest grasp, iterationsToBestGrasp");
}

void printLine(char* file, struct solution* solution) {
    char* line = (char*) malloc(256 * sizeof (char));
    char* lineChunk = (char*) malloc(50 * sizeof (char));
    sprintf(lineChunk, "\n");
    strcpy(line, lineChunk);
    sprintf(lineChunk, "%s,", file);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", tspInstance->citiesAmount);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%s,", getConstructiveMethodName(config.constructiveMethodIndex));
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", solution->constructive_distance);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%.6f,", solution->constructiveTime);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%s,", getLocalSearchMethodName(config.localSearchMethodIndex));
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", solution->local_search_distance);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%.6f,", solution->localSearchTime);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", config.alpha);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", randomSeed);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%.6f,", solution->totalTime);
    strcat(line, lineChunk);
    if (config.GRASP_criterion_type != 0) {
        sprintf(lineChunk, "%s,", config.alphaType);
        strcat(line, lineChunk);
        sprintf(lineChunk, "%.6f,", solution->GRASPTime);
        strcat(line, lineChunk);
        sprintf(lineChunk, "%.6f,", solution->timeToBestSolution);
        strcat(line, lineChunk);
        sprintf(lineChunk, "%d,", solution->iterationsToBestSolution);
        strcat(line, lineChunk);
    } else {
        sprintf(lineChunk, "%s", config.alphaType);
        strcat(line, lineChunk);
    }


    writeResultToFile(line);
    free(lineChunk);
    free(line);
}

void executeMethod(char* file) {
    printf("\nExecuting TSP for %s instance \n", file);
    clock_t start = 0, end = 0;
    double readTime = 0, allocationTime = 0, totalTime = 0;
    if (tspLibData != 0) {
        printf("\nInstance name: %s   file: %s", tspLibData->instanceName, file);
        if (strcmp(previousInstance, file) != 0) {
            freeInstancesMemory(tspLibData, tspInstance);
            //            printf("\n Reading different instance");
            start = clock();
            tspLibData = parseTSPLibFileEuclidian2D(file);
            end = clock();
        } else {
            //            printf("\nDidnt need to read again");
        }
    } else {
        printf("\nReading first time");
        start = clock();
        tspLibData = parseTSPLibFileEuclidian2D(file);
        end = clock();
        printf("\n Finished parsing file in %.6f seconds.", ((double) (end - start)) / CLOCKS_PER_SEC);
    }
    readTime = ((double) (end - start)) / CLOCKS_PER_SEC;
    if (tspLibData) {
        if (tspInstance != 0) {
            if (strcmp(previousInstance, file) != 0) {
                printf("\nCalculating distances.");
                start = clock();
                tspInstance = allocateTSPInstanceEuclidian2D(tspLibData);
                end = clock();
                //                printf("\nAllocating different instance");

            } else {
                //                printf("\nDidnt need to reallocate");
            }
        } else {
            //            printf("\nAllocating first time");
            printf("\nCalculating distances.");
            start = clock();
            tspInstance = allocateTSPInstanceEuclidian2D(tspLibData);
            end = clock();
            printf("\nFinished calculating distances in %.6f seconds.", ((double) (end - start)) / CLOCKS_PER_SEC);
        }
        allocationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
        struct solution* solution = NULL;
        //        struct solution* localSearchSolution = NULL;
        start = clock();
        if (tspInstance) {
            initializePairsMap();
            if (config.GRASP_criterion_type != 0) {
                solution = GRASP_controller();
            } else {
                solution = constructive_controller();
                solution = local_search_controller(solution);
            }
            end = clock();
            totalTime = ((double) (end - start)) / CLOCKS_PER_SEC;
            solution->totalTime = totalTime;
            printLine(file, solution);
            //            printf("\n<instance>\n");
            //            printf("\t<name>%s</name>\n", file);
            //            printf("\t<N>%d</N>", tspInstance->citiesAmount);
            //            printf("\t<ConstructiveMethod>%s</ConstructiveMethod>\n", getConstructiveMethodName(config.constructiveMethodIndex));
            //            printf("\t<ConstructiveDistance>%.2f</ConstructiveDistance>\n", constructiveSolution->distance);
            //            printf("\t<ConstructiveCalcTime>%f</ConstructiveCalcTime>\n", constructiveCalculationTime);
            //            printf("\t<LocalSearchMethod>%s</LocalSearchMethod>\n", getLocalSearchMethodName(config.localSearchMethodIndex));
            //            printf("\t<LocalSearchDistance>%.2f</LocalSearchDistance>\n", localSearchSolution->distance);
            //            printf("\t<LocalSearchCalcTime>%f</LocalSearchCalcTime>\n", localSearchCalculationTime);
            //            printf("\t<alpha>%d</alpha>\n", config.alpha);
            //            printf("\t<randomSeed>%d</randomSeed>\n", randomSeed);
            //            printf("</instance>");
            //            printTSPLibData(tspLibData);
            //            printInstanceData(tspInstance);
            //            verifyCanonicalDistance();
            //        printf("instance: %s \n \t distancia: %.2f tempos leitura: %f \t "
            //                "alocação: %f \t calculo %f  \n\n\n", file, distance, readTime, allocationTime, calculationTime);
            //            printInstanceData(tspInstance);
            printf("\n freeing solution from execute method");
            //            freeSolution(solution);
        }
    }
    strcpy(previousInstance, file);

    //    freeInstancesMemory(tspLibData, tspInstance);
}

void localSearchArguments(char *file, int localSearchIndex) {
    if (localSearchIndex == 5) {
        config.localSearchMethodIndex = 1;
        executeMethod(file);
        config.localSearchMethodIndex = 2;
        executeMethod(file);
        config.localSearchMethodIndex = 3;
        executeMethod(file);
        config.localSearchMethodIndex = 4;
        executeMethod(file);
    } else {
        executeMethod(file);
    }
}

void constructiveMethodArguments(char *file, int constructiveIndex, int localSearchIndex) {
    if (constructiveIndex == 4) {
        for (int x = 0; x < CONSTRUCTIVE_APPROACHES_AMOUNT; x++) {
            config.constructiveMethodIndex = x;
            //                        printConfigs();
            localSearchArguments(file, localSearchIndex);
        }
    } else {
        localSearchArguments(file, localSearchIndex);
    }
}

void execute(char *file) {
    int alpha = originalAlpha;
    int alphaUB = originalUBAlpha;
    int constructiveMethodIndex = originalConstructiveMethodIndex;
    int localSearchMethodIndex = originalLocalSearchMethodIndex;
    if (config.testAlpha == 1) {
        alphaUB = config.alphaUB;
    }
    for (int i = 0; i < config.repeatTimes; i++, randomSeed++) {
        randomizerConstructive = seedRand(randomSeed);
        randomizerLocalSearch = seedRand(randomSeed);
        for (int j = alpha; j <= alphaUB; j = j + config.alphaStep) {
            config.alpha = j;
            constructiveMethodArguments(file, constructiveMethodIndex, localSearchMethodIndex);
        }
    }
}

void executeMethodDir() {
    struct dirent *de; // Pointer for directory entry 
    DIR *dr = opendir(config.path);
    if (dr == NULL) {
        printf("\nCould not open current directory");
    } else {
        while ((de = readdir(dr)) != NULL)
            if (strcmp(de->d_name, ".") != 0
                    && strcmp(de->d_name, "..") != 0
                    && strcmp(de->d_name, ".DS_Store") != 0) {
                char *filePath1 = concat(config.path, "/");
                char *filePath2 = concat(filePath1, de->d_name);
                execute(filePath2);
                free(filePath1);
                free(filePath2);
                //                        printf("%s\n",de->d_name);
            }
        closedir(dr);
    }
}

void printArguments(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        printf("[%d]: %s\n", i, argv[i]);
    }
}

int getConstructiveMethodIndex() { //index 0 = NN
    int index = 0; //NN
    if (strcmp(config.constructiveMethod, "DSNN") == 0) {
        index = 1;
    } else if (strcmp(config.constructiveMethod, "RNN") == 0) {
        index = 2;
    } else if (strcmp(config.constructiveMethod, "RDSNN") == 0) {
        index = 3;
    } else if (strcmp(config.constructiveMethod, "all") == 0) {
        index = 4;
    }
    return index;
}

int getLocalSearchMethodIndex() {
    int index = 0;
    if (strcmp(config.localSearchMethod, "first") == 0) {
        index = 1;
    } else if (strcmp(config.localSearchMethod, "best") == 0) {
        index = 2;
    } else if (strcmp(config.localSearchMethod, "first_2") == 0) {
        index = 3;
    } else if (strcmp(config.localSearchMethod, "best_2") == 0) {
        index = 4;
    } else if (strcmp(config.localSearchMethod, "both") == 0) {
        index = 5;
    }
    return index;
}

//qt is 0, ql is 1

int getAlphaTypeIndex() {
    int index = 0;
    if (strcmp(config.alphaType, "ql") == 0) {
        index = 1;
    }
    return index;
}

void invokeExecution() {
    if (strcmp(config.mode, "folder") == 0) {
        executeMethodDir();
    } else if (strcmp(config.mode, "file") == 0) {
        execute(config.path);
    } else {
        printf("\n\n------->Invalid mode!<--------- Choose file or folder mode.");
    }
}

void printHelp() {
    printf("Usage: \n "
            "argument 1: log file name \n"
            "argument 2: constructive method (Accepted values: NN, DSNN, RNN, RDSNN, all) \n"
            "argument 3 local search method (Accepted values: best, first, best_2, first_2 none) \n"
            "argument 4: times to repeat \n"
            "argument 5: mode. Accepted values: folder or file \n "
            "argument 6: path \n"
            "argument 7: 0 use timestamp initial seed or use specified initial seed\n"
            "argument 8: RCL type: 'qt' for quatity-based, 'ql' for quality-based\n"
            "argument 9: GRASP type: 0 for disabled | 1 for time based | 2 for iteration based \n"
            "argument 10: GRASP parameter (time or iteration) limit \n"
            "[argument 11: alpha value *optional*] \n "
            "[argument 12: alpha value upper bound *optional* (if provided, argument 5 will be treated as the lower bound and all values will be tested]\n"
            "[argument 13: alpha incremental step\n");

    printf("\n Note that if you choose a method that is random-based (RNN, RDSNN, all),"
            " you MUST provide the alpha value.");
    printf("\n Example: ./tspProblem execution1 DSNN first 10 folder instances 0");
    printf("\n Example 2: ./tspProblem execution1 RDSNN none 5 file instances/a280.tsp 2 qt 2 50 20 ");
    printf("\n Example 3: ./tspProblem myexecution all first 5 file instances/a280.tsp 0 qt 1 60 1 10 2 \n");
}

void printConfigs() {
    printf("\nlog name: %s", config.logName);
    printf("\nconstructive method: %s", config.constructiveMethod);
    printf("\nlocal search method: %s", config.localSearchMethod);
    printf("\nmode: %s", config.mode);
    printf("\npath: %s", config.path);
    printf("\nconstructive methodIndex: %d", config.constructiveMethodIndex);
    printf("\nlocal search methodIndex: %d", config.localSearchMethodIndex);
    printf("\nrepeatTimes: %d", config.repeatTimes);
    printf("\nalpha: %d", config.alpha);
    printf("\nalphaUB: %d", config.alphaUB);
    printf("\nalpha type: %s", config.alphaType);
    printf("\nalpha index: %d", config.alphaTypeIndex);
    printf("\ntestAlpha: %d", config.testAlpha);
    printf("\nAlpha incremental step: %d", config.alphaStep);
    printf("\n initial seed %d", config.initialSeed);
    printf("\n GRASP type (0 disabled, 1 time, 2 iteration): %d", config.GRASP_criterion_type);
    printf("\n GRASP parameter: %d\n", config.GRASP_criterion_parameter);
}

int main(int argc, char** argv) {
    //        printArguments(argc, argv);
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc >= 8) {
        config.logName = malloc(strlen(argv[1]) + 5);
        sprintf(config.logName, "%s.csv", argv[1]);
        config.constructiveMethod = argv[2];
        config.localSearchMethod = argv[3];
        config.constructiveMethodIndex = getConstructiveMethodIndex();
        config.localSearchMethodIndex = getLocalSearchMethodIndex();
        config.repeatTimes = atoi(argv[4]);
        config.mode = argv[5];
        config.path = argv[6];
        config.initialSeed = atoi(argv[7]);
        config.alphaStep = 10;
        if (argc >= 9) {
            config.alphaType = argv[8];
            config.alphaTypeIndex = getAlphaTypeIndex();
            if (argc >= 11) {
                config.GRASP_criterion_type = atoi(argv[9]);
                config.GRASP_criterion_parameter = atoi(argv[10]);
                if (argc >= 12) {
                    config.alpha = atoi(argv[11]);
                    if (argc >= 13) {
                        config.alphaUB = atoi(argv[12]);
                        config.testAlpha = 1;
                        if (argc >= 14) {
                            config.alphaStep = atoi(argv[13]);
                        }
                    }
                }

            } else {
                config.GRASP_criterion_type = 0;
            }
        } else {
            config.alpha = 0;
            config.alphaType = 0;
        }
        if (config.initialSeed == 0) {
            config.initialSeed = time(NULL);
        }
        randomSeed = config.initialSeed;
        originalAlpha = config.alpha;
        originalUBAlpha = config.alpha;
        originalConstructiveMethodIndex = config.constructiveMethodIndex;
        originalLocalSearchMethodIndex = config.localSearchMethodIndex;
        time_t ltime;
        ltime = time(NULL);
        printf("\n\n\n");
        printConfigs();
        printf("\nStarted at: %s", asctime(localtime(&ltime)));
        printf("\nSaving results to file %s\n", config.logName);
        resultsFile = fopen(config.logName, "a");
        printHeader();
        invokeExecution();
        fclose(resultsFile);
        ltime = time(NULL);
        printf("\nFinished at: %s", asctime(localtime(&ltime)));
    } else {
        printHelp();
    }
    return (EXIT_SUCCESS);
}