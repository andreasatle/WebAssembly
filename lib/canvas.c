#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten.h>

// Set number of circles in simulation.
#define NUM_CIRCLES 1000

// Container for 2D coordinates.
struct Coordinate2D {
    short x;
    short y;
};

// Container for RGB values.
struct ColorRGB {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// Global arrays.
struct ColorRGB color[NUM_CIRCLES];
struct Coordinate2D position[NUM_CIRCLES];
struct Coordinate2D velocity[NUM_CIRCLES];
short radius[NUM_CIRCLES];

// Canvas dimensions from javascript.
// These are initialized in initCircles.
short canvasWidth;
short canvasHeight;

// Get a random value 0 - max-1.
int getRand(int max) {
    return rand()%max;
}

// Random initialization of circle attributes.
void initCircles(int width, int height) {
    // Initialize random seed.
    srand(time(NULL));

    // Initialize the size of the canvas from javascript.
    canvasWidth = (short)width;
    canvasHeight = (short)height;

    // Loop over circles.
    for (int i = 0; i < NUM_CIRCLES; ++i) {
        radius[i] = (short)(10 + getRand(40));
        color[i].red = (unsigned char)getRand(256);
        color[i].green = (unsigned char)getRand(256);
        color[i].blue = (unsigned char)getRand(256);
        position[i].x = radius[i] + (short)getRand(canvasWidth-2*radius[i]);
        position[i].y = radius[i] + (short)getRand(canvasHeight-2*radius[i]);
        velocity[i].x = (short)(-10 + getRand(20));
        velocity[i].y = (short)(-10 + getRand(20));
    }
}

// Update circle positions and velocities.
void updateCircles() {

    // Loop over circles.
    for (int i = 0; i < NUM_CIRCLES; ++i) {

        // Update position.
        position[i].x += velocity[i].x;
        position[i].y += velocity[i].y;

        // Check left boundary.
        if (position[i].x < radius[i]) {
            position[i].x = 2*radius[i]-position[i].x;
            velocity[i].x = -velocity[i].x;
        }

        // Check right boundary.
        if (position[i].x >= canvasWidth-radius[i]) {
            position[i].x = 2*(canvasWidth-radius[i])-position[i].x;
            velocity[i].x = -velocity[i].x;
        }

        // Check top boundary.
        if (position[i].y < radius[i]) {
            position[i].y = 2*radius[i]-position[i].y;
            velocity[i].y = -velocity[i].y;
        }

        // Check bottom boundary.
        if (position[i].y >= canvasHeight-radius[i]) {
            position[i].y = 2*(canvasHeight-radius[i])-position[i].y;
            velocity[i].y = -velocity[i].y;
        }
    }
}

// Give acces to position array.
struct Coordinate2D * getPosition() {
    return position;
}

// Give acces to color array.
struct ColorRGB * getColor() {
    return color;
}

// Give access to radius array.
short * getRadius() {
    return radius;
}

// Return number of circles.
int getNumCircles() {
    return NUM_CIRCLES;
}