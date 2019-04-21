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
#include "TSPutil.h"
#include "mtwister.h"
/*
 * 
 */

#define BIG_DISTANCE 999999
#define SOLVING_APPROACHES_AMOUNT 4
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
    double** graphMatrix;
};

struct Config {
    char* method;
    int methodIndex;
    int repeatTimes;
    char* mode;
    char* path;
    int alpha;
    int alphaUB;
    int testAlpha;
};

struct Config config;

struct TSPLibData *tspLibData = 0;
struct TSPInstance *tspInstance = 0;
char previousInstance[4096] = "";
int originalAlpha = 0;
int originalUBAlpha = 0;
int originalMethodIndex = 0;
int randomSeed = RANDOM_SEED;

MTRand r;

int nint(int x) {
    return (int) (x + 0.5);
}

int getRandomInt(int lb, int ub) {
    double randN = genRand(&r);
    int generatedInt = randN * (ub - lb + 1) + lb;
    return generatedInt;
}

void printNd(struct NeighborDistance *nd, int size) {
    for (int i = 0; i < size; i++) {
        printf("\n to %d: %f",
                nd[i].target,
                nd[i].distanceFromOrigin);
    }
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
        printf("\n %f %f", data->x[i], data->y[i]);
    }
}

void printInstanceData(struct TSPInstance *instance) {
    for (int i = 1; i < instance->citiesAmount; i++) {
        for (int j = 1; j < instance->citiesAmount; j++) {
            if (j == instance->citiesAmount) {
                printf("\n");
            }
            printf("\nmatrix[%d][%d]=%.2f", i, j, instance->graphMatrix[i][j]);

        }
    }
}

void printRoute(int route[], int routeSize, double length) {
    printf("\nRoute: ");
    for (int i = 0; i < routeSize; i++) {
        printf(" %d ", route[i]);
    }
    printf("\nTotal length: %f, total cities: %d", length, routeSize - 1);
}

int getNearestVertexNotVisited(double distances[], int vertexAmount, int origin, int visitedVertexes[]) {
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

int* getNNearestVertexNotVisited(double distances[], int vertexAmount, int origin, int visitedVertexes[], int N) {
    struct NeighborDistance *nd = calloc(vertexAmount, sizeof (*nd));

    for (int i = 0; i < vertexAmount; i++) {
        nd[i].distanceFromOrigin = distances[i];
        nd[i].target = i;
    }
    sortNeighborsByDistance(nd, vertexAmount);
    //        printNd(nd, vertexAmount);
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
    //        printf("\nN nearest not yet visited: ");
    //        for (int i = 0; i < notVisitedIndex; i++) {
    //            printf("\n %d", NNearestNotVisited[i]);
    //        }

    if (nd) {
        free(nd);
    }

    return NNearestNotVisited;
}

double finishCycle(int startingNode, int lastNode, double totalDistance, int* route, int lastRouteIndex) {
    route[lastRouteIndex] = startingNode;
    totalDistance += tspInstance->graphMatrix[lastNode][startingNode];
    return totalDistance;
}

double symmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {  
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
        totalDistance = finishCycle(startingNode, lastVisited, totalDistance, routeOrder, visitNumber);
//                printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);

    }
    return totalDistance;
}

int getCandidatesSetSize(int neighborsAmount) {
    if (config.alpha == 0) {
        return 1;
    }
    double sizeD = neighborsAmount * ((double) config.alpha / 100);
    return ceil(sizeD);
}

int getRandomNearestNotVisited(double distances[], int vertexAmount, int origin,
        int visitedVertexes[], int candidateSetSize) {
    clock_t start = 0, end = 0;
    double functionTime = 0;
    start = clock();
    int randomNearestNotVisited = -1;
    if (candidateSetSize != 0) {
        int* NNearestNotVisited = getNNearestVertexNotVisited(distances,
                vertexAmount, origin, visitedVertexes, candidateSetSize);
        //                printf("\n origin: %d", origin);
        //                printf("\nN nearest not yet visited: ");
        //                for (int i = 0; i < candidateSetSize; i++) {
        //                    printf("\n %d", NNearestNotVisited[i]);
        //                }
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

double randomSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) { 
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
        totalDistance = finishCycle(startingNode, lastVisited, totalDistance, routeOrder, visitNumber);
//        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
    }
    return totalDistance;
}

double randomDoubleSidedSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;

    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        int routeOrder[instance->citiesAmount + 1];

        int head1 = startingNode;
        int head2 = startingNode;

        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int candidateSetSize = getCandidatesSetSize(instance->citiesAmount - 1);
        int visitNumber = 1;
        int lastVisitedNode = startingNode;
        int lastVisitedDistance = 0;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            //            printf("\nHead1: %d   || Head2: %d", head1, head2);


            int randomNearestNotVisitedHead1 = getRandomNearestNotVisited(instance->graphMatrix[head1],
                    instance->citiesAmount, head1, visitedVertexes, candidateSetSize);

            int randomNearestNotVisitedHead2 = getRandomNearestNotVisited(instance->graphMatrix[head2],
                    instance->citiesAmount, head2, visitedVertexes, candidateSetSize);



            if (randomNearestNotVisitedHead1 != -1 && randomNearestNotVisitedHead2 != -1) {
                double nearestNotVisitedHead1Distance = instance->graphMatrix[head1][randomNearestNotVisitedHead1];
                double nearestNotVisitedHead2Distance = instance->graphMatrix[head2][randomNearestNotVisitedHead2];
                //                printf("\n nearestHead1: %d distance: %f || nearestHead2: %d distance: %f",
                //                        nearestNotVisitedHead1, nearestNotVisitedHead1Distance, nearestNotVisitedHead2, nearestNotVisitedHead2Distance);
                if (nearestNotVisitedHead1Distance <= nearestNotVisitedHead2Distance) {
                    head1 = randomNearestNotVisitedHead1;
                    lastVisitedNode = head1;
                    lastVisitedDistance = nearestNotVisitedHead1Distance;
                    //                    printf("\n Visitei Head1");
                } else {
                    head2 = randomNearestNotVisitedHead2;
                    lastVisitedNode = head2;
                    lastVisitedDistance = nearestNotVisitedHead2Distance;
                    //                    printf("\n Visitei Head2");
                }
                visitedVertexes[lastVisitedNode] = 1;
                totalDistance += lastVisitedDistance;
                routeOrder[visitNumber] = lastVisitedNode;
            }
        }
        totalDistance = finishCycle(startingNode, lastVisitedNode, totalDistance, routeOrder, visitNumber);
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
    }

    return totalDistance;
}

double doubleSidedSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance) {
    double totalDistance = 0;

    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);
        int routeOrder[instance->citiesAmount + 1];

        int head1 = startingNode;
        int head2 = startingNode;
        int lastVisitedNode = startingNode;
        double lastVisitedNodeDistance = 0;
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int visitNumber = 1;
        for (; visitNumber < instance->citiesAmount; visitNumber++) {
            //            printf("\nHead1: %d   || Head2: %d", head1, head2);
            int nearestNotVisitedHead1 = getNearestVertexNotVisited(instance->graphMatrix[head1],
                    instance->citiesAmount, head1, visitedVertexes);

            int nearestNotVisitedHead2 = getNearestVertexNotVisited(instance->graphMatrix[head2],
                    instance->citiesAmount, head2, visitedVertexes);
            if (nearestNotVisitedHead1 != -1 && nearestNotVisitedHead2 != -1) {
                double nearestNotVisitedHead1Distance = instance->graphMatrix[head1][nearestNotVisitedHead1];
                double nearestNotVisitedHead2Distance = instance->graphMatrix[head2][nearestNotVisitedHead2];
                //                printf("\n nearestHead1: %d distance: %f || nearestHead2: %d distance: %f",
                //                        nearestNotVisitedHead1, nearestNotVisitedHead1Distance, nearestNotVisitedHead2, nearestNotVisitedHead2Distance);
                if (nearestNotVisitedHead1Distance <= nearestNotVisitedHead2Distance) {
                    head1 = nearestNotVisitedHead1;
                    lastVisitedNode = head1;
                    lastVisitedNodeDistance = nearestNotVisitedHead1Distance;
                    //                    printf("\n Visitei Head1");
                } else {
                    head2 = nearestNotVisitedHead2;
                    lastVisitedNode = head2;
                    lastVisitedNodeDistance = nearestNotVisitedHead2Distance;
                    //                    printf("\n Visitei Head2");
                }
                visitedVertexes[lastVisitedNode] = 1;
                totalDistance += lastVisitedNodeDistance;
                routeOrder[visitNumber] = lastVisitedNode;

            }
        }
        totalDistance = finishCycle(startingNode, lastVisitedNode, totalDistance, routeOrder, visitNumber);
        //        printRoute(routeOrder, instance->citiesAmount + 1, totalDistance);
    }

    return totalDistance;
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

char* getMethodName(int methodIndex) { //index 0 = NN
    switch (methodIndex) {
        default: return "Nearest neighbor (NN)";
        case 1: return "Double-sided nearest neighbor (DSNN)";
        case 2: return "Random Nearest Neighbor (RNN)";
        case 3: return "Random Double-sided nearest neighbor (RDSNN)";
    }
}

