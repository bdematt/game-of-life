#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <webgpu/webgpu.h>
#include "webgpu-utils.h"
#include "life.h"

// Globals
Life* life;

void main_loop() {
    life->tick();
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

    // Create and configure Surface
    WGPUSurface surface = createSurface(instance);
    if (!surface) {
        std::cout << "❌ Surface creation failed!" << std::endl;
        return -1;
    }
    configureSurface(device, surface);

    // Get queue
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    std::cout << "✅ WebGPU queue obtained!" << std::endl;
    std::cout << "🎉 WebGPU initialization complete!" << std::endl;

    

    life = new Life(
        instance,
        adapter,
        device,
        surface,
        queue
    );

    emscripten_set_main_loop(main_loop, 0, 1);
    
    return 0;
}