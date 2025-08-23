#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include "webgpu-utils.h"
#include "life.h"

// Globals
Life* life;

void main_loop() {
    if (glfwWindowShouldClose(life->getWindow())) {
        emscripten_cancel_main_loop();
        return;
    }
    life->tick();
    glfwPollEvents();
    glfwSwapBuffers(life->getWindow());
}

int main() {
    std::cout << "🚀 Starting WebGPU application..." << std::endl;
    
    // Create WebGPU instance
    WGPUInstanceDescriptor instanceDesc = {};
    WGPUInstance instance = wgpuCreateInstance(&instanceDesc);
    if (!instance)
    {
        std::cout << "❌ Failed to create WebGPU instance" << std::endl;
        return -1;
    }
    std::cout << "✅ WebGPU instance created!" << std::endl;

    // Request adapter
    WGPURequestAdapterOptions options = {};
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    WGPUAdapter adapter = requestAdapterSync(instance, &options);
    if (!adapter)
    {
        std::cout << "❌ Failed to get WebGPU adapter" << std::endl;
        return -1;
    }
    std::cout << "✅ WebGPU adapter acquired!" << std::endl;

    // Request device
    WGPUDeviceDescriptor deviceDesc = {};
    WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
    if (!device) {
        std::cout << "❌ Failed to get WebGPU device" << std::endl;
        return -1;
    }
    std::cout << "✅ WebGPU device acquired!" << std::endl;
    
    // Get queue
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    std::cout << "✅ WebGPU queue obtained!" << std::endl;
    std::cout << "🎉 WebGPU initialization complete!" << std::endl;

    // Initialize GLFW & Window
    glfwInit();
        if (!glfwInit()) {
        std::cout << "❌ Could not initialize GLFW" << std::endl;
        return 1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "Game of Life", NULL, NULL);
    if (!window) {
        std::cout << "❌ Could not open window" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    std::cout << "✅ GLFW window opened!" << std::endl;

    life = new Life(
        instance,
        adapter,
        device,
        queue,
        window
    );

    emscripten_set_main_loop(main_loop, 0, 1);
    
    return 0;
}