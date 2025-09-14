#pragma once

#include <webgpu/webgpu.h>
#include <vector>
#include <memory>
#include "WebGPUContext.h"
#include "Geometry.h"
#include "RenderPipeline.h"

class Life
{
public:
    static constexpr uint32_t GRID_SIZE = 32;

    class InitializationError : public std::runtime_error {
        public:
            InitializationError(const std::string& msg) 
                : std::runtime_error("Initialization failed: " + msg) {}
    };
    class RuntimeError : public std::runtime_error {
        public:
            RuntimeError(const std::string& msg) 
                : std::runtime_error("Encountered an unexpected runtime error: " + msg) {}
    };
    Life();
    void tick();

private:
    // WebGPU Context
    std::unique_ptr<WebGPUContext> context;

    // Component Classes
    std::unique_ptr<Geometry> geometry; 
    std::unique_ptr<RenderPipeline> pipeline; 
    
    // Uniform Buffer & Bindgroup
    WGPUBuffer uniformBuffer = nullptr;
    WGPUBindGroup uniformBindGroup = nullptr;

    // Cell State Storage Buffer
    WGPUBuffer cellStateStorageBufferA = nullptr;
    WGPUBuffer cellStateStorageBufferB = nullptr;
    std::vector<uint32_t> cellStateArray;
    
    // Shader module and render pipeline
    WGPURenderPipeline cellPipeline = nullptr;

    // Bind group
    WGPUBindGroupLayout bindGroupLayout = nullptr;
    WGPUBindGroup bindGroups[2] = {nullptr, nullptr};
    
    // Surface state
    bool surfaceCreated = false;
    int width = 800;   // Canvas dimensions
    int height = 600;
    
    static constexpr uint32_t INSTANCE_COUNT = GRID_SIZE * GRID_SIZE;
    static constexpr float UNIFORM_ARRAY[2] = {
        static_cast<float>(GRID_SIZE), 
        static_cast<float>(GRID_SIZE)
    };

    // Frame count
    uint32_t frameCount = 0;

    // Internal methods
    void createUniformBuffer();
    void createRenderPipeline();
    void createCellStateStorageBuffer();
    void createBindGroupLayout();
    void createBindGroup();
};