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
/*
 * 
 */

#define BIG_DISTANCE 999999
#define CONSTRUCTIVE_APPROACHES_AMOUNT 4
#define RANDOM_SEED 2

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
    int* route;
    double distance;
};

struct Config {
    char* constructiveMethod;
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
};

struct Config config;

struct TSPLibData *tspLibData = 0;
struct TSPInstance *tspInstance = 0;
char previousInstance[4096] = "";
int originalAlpha = 0;
int originalUBAlpha = 0;
int originalConstructiveMethodIndex = 0;
int originalLocalSearchMethodIndex = 0;
int randomSeed = RANDOM_SEED;
FILE *resultsFile;
MTRand r;

int nint(double x) {
    return (int) (x + 0.5);
}

int getRandomInt(int lb, int ub) {
    double randN = genRand(&r);
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
        printf("\n to %d: %f",
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
    char * line = NULL;
    size_t len = 0;
    ssize_t read = 0;
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
            for (int i = 0; i < instance->citiesAmount; i++) {
                instance->graphMatrix[i] = calloc(instance->citiesAmount, sizeof *(instance->graphMatrix));
            }
            for (int i = 0; i < instance->citiesAmount; i++) {
                for (int j = 0; j < instance->citiesAmount; j++) {
                    instance->graphMatrix[i][j] = getEuclidianDistance(i, j, data);
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
        char * line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        int dataSection = 0;
        int vertexAmount = 0;
        int vertexCounter = 0;
        fp = fopen(filePath, "r");
        if (fp) {
            while ((read = getline(&line, &len, fp)) != -1) {
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
}

void printRoute(int route[], int routeSize, double length) {
    printf("\nRoute: ");
    for (int i = 0; i < routeSize; i++) {
        printf(" %d ", route[i]);
    }
    printf("\nTotal length (provided): %f, total length (calculated): %f, total cities: %d", length, getRouteDistance(route, routeSize), routeSize - 1);
}

int getNearestVertexNotVisited(int distances[], int vertexAmount, int origin, int visitedVertexes[]) {
    double minDistance = BIG_DISTANCE;
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

int * twoOptSwap(int* route, int routeSize, int i, int j) {
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

double calculateNewDistance(double oldDistance, int swappedPosition1, int swappedPosition2, int* oldRoute, int* newRoute) {
    int pair1_old_x = oldRoute[swappedPosition1 - 1];
    int pair1_old_y = oldRoute[swappedPosition1];
    int pair2_old_x = oldRoute[swappedPosition2];
    int pair2_old_y = oldRoute[swappedPosition2 + 1];

    int pair1_new_x = newRoute[swappedPosition1 - 1];
    int pair1_new_y = newRoute[swappedPosition1];
    int pair2_new_x = newRoute[swappedPosition2];
    int pair2_new_y = newRoute[swappedPosition2 + 1];

    double subtract1 = tspInstance->graphMatrix[pair1_old_x][pair1_old_y];
    double subtract2 = tspInstance->graphMatrix[pair2_old_x][pair2_old_y];
    double add1 = tspInstance->graphMatrix[pair1_new_x][pair1_new_y];
    double add2 = tspInstance->graphMatrix[pair2_new_x][pair2_new_y];
    //    printf("\n neighborDistance = oldDistance - d(%d,%d) - d(%d,%d) + d(%d,%d) + d(%d,%d)", pair1_old_x, pair1_old_y, pair2_old_x, pair2_old_y, pair1_new_x, pair1_new_y,
    //            pair2_new_x, pair2_new_y);
    double newDistance = oldDistance - subtract1 - subtract2 + add1 + add2;
    //    printf("\n %f = %f - %f - %f + %f + %f", newDistance, oldDistance, subtract1, subtract2, add1, add2);
    return newDistance;
}

int searchFirstImprovementNeighbor(int solutionSize, struct solution* currentSolution) {
    for (int i = 1; i < solutionSize - 1; i++) {
        for (int j = i + 1; j < solutionSize - 1; j++) {
            int* neighbor = twoOptSwap(currentSolution->route, solutionSize, i, j);
            double neighborDistance = calculateNewDistance(currentSolution->distance, i, j, currentSolution->route, neighbor);
            //            printf("\n Neighbor: ");
            //            printRoute(neighbor, solutionSize, neighborDistance);
            if (neighborDistance < currentSolution->distance) {
                currentSolution->distance = neighborDistance;
                memcpy(currentSolution->route, neighbor, solutionSize * sizeof (int));
                free(neighbor);
                return 1;
            }
            free(neighbor);
        }
    }
    return 0;
}

int searchBestImprovementNeighbor(int solutionSize, struct solution* currentSolution) {
    int improvementFound = 0;
    struct solution* bestKnownSolution = malloc(sizeof *(bestKnownSolution));
    bestKnownSolution->route = calloc(solutionSize, sizeof (int));
    memcpy(bestKnownSolution->route, currentSolution->route, solutionSize * sizeof (int));
    bestKnownSolution->distance = currentSolution->distance;
    //    printf("\nStarting best improvement neighbor search. Current solution:");
    //    printRoute(bestKnownSolution->route, solutionSize, bestKnownSolution->distance);
    for (int i = 1; i < solutionSize - 1; i++) {
        for (int j = i + 1; j < solutionSize - 1; j++) {
            int* neighbor = twoOptSwap(currentSolution->route, solutionSize, i, j);
            double neighborDistance = calculateNewDistance(currentSolution->distance, i, j, currentSolution->route, neighbor);
            //            printf("\n Neighbor: ");
            //            printRoute(neighbor, solutionSize, neighborDistance);
            if (neighborDistance < bestKnownSolution->distance) {
                bestKnownSolution->distance = neighborDistance;
                memcpy(bestKnownSolution->route, neighbor, solutionSize * sizeof (int));
                //                bestKnownSolution->route = neighbor;
            }
            free(neighbor);
            //            getchar();
        }
    }
    //    printf("\n Best neighbor:");
    //    printRoute(bestKnownSolution->route, solutionSize, bestKnownSolution->distance);
    if (bestKnownSolution->distance < currentSolution->distance) {
        currentSolution->distance = bestKnownSolution->distance;
        memcpy(currentSolution->route, bestKnownSolution->route, solutionSize * sizeof (int));
        improvementFound = 1;
    }
    free(bestKnownSolution->route);
    free(bestKnownSolution);
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

struct solution* localSearch(struct solution* constructiveSolution, int constructiveSolutionSize) {
    struct solution* currentSolution = malloc(sizeof *(currentSolution));
    currentSolution->distance = constructiveSolution->distance;
    currentSolution->route = constructiveSolution->route;
    //    printf("\n Constructive solution: ");
    //    printRoute(constructiveSolution->route, constructiveSolutionSize, constructiveSolution->distance);
    int keepSearching = 1;
    do {
        //                    printf("\n Current solution: ");
        //                    printRoute(currentSolution->route, constructiveSolutionSize, currentSolution->distance);
        int newSolution = 0;
        switch (config.localSearchMethodIndex) {
            case 1:
                newSolution = searchFirstImprovementNeighbor(constructiveSolutionSize, currentSolution);
                break;
            case 2:
                newSolution = searchBestImprovementNeighbor(constructiveSolutionSize, currentSolution);
                break;
        }
        if (newSolution == 0) {
            //                            printf("\nLocal optimum found. Stopping.");
            keepSearching = 0;
        } else {
            //                            printf("\n Improvement found. Moving to neighbor.");
            //                            printf("\n Current solution: ");
            //                            printRoute(currentSolution->route, constructiveSolutionSize, currentSolution->distance);
        }
        //            printf("\n\n\n");
        //        getchar();
    } while (keepSearching == 1);
//    printRoute(currentSolution->route, constructiveSolutionSize, currentSolution->distance);
    //    testRoute();
    return currentSolution;
}

int* getNNearestVertexNotVisited(int distances[], int vertexAmount, int origin, int visitedVertexes[], int N) {
    struct NeighborDistance *nd = calloc(vertexAmount, sizeof (*nd));
    //TODO usar memcpy aqui
    for (int i = 0; i < vertexAmount; i++) {
        nd[i].distanceFromOrigin = distances[i];
        nd[i].target = i;
    }
    sortNeighborsByDistance(nd, vertexAmount);
//    printNd(nd, vertexAmount);
    
//    for (int i = 0; i < vertexAmount; i++) {
////        printf("\ndistance to %d=%d",i,distances[i]);
//    }
    int* NNearestNotVisited = (int*) calloc(N, sizeof (int));
    initializeArray(NNearestNotVisited, N, -1);
    int notVisitedIndex = 0;
    for (int j = 0; j < vertexAmount; j++) {
        int current = nd[j].target;
        if (visitedVertexes[current] == 0 && origin != current
                && notVisitedIndex < N) {
            NNearestNotVisited[notVisitedIndex++] = current;
        }
        if (notVisitedIndex >= N) {
            break;
        }
    }
//            printf("\nN nearest not yet visited: ");
            for (int i = 0; i < notVisitedIndex; i++) {
//                printf("\n %d", NNearestNotVisited[i]);
            }

    if (nd) {
        free(nd);
    }

    return NNearestNotVisited;
}

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
        solution->route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->distance = totalDistance;
        memcpy(solution->route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
//                printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
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

int getRandomNearestNotVisited(int distances[], int vertexAmount, int origin,
        int visitedVertexes[], int candidateSetSize) {
    clock_t start = 0, end = 0;
    double functionTime = 0;
    start = clock();
    int randomNearestNotVisited = -1;
    if (candidateSetSize != 0) {
        int* NNearestNotVisited = getNNearestVertexNotVisited(distances,
                vertexAmount, origin, visitedVertexes, candidateSetSize);
//        printf("\n origin: %d", origin);
//        printf("\nN nearest not yet visited: ");
        for (int i = 0; i < candidateSetSize; i++) {
//            printf("\n %d", NNearestNotVisited[i]);
        }
        int randomIndex = -1;
        do {
            randomIndex = getRandomInt(0, candidateSetSize - 1);
            //            if(randomIndex==-1){
            //                printf("\n Searching another one");
            //            }
        } while (NNearestNotVisited[randomIndex] == -1);
        //                printf("\n candidateSize: %d random choosen: %d\n", candidateSetSize, randomIndex);
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
        //        printf("\n candidateSetSize: %d\n", candidateSetSize);
        int visitNumber = 1;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            int randomNearestNotVisited = getRandomNearestNotVisited(instance->graphMatrix[lastVisited],
                    instance->citiesAmount, lastVisited, visitedVertexes, candidateSetSize);
            //            printf("\n%d", randomNearestNotVisited);
            if (randomNearestNotVisited != -1) {
                visitedVertexes[randomNearestNotVisited] = 1;
                totalDistance += instance->graphMatrix[lastVisited][randomNearestNotVisited];
                routeOrder[visitNumber] = randomNearestNotVisited;
                //                printf("\n random nearest: %d  totalDistance: %f visitnumber: %d", randomNearestNotVisited, totalDistance, visitNumber);
                lastVisited = randomNearestNotVisited;
            }
        }
        totalDistance = finishCycle(startingNode, totalDistance, routeOrder, instance->citiesAmount + 1);
//                        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        struct solution* solution = malloc(sizeof *(solution));
        solution->route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->distance = totalDistance;
        memcpy(solution->route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
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
            int randomNearestNotVisitedHead1 = getRandomNearestNotVisited(instance->graphMatrix[head1],
                    instance->citiesAmount, head1, visitedVertexes, candidateSetSize);
            int randomNearestNotVisitedHead2 = getRandomNearestNotVisited(instance->graphMatrix[head2],
                    instance->citiesAmount, head2, visitedVertexes, candidateSetSize);
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
        solution->route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->distance = totalDistance;
        memcpy(solution->route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
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
        solution->route = calloc(instance->citiesAmount + 1, sizeof (int));
        solution->distance = totalDistance;
        memcpy(solution->route, routeOrder, (instance->citiesAmount + 1) * sizeof (int));
//        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
        //        testRoute();
        return solution;
    }

    return NULL;
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
    }
}

void writeResultToFile(char* result) {
    fputs(result, resultsFile);
}

void printHeader() {
    writeResultToFile("name,n,constructive method, constructive distance, constructive calc time, local search "
            "method, localsearch distance, localsearch time, alpha, random seed");
}

void printLine(char* file, struct solution* constructiveSolution, struct solution* localSearchSolution,
        double constructiveCalculationTime, double localSearchCalculationTime) {
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
    sprintf(lineChunk, "%.2f,", constructiveSolution->distance);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%f,", constructiveCalculationTime);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%s,", getLocalSearchMethodName(config.localSearchMethodIndex));
    strcat(line, lineChunk);
    sprintf(lineChunk, "%.2f,", localSearchSolution->distance);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%f,", localSearchCalculationTime);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d,", config.alpha);
    strcat(line, lineChunk);
    sprintf(lineChunk, "%d", randomSeed);
    strcat(line, lineChunk);
    writeResultToFile(line);
    free(lineChunk);
    free(line);
}

void executeMethod(char* file) {
    //    printf("\nExecuting TSP for %s instance \n", file);
    clock_t start = 0, end = 0;
    double constructiveCalculationTime = 0, readTime = 0, allocationTime = 0, localSearchCalculationTime = 0;
    if (tspLibData != 0) {
        //        printf("\nInstance name: %s   file: %s", tspLibData->instanceName, file);
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
        //        printf("\nReading first time");
        start = clock();
        tspLibData = parseTSPLibFileEuclidian2D(file);
        end = clock();
    }
    readTime = ((double) (end - start)) / CLOCKS_PER_SEC;
    if (tspLibData) {

        if (tspInstance != 0) {
            if (strcmp(previousInstance, file) != 0) {
                start = clock();
                tspInstance = allocateTSPInstanceEuclidian2D(tspLibData);
                end = clock();
                //                printf("\nAllocating different instance");
            } else {
                //                printf("\nDidnt need to reallocate");
            }
        } else {
            //            printf("\nAllocating first time");
            start = clock();
            tspInstance = allocateTSPInstanceEuclidian2D(tspLibData);
            end = clock();
        }
        allocationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
        struct solution* constructiveSolution = NULL;
        struct solution* localSearchSolution = NULL;
        if (tspInstance) {
            start = clock();
            switch (config.constructiveMethodIndex) {
                default:
                    //                    config.alpha = 0;
                    constructiveSolution = symmetricGreedyTSP(0, tspInstance);
                    break;
                case 1:
                    //                    config.alpha = 0;
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
            constructiveCalculationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
            start = clock();
            if (config.localSearchMethodIndex == 0) {
                localSearchSolution = malloc(sizeof *(localSearchSolution));
                localSearchSolution->distance = 0;
                localSearchCalculationTime = 0;
            } else {
                if (constructiveSolution != NULL) {
                    //                        printRoute(constructiveSolution->route, tspInstance->citiesAmount + 1, constructiveSolution->distance);
                    localSearchSolution = localSearch(constructiveSolution, tspInstance->citiesAmount + 1);
                }
            }
            end = clock();
            localSearchCalculationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
            printLine(file, constructiveSolution, localSearchSolution, constructiveCalculationTime, localSearchCalculationTime);
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
            if (localSearchSolution != NULL) {
                //                if (localSearchSolution->distance != 0) {
                //                    free(localSearchSolution->route);
                //                }
                free(localSearchSolution);
            }
            if (constructiveSolution != NULL) {
                if (constructiveSolution->route != NULL) {
                    free(constructiveSolution->route);
                }
                free(constructiveSolution);
            }
        }

    }
    strcpy(previousInstance, file);

    //    freeInstancesMemory(tspLibData, tspInstance);
}

void localSearchArguments(char *file, int localSearchIndex) {
    if (localSearchIndex == 3) {
        config.localSearchMethodIndex = 1;
        executeMethod(file);
        config.localSearchMethodIndex = 2;
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
        r = seedRand(randomSeed);
        for (int j = alpha; j <= alphaUB; j = j + 10) {
            config.alpha = j;
            constructiveMethodArguments(file, constructiveMethodIndex, localSearchMethodIndex);
        }
    }
}

void executeMethodDir() {
    struct dirent *de; // Pointer for directory entry 
    DIR *dr = opendir(config.path);
    if (dr == NULL) {
        printf("Could not open current directory");
    }

    while ((de = readdir(dr)) != NULL)
        if (strcmp(de->d_name, ".") != 0
                && strcmp(de->d_name, "..") != 0
                && strcmp(de->d_name, ".DS_Store") != 0) {
            char *filePath = concat(concat(config.path, "/"), de->d_name);
            execute(filePath);
            free(filePath);
            //                        printf("%s\n",de->d_name);
        }
    closedir(dr);
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
    } else if (strcmp(config.localSearchMethod, "both") == 0) {
        index = 3;
    }
    return index;
}

void invokeExecution() {
    if (strcmp(config.mode, "folder") == 0) {
        executeMethodDir();
    } else if (strcmp(config.mode, "file") == 0) {
        execute(config.path);
    }
}

void printHelp() {
    printf("Usage: \n "
            "argument1: log file name"
            "argument2: constructive method (Accepted values: NN, DSNN, RNN, RDSNN, all) \n"
            "argument3 local search method (Accepted values: best, first, none) \n"
            "argument4: times to repeat \n"
            "argument5: mode. Accepted values: folder or file \n "
            "argument6: path \n"
            "[argument 7: alpha value *optional*] \n "
            "[argument 8: alpha value upper bound *optional* (if provided, argument 5 will be treated as the lower bound and all values will be tested]");

    printf("\n Note that if you choose a method that is random-based (RNN, RDSNN, all),"
            " you MUST provide the alpha value.");
    printf("\n Example: ./tspProblem execution1 DSNN first 10 folder instances");
    printf("\n Example 2: ./tspProblem execution1 RDSNN none 5 file instances/a280.tsp 20");
    printf("\n Example 3: ./tspProblem myexecution all first 5 file instances/a280.tsp 10 \n");
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
    printf("\ntestAlpha: %d\n", config.testAlpha);
}

int main(int argc, char** argv) {
    //        printArguments(argc, argv);
    if (argc >= 7) {
        config.logName = argv[1];
        config.constructiveMethod = argv[2];
        config.localSearchMethod = argv[3];
        config.constructiveMethodIndex = getConstructiveMethodIndex();
        config.localSearchMethodIndex = getLocalSearchMethodIndex();
        config.repeatTimes = atoi(argv[4]);
        config.mode = argv[5];
        config.path = argv[6];
        if (argc >= 8) {
            config.alpha = atoi(argv[7]);
            if (argc >= 9) {
                config.alphaUB = atoi(argv[8]);
                config.testAlpha = 1;
            }
        } else {
            config.alpha = 0;
        }

        originalAlpha = config.alpha;
        originalUBAlpha = config.alpha;
        originalConstructiveMethodIndex = config.constructiveMethodIndex;
        originalLocalSearchMethodIndex = config.localSearchMethodIndex;
        //        printConfigs();
        //        printf("\n alpha = %d  alphaUB = %d \n", alpha, alphaUB);
        //        printf("<instances>");
        strcat(config.logName, ".csv");

        printf("\nSaving results to file %s\n", config.logName);
        resultsFile = fopen(config.logName, "w+");
        printHeader();
        invokeExecution();
        fclose(resultsFile);
    } else {
        printHelp();
    }
    return (EXIT_SUCCESS);
}