void executeMethod(char* file) {
    //    printf("\nExecuting TSP for %s instance \n", file);
    clock_t start = 0, end = 0;
    double calculationTime = 0, readTime = 0, allocationTime = 0;
    double distance = 0;
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
        if (tspInstance) {
            start = clock();
            switch (config.methodIndex) {
                default:
                    //                    config.alpha = 0;
                    distance = symmetricGreedyTSP(1, tspInstance);
                    break;
                case 1:
                    //                    config.alpha = 0;
                    distance = doubleSidedSymmetricGreedyTSP(1, tspInstance);
                    break;
                case 2:
                    distance = randomSymmetricGreedyTSP(1, tspInstance);
                    break;
                case 3:
                    distance = randomDoubleSidedSymmetricGreedyTSP(1, tspInstance);
                    break;
            }
            end = clock();
            calculationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("\n<instance>\n");
            printf("\t<name>%s</name>\n", file);
            printf("\t<method>%s</method>\n", getMethodName(config.methodIndex));
            printf("\t<distance>%.2f</distance>\n", distance);
            printf("\t<calcTime>%f</calcTime>\n", calculationTime);
            printf("\t<allocTime>%f</allocTime>\n", allocationTime);
            printf("\t<readTime>%f</readTime>\n", readTime);
            printf("\t<alpha>%d</alpha>\n", config.alpha);
            printf("\t<randomSeed>%d</randomSeed>\n", randomSeed);
            printf("</instance>");
            //        printf("instance: %s \n \t distancia: %.2f tempos leitura: %f \t "
            //                "alocação: %f \t calculo %f  \n\n\n", file, distance, readTime, allocationTime, calculationTime);
        }
    }
    strcpy(previousInstance, file);
    //    freeInstancesMemory(tspLibData, tspInstance);
}

void execute(char *file) {
    int alpha = originalAlpha;
    int alphaUB = originalUBAlpha;
    int methodIndex = originalMethodIndex;

    if (config.testAlpha == 1) {
        alphaUB = config.alphaUB;
    }
    for (int i = 0; i < config.repeatTimes; i++, randomSeed++) {
        r = seedRand(randomSeed);
        for (int j = alpha; j <= alphaUB; j = j + 2) {
            config.alpha = j;
            if (methodIndex == 4) {
                for (int x = 0; x < SOLVING_APPROACHES_AMOUNT; x++) {
                    config.methodIndex = x;
                    //                        printConfigs();
                    executeMethod(file);
                }
            } else {
                executeMethod(file);
            }
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

int getMethodIndex() { //index 0 = NN
    int index = 0; //NN
    if (strcmp(config.method, "DSNN") == 0) {
        index = 1;
    } else if (strcmp(config.method, "RNN") == 0) {
        index = 2;
    } else if (strcmp(config.method, "RDSNN") == 0) {
        index = 3;
    } else if (strcmp(config.method, "all") == 0) {
        index = 4;
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
    printf("Usage: \n argument1: method (Accepted values: NN, DSNN, RNN, RDSNN, all) \n"
            "argument2: times to repeat \n"
            "argument3: mode. Accepted values: folder or file \n "
            "argument4: path \n"
            "[argument 5: alpha value *optional*] \n "
            "[argument 6: alpha value upper bound *optional* (if provided, argument 5 will be treated as the lower bound and all values will be tested]");

    printf("\n Note that if you choose a method that is random-based (RNN, RDSNN, all),"
            " you MUST provide the alpha value.");
    printf("\n Example: ./tspProblem DSNN 10 folder instances");
    printf("\n Example 2: ./tspProblem RDSNN 5 file instances/a280.tsp 20");
    printf("\n Example 3: ./tspProblem all 5 file instances/a280.tsp 10 \n");
}

void printConfigs() {
    printf("\nmethod: %s", config.method);
    printf("\nmode: %s", config.mode);
    printf("\npath: %s", config.path);
    printf("\nmethodIndex: %d", config.methodIndex);
    printf("\nrepeatTimes: %d", config.repeatTimes);
    printf("\nalpha: %d", config.alpha);
    printf("\nalphaUB: %d", config.alphaUB);
    printf("\ntestAlpha: %d", config.testAlpha);
}

int main(int argc, char** argv) {
    //        printArguments(argc, argv);
    if (argc >= 5) {
        config.method = argv[1];
        config.methodIndex = getMethodIndex();
        config.repeatTimes = atoi(argv[2]);
        config.mode = argv[3];
        config.path = argv[4];
        if (argc >= 6) {
            config.alpha = atoi(argv[5]);
            if (argc >= 7) {
                config.alphaUB = atoi(argv[6]);
                config.testAlpha = 1;
            }
        } else {
            config.alpha = 0;
        }

        originalAlpha = config.alpha;
        originalUBAlpha = config.alpha;
        originalMethodIndex = config.methodIndex;

        //        printf("\n alpha = %d  alphaUB = %d \n", alpha, alphaUB);
        printf("<instances>");
        invokeExecution();
        printf("\n</instances>\n");
    } else {
        printHelp();
    }
    return (EXIT_SUCCESS);
}