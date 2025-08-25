#pragma once

#include <webgpu/webgpu.h>
#include <vector>

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
    
    // Vertex buffer
    WGPUBuffer vertexBuffer = nullptr;
    
    // Surface state
    bool surfaceCreated = false;
    int width = 800;   // Canvas dimensions
    int height = 600;

    // Vertex data for a square (two triangles)
    // In Normalized Device Coordinates (-1 to 1)
    static constexpr float vertices[] = {
        // X,    Y,
        -0.8f, -0.8f, // Triangle 1 (Blue)
         0.8f, -0.8f,
         0.8f,  0.8f,

        -0.8f, -0.8f, // Triangle 2 (Red) 
         0.8f,  0.8f,
        -0.8f,  0.8f,
    };
    
    static constexpr uint32_t vertexCount = 6; // 6 vertices (2 triangles * 3 vertices each)

    // Internal methods
    void createVertexBuffer();
};