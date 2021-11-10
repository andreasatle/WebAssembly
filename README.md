# WebAssembly using C

On mac, we can install *emscripten* by
```brew install emscripten```.

To compile a file ```lib/canvas.c``` to wasm, we can write
```emcc -Oz lib/canvas.c -s WASM=1 -s EXPORTED_FUNCTIONS="['_getColor', '_getPosition', '_getRadius', '_getNumCircles', '_initCircles', '_updateCircles']" -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" -o public/canvas.js```.

We need to include ```-s WASM=1``` to compile to WebAssembly. By default, only the main-function is accessible. We need to add ```-s EXPORTED_FUNCTIONS="['_getColor', '_getPosition', '_getRadius', '_getNumCircles', '_initCircles', '_updateCircles']"``` in order to access other functions than main. Observe that ```main``` has to be included in the list, if this flag is used. Also, all functions start with an ```_```.

We can choose optimization level by the flag ```-On```, where ```n``` can be 0, 1, 2, 3, s or z. When optimization is used, then it is necessary to specify ```-s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']"``` to use ```cwrap``` and ```ccall``. They are simply removed due to space optimization.

## Trivial webserver using Express
We can write a trivial server in node.js/express in the project root directory:
```
// Initialize Express
const express = require('express')
const app = express()

// Serve static files from /public
app.use(express.static('public'))

// Start server
app.listen(2222, ()=>console.log('Server running on port 2222!'))
```

## HTML-code

We add an html file in the *public/index.html*:
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>WebAssembly Demo</title>
    <style media="screen">
        canvas {
            width: 80%;
            height: 80%;
            margin: 0%;
            border-style: solid;
        }
    </style>
</head>
<body>
    <h1>WebAssembly Demo</h1>
    <canvas id="canvasID" width=2000 height=1000></canvas>
    <script src="canvas.js"></script>
    <script src="bouncing_spheres.js"></script>
</body>
</html>
```

## Javascript-codes
The first javascript-file *canvas.js* is created by the compilation.
The second javascript-file *bouncing_spheres.js* plots the spheres
in a canvas.
```
const canvas = document.getElementById('canvasID')
const context = canvas.getContext('2d')

// Wait for WASM (Emscripten) runtime to be ready
Module['onRuntimeInitialized'] = () => {
    // Initialize circles: radius, color, position
    _initCircles(canvas.width, canvas.height)

    // Get the number of circles
    let numCircles = _getNumCircles()

    // Setup the arrays
    let position = new Int16Array( Module.HEAP16.buffer, _getPosition(), 2*numCircles)
    let color = new Uint8Array( Module.HEAPU8.buffer, _getColor(), 3*numCircles)
    let radius = new Uint16Array( Module.HEAP16.buffer, _getRadius(), numCircles)

    // Function Definition: Update circles and print canvas
    const render = () => {
        // Update the position and velocity in the c-code.
        _updateCircles()

        // Clear the canvas
        context.clearRect(0, 0, canvas.width, canvas.height)

        // Draw the circles
        for (let i = 0; i < numCircles; ++i) {
            context.beginPath();
            context.arc(position[2*i], position[2*i+1], radius[i], 0, 2*Math.PI, false)
            context.fillStyle = `rgba(${color[3*i]},${color[3*i+1]},${color[3*i+2]},0.75)`
            context.fill()
        }

        // Re-render with frequency set by the environment.
        window.requestAnimationFrame(() => {
            render()
        })
    }

    // Call infinite render-function
    render()
}
```

## C code

The C-code is provided in *lib/canvas.c* initializes and updates the spheres in between the rendering in the javascript file.
```
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten.h>

// Set number of circles in simulation.
#define NUM_CIRCLES 1000

// Struct containing 2D coordinates.
struct Coordinate2D {
    short x;
    short y;
};

// Struct containing RGB values.
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

// Give access to position array.
struct Coordinate2D * getPosition() {
    return position;
}

// Give access to color array.
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
```