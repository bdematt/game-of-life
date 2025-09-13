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
    
    // Vertex buffer and layout
    WGPUBuffer vertexBuffer = nullptr;
    WGPUVertexBufferLayout vertexBufferLayout = {};
    WGPUVertexAttribute vertexAttribute = {};

    // Uniform Buffer & Bindgroup
    WGPUBuffer uniformBuffer = nullptr;
    WGPUBindGroup uniformBindGroup = nullptr;
    
    // Shader module and render pipeline
    WGPUShaderModule cellShaderModule = nullptr;
    WGPURenderPipeline cellPipeline = nullptr;
    
    // Surface state
    bool surfaceCreated = false;
    int width = 800;   // Canvas dimensions
    int height = 600;

    // Vertex data for a square (two triangles)
    // In Normalized Device Coordinates (-1 to 1)
    static constexpr float VERTICES[] = {
        // X,    Y,
        -0.8f, -0.8f, // Triangle 1 (Blue)
         0.8f, -0.8f,
         0.8f,  0.8f,

        -0.8f, -0.8f, // Triangle 2 (Red) 
         0.8f,  0.8f,
        -0.8f,  0.8f,
    };
    
    static constexpr uint32_t VERTEX_COUNT = 6; // 6 vertices (2 triangles * 3 vertices each)
    static constexpr uint32_t GRID_SIZE = 4;
    static constexpr float UNIFORM_ARRAY[2] = {
        static_cast<float>(GRID_SIZE), 
        static_cast<float>(GRID_SIZE)
    };

    // Internal methods
    void createShaderModule();
    void createVertexBuffer();
    void createUniformBuffer();
    void setupVertexLayout();
    void createRenderPipeline();
    void createBindGroup();
};