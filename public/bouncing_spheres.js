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

