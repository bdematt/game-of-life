#pragma once

#include <webgpu/webgpu.h>

class Life
{
public:
    Life(WGPUInstance instance, WGPUAdapter adapter, WGPUDevice device, WGPUSurface surface, WGPUQueue queue);
    ~Life();

    void tick();

private:
    // WebGPU objects
    WGPUInstance instance = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUSurface surface = nullptr;
    
    // Surface state
    bool surfaceCreated = false;
    int width = 800;   // Canvas dimensions
    int height = 600;

    // Internal methods
    void createSurface();
    void configureSurface();
};