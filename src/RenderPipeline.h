#pragma once
#include <webgpu/webgpu.h>
#include <vector>
#include <memory>
#include "WebGPUContext.h"
#include "Geometry.h"

class RenderPipeline
{
public:
    RenderPipeline(const WebGPUContext& context, const Geometry& geometry);
    ~RenderPipeline();
    void renderFrame(const WebGPUContext &context, const Geometry &geometry);

    // Getters
    WGPUShaderModule getCellShaderModule() const { return cellShaderModule; }

private:
    static constexpr const char* SHADER_CODE = R"(
        struct VertexInput {
            @location(0) pos: vec2f,
            @builtin(instance_index) instance: u32,
        };

        struct VertexOutput {
            @builtin(position) pos: vec4f,
            @location(0) cell: vec2f
        };

        @group(0) @binding(0) var<uniform> grid: vec2f;
        @group(0) @binding(1) var<storage> cellState: array<u32>;

        @vertex
        fn vertexMain(input: VertexInput) -> VertexOutput {

            let i = f32(input.instance);
            let cell = vec2f(i % grid.x, floor(i / grid.x)); // Grid cell X,Y (between 0 and GRID_SIZE-1)
            let state = f32(cellState[input.instance]);
            let cellOffset = cell / grid * 2;
            let gridPos = (input.pos * state + 1) / grid - 1 + cellOffset;

            var output: VertexOutput;
            output.pos = vec4f(gridPos, 0, 1);
            output.cell = cell;
            return output;
        }

        struct FragInput {
            @location(0) cell: vec2f,
        };
        
        @fragment
        fn fragmentMain(input: FragInput) -> @location(0) vec4f {
            let c = input.cell / grid;
            return vec4f(c, 1-c.y, 1);
        }
    )";
    WGPUShaderModule cellShaderModule = nullptr;

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
    
    static constexpr uint32_t GRID_SIZE = 32;
    static constexpr uint32_t INSTANCE_COUNT = GRID_SIZE * GRID_SIZE;
    static constexpr float UNIFORM_ARRAY[2] = {
        static_cast<float>(GRID_SIZE), 
        static_cast<float>(GRID_SIZE)
    };

    // Frame count
    uint32_t frameCount = 0;

    // Internal methods
    void createCellShaderModule(const WebGPUContext& context);
    void createUniformBuffer(const WebGPUContext& context);
    void createRenderPipeline(const WebGPUContext& context, const Geometry& geometry);
    void createCellStateStorageBuffer(const WebGPUContext& context);
    void createBindGroupLayout(const WebGPUContext& context);
    void createBindGroup(const WebGPUContext& context);
    void cleanup();

};