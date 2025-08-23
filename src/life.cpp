#include "life.h"
#include <iostream>
#include <GLFW/glfw3.h>

Life::Life(
    WGPUInstance instance,
    WGPUAdapter adapter,
    WGPUDevice device,
    WGPUQueue queue,
    GLFWwindow* window
)
{
    std::cout << "🔧 Life created" << std::endl;
}

Life::~Life()
{
    // Clean up WebGPU resources
    if (surface)
        wgpuSurfaceRelease(surface);
    if (queue)
        wgpuQueueRelease(queue);
    if (device)
        wgpuDeviceRelease(device);
    if (adapter)
        wgpuAdapterRelease(adapter);
    if (instance)
        wgpuInstanceRelease(instance);

    std::cout << "🔧 Life destroyed" << std::endl;
}

void Life::tick()
{
    // Tick...
    std::cout << "🔧 Tick..." << std::endl;
}

GLFWwindow* Life::getWindow()
{
    return window;
}