#pragma once

#include <webgpu/webgpu.h>
#include <vector>
#include <memory>
#include "WebGPUContext.h"

class Life
{
public:
    Life();
    void tick();

private:
    // WebGPU Context
    std::unique_ptr<WebGPUContext> context;
    
    // Vertex buffer and layout
    WGPUBuffer vertexBuffer = nullptr;
    WGPUVertexBufferLayout vertexBufferLayout = {};
    WGPUVertexAttribute vertexAttribute = {};

    // Uniform Buffer & Bindgroup
    WGPUBuffer uniformBuffer = nullptr;
    WGPUBindGroup uniformBindGroup = nullptr;

    // Cell State Storage Buffer
    WGPUBuffer cellStateStorageBufferA = nullptr;
    WGPUBuffer cellStateStorageBufferB = nullptr;
    std::vector<uint32_t> cellStateArray;
    
    // Shader module and render pipeline
    WGPUShaderModule cellShaderModule = nullptr;
    WGPURenderPipeline cellPipeline = nullptr;

    // Bind group
    WGPUBindGroupLayout bindGroupLayout = nullptr;
    WGPUBindGroup bindGroups[2] = {nullptr, nullptr};
    
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
    static constexpr uint32_t GRID_SIZE = 32;
    static constexpr uint32_t INSTANCE_COUNT = GRID_SIZE * GRID_SIZE;
    static constexpr float UNIFORM_ARRAY[2] = {
        static_cast<float>(GRID_SIZE), 
        static_cast<float>(GRID_SIZE)
    };

    // Frame count
    uint32_t frameCount = 0;

    // Internal methods
    void createShaderModule();
    void createVertexBuffer();
    void createUniformBuffer();
    void setupVertexLayout();
    void createRenderPipeline();
    void createCellStateStorageBuffer();
    void createBindGroupLayout();
    void createBindGroup();
};