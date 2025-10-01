#pragma once
#include <cstdint>
#include "webgpu.hpp"

class Life
{
private:
    // WGPU Context
    wgpu::Instance instance {};
    wgpu::Adapter adapter{nullptr};
    wgpu::Device device{nullptr};
    wgpu::Queue queue{nullptr};
    wgpu::Surface surface{nullptr};
    wgpu::SurfaceConfiguration surfaceConfig{};
    wgpu::RenderPipeline renderPipeline{nullptr};
    wgpu::Buffer vertexBuffer{nullptr};
    wgpu::Buffer uniformBuffer{nullptr};
    wgpu::Buffer storageBuffer{nullptr};
    wgpu::BindGroupLayout bindGroupLayout{nullptr};
    wgpu::BindGroup bindGroup{nullptr};

    // Geometry
    static constexpr float VERTICES[] = {
        -0.8f, -0.8f,
         0.8f, -0.8f,
         0.8f,  0.8f,
        
        -0.8f, -0.8f,
         0.8f,  0.8f,
        -0.8f,  0.8f,
    };
    static constexpr int GRID_SIZE = 32;
    static constexpr float UNIFORM_ARRAY[2] = {
        static_cast<float>(GRID_SIZE), 
        static_cast<float>(GRID_SIZE)
    };

    // Cell State
    std::vector<uint32_t> cellStateArray;
    
    void requestAdapter();
    void requestDevice();
    void createSurface();
    void configureSurface();
    void createRenderPipeline();
    void createVertexBuffer();
    void createUniformBuffer();
    void createStorageBuffers();
    void createBindGroupLayout();
    void createBindGroup();
    void cleanup();

public:
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
    ~Life();

    const wgpu::Instance& getInstance() const { return instance; }
    const wgpu::Adapter& getAdapter() const { return adapter; }
    const wgpu::Device& getDevice() const { return device; }
    const wgpu::Queue& getQueue() const { return queue; }
    const wgpu::Surface& getSurface() const { return surface; }
    const wgpu::SurfaceConfiguration& getSurfaceConfig() const { return surfaceConfig; }
    const wgpu::RenderPipeline& getRenderPipeline() const { return renderPipeline; }
    const wgpu::Buffer& getVertexBuffer() const { return vertexBuffer; }
    const wgpu::Buffer& getUniformBuffer() const { return uniformBuffer; }
    const wgpu::BindGroupLayout& getBindGroupLayout() const { return bindGroupLayout; }
    const wgpu::BindGroup& getBindGroup() const { return bindGroup; }
    void renderFrame();

};


