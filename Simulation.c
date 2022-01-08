/*
    Simulation.c
    Contains utility functions
*/
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

float generateRandom();

// Server Packet Loss
int SimulateLoss(float pktLossRatio) {
    return generateRandom() < pktLossRatio ? 1 : 0;
}

// Client ACK Loss
int SimulateACKLoss(float ACKLossRatio) {
    return generateRandom() < ACKLossRatio ? 1 : 0;
}

// Generate random float between 0 and 1
float generateRandom() {
    return ((float)rand())/RAND_MAX;
}

// Alternate n between 1 and 0
short alternateNum(short n) {
    return n == 0 ? 1 : 0;
}