#pragma once
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>

class Life
{
public:
    Life(
        WGPUInstance instance,
        WGPUAdapter adapter,
        WGPUDevice device,
        WGPUQueue queue,
        GLFWwindow* window
    );
    ~Life();
    void tick();
    GLFWwindow* getWindow();

private:
    // WebGPU objects
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;

    // GLFW window reference
    GLFWwindow *window;

};