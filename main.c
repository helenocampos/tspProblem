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
/*
 * 
 */

#define BIG_DISTANCE 999999
#define INSTANCES_FOLDER "instances/"
#define SOLVING_APPROACHES_AMOUNT 2

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
    printf("\nTotal length: %f, total cities: %d", length, routeSize);
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
    //    printf("\n min distance = %f  from %d to %d", minDistance, origin, minDistanceVertex);
    return minDistanceVertex;
}

void getNNearestVertexNotVisited(double distances[], int vertexAmount, int origin, int visitedVertexes[], int N) {
    int NNearestNotVisited[N];
    double minDistance = BIG_DISTANCE;
    int minDistanceVertex = -1;
    for (int j = 0; j < vertexAmount; j++) {
        //        printf("\n %d to %d = %f. visited: %d", origin, j, distances[j], visitedVertexes[j]);
        if (distances[j] < minDistance && visitedVertexes[j] == 0 && origin != j) {
            minDistance = distances[j];
            minDistanceVertex = j;
        }
    }
    //    printf("\n min distance = %f  from %d to %d", minDistance, origin, minDistanceVertex);
//    return NNearestNotVisited;
}

void initializeArray(int array[], int size, int value) {
    for (int i = 0; i < size; i++) {
        array[i] = value;
    }
}

double symmetricGreedyTSP(int startingNode, struct TSPInstance *instance) { //using a greedy approach 
    double totalDistance = 0;
    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);

        int routeOrder[instance->citiesAmount + 1];
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int lastVisited = startingNode;
        for (int visitNumber = 0; visitNumber < instance->citiesAmount; visitNumber++) {
            //            //find the edge with the minimum distance among the not visited vertexes
            int nearestNotVisited = getNearestVertexNotVisited(instance->graphMatrix[visitNumber],
                    instance->citiesAmount, visitNumber, visitedVertexes);
            if (nearestNotVisited != -1) {
                visitedVertexes[nearestNotVisited] = 1;
                totalDistance += instance->graphMatrix[lastVisited][nearestNotVisited];
                routeOrder[visitNumber + 1] = nearestNotVisited;
                //            printf("\n nearest: %d visitnumber: %d", nearestNotVisited, visitNumber);

                lastVisited = nearestNotVisited;
            }
        }
        //        printRoute(routeOrder, instance->citiesAmount, totalDistance);
    }
    return totalDistance;
}

double randomSymmetricGreedyTSP(int startingNode, struct TSPInstance *instance, float alpha) { //using a random greedy approach 
    double totalDistance = 0;
    if (startingNode < instance->citiesAmount) {
        int visitedVertexes[instance->citiesAmount]; //if value is 1 then the vertex was already visited
        initializeArray(visitedVertexes, instance->citiesAmount, 0);

        int routeOrder[instance->citiesAmount + 1];
        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;
        int lastVisited = startingNode;
        for (int visitNumber = 0; visitNumber < instance->citiesAmount; visitNumber++) {
            //            //find the edge with the minimum distance among the not visited vertexes
            int nearestNotVisited = getNearestVertexNotVisited(instance->graphMatrix[visitNumber],
                    instance->citiesAmount, visitNumber, visitedVertexes);
            if (nearestNotVisited != -1) {
                visitedVertexes[nearestNotVisited] = 1;
                totalDistance += instance->graphMatrix[lastVisited][nearestNotVisited];
                routeOrder[visitNumber + 1] = nearestNotVisited;
                //            printf("\n nearest: %d visitnumber: %d", nearestNotVisited, visitNumber);

                lastVisited = nearestNotVisited;
            }
        }
        //        printRoute(routeOrder, instance->citiesAmount, totalDistance);
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

        routeOrder[0] = startingNode;
        visitedVertexes[startingNode] = 1;

        for (int visitNumber = 0; visitNumber < instance->citiesAmount; visitNumber++) {
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
                    visitedVertexes[nearestNotVisitedHead1] = 1;
                    totalDistance += nearestNotVisitedHead1Distance;
                    routeOrder[visitNumber + 1] = nearestNotVisitedHead1;
                    head1 = nearestNotVisitedHead1;
                    //                    printf("\n Visitei Head1");
                } else {
                    visitedVertexes[nearestNotVisitedHead2] = 1;
                    totalDistance += nearestNotVisitedHead2Distance;
                    routeOrder[visitNumber + 1] = nearestNotVisitedHead2;
                    head2 = nearestNotVisitedHead2;
                    //                    printf("\n Visitei Head2");
                }
            }
        }
        //        printRoute(routeOrder, instance->citiesAmount, totalDistance);
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

void executeMethod(char* file, int methodIndex) {
    //    printf("\nExecuting TSP for %s instance \n", file);
    struct TSPLibData *tspLibData = 0;
    struct TSPInstance *tspInstance = 0;
    clock_t start = 0, end = 0;
    double calculationTime = 0, readTime = 0, allocationTime = 0;
    double distance = 0;
    start = clock();
    tspLibData = parseTSPLibFileEuclidian2D(file);
    end = clock();
    if (tspLibData) {
        readTime = ((double) (end - start)) / CLOCKS_PER_SEC;
        start = clock();
        tspInstance = allocateTSPInstanceEuclidian2D(tspLibData);
        end = clock();
        allocationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
        if (tspInstance) {
            start = clock();
            switch (methodIndex) {
                default: distance = symmetricGreedyTSP(1, tspInstance);
                    break;
                case 1: distance = doubleSidedSymmetricGreedyTSP(1, tspInstance);
                    break;
            }
            end = clock();
            calculationTime = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("<instance>\n");
            printf("\t<name>%s</name>\n", file);
            printf("\t<method>%s</method>\n", getMethodName(methodIndex));
            printf("\t<distance>%.2f</distance>\n", distance);
            printf("\t<calcTime>%f</calcTime>\n", calculationTime);
            printf("\t<allocTime>%f</allocTime>\n", allocationTime);
            printf("\t<readTime>%f</readTime>\n", readTime);
            printf("</instance>\n");
            //        printf("instance: %s \n \t distancia: %.2f tempos leitura: %f \t "
            //                "alocação: %f \t calculo %f  \n\n\n", file, distance, readTime, allocationTime, calculationTime);
        }
    }

    freeInstancesMemory(tspLibData, tspInstance);
}

void executeMethodDir(char* path, int methodIndex) {
    struct dirent *de; // Pointer for directory entry 
    DIR *dr = opendir(path);
    if (dr == NULL) {
        printf("Could not open current directory");
    }

    while ((de = readdir(dr)) != NULL)
        if (strcmp(de->d_name, ".") != 0
                && strcmp(de->d_name, "..") != 0
                && strcmp(de->d_name, ".DS_Store") != 0) {
            char *filePath = concat(concat(path, "/"), de->d_name);
            executeMethod(filePath, methodIndex);
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

int getMethodIndex(char* method) { //index 0 = NN
    int index = 0; //NN
    if (strcmp(method, "DSNN") == 0) {
        index = 1;
    } else if (strcmp(method, "RNN") == 0) {
        index = 2;
    } else if (strcmp(method, "RDSNN") == 0) {
        index = 3;
    } else if (strcmp(method, "all") == 0) {
        index = 4;
    }
    return index;
}

void invokeExecution(char* path, char*mode, int methodIndex) {
    if (strcmp(mode, "folder") == 0) {
        executeMethodDir(path, methodIndex);
    } else if (strcmp(mode, "file") == 0) {

        executeMethod(path, methodIndex);
    }
}

int a(int argc, char** argv) {
    //    printArguments(argc, argv);
    if (argc == 5) {
        char* method = argv[1];
        int methodIndex = getMethodIndex(method);
        int repeatTimes = atoi(argv[2]);
        char* mode = argv[3];
        char* path = argv[4];
        for (int i = 0; i < repeatTimes; i++) {
            if (methodIndex == 4) {
                for (int x = 0; x < SOLVING_APPROACHES_AMOUNT; x++) {
                    invokeExecution(path, mode, x);
                }
            } else {
                invokeExecution(path, mode, methodIndex);
            }

        }
    } else {
        printf("Usage: \n argument1: method (Accepted values: NN, DSNN, RNN, RDSNN, all) argument2: times to repeat argument3: mode. Accepted values: folder or file \n argument4: path");
        printf("\n Example: ./tspProblem DSNN 10 folder instances");
        printf("\n Example 2: ./tspProblem RDSNN 5 file instances/a280.tsp");
        printf("\n Example 3: ./tspProblem all 5 file instances/a280.tsp");
    }
    return (EXIT_SUCCESS);
}


void printNd(struct NeighborDistance *nd, int size){
    for(int i=0; i<size; i++){
        printf("\n%f", nd[i].distanceFromOrigin);
    }
}

int main(int argc, char** argv) {
    struct NeighborDistance *nd = 0;
    nd = malloc(5 * sizeof (*nd));
    nd[0].distanceFromOrigin=5;
    nd[1].distanceFromOrigin=4;
    nd[2].distanceFromOrigin=3;
    nd[3].distanceFromOrigin=2;
    nd[4].distanceFromOrigin=1;
    
    printNd(nd, 5);
    sortNeighborsByDistance(nd, 5);
    printf("\n");
    printNd(nd, 5);
    return (EXIT_SUCCESS);
}

