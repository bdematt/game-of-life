#include "ComputePipeline.h"
#include "Life.h"
#include <iostream>

ComputePipeline::ComputePipeline(const WebGPUContext& context)
{
    std::cout << "🔧 Creating ComputePipeline..." << std::endl;
    
    try {
        createSimulationShaderModule(context);
        createBindGroupLayout(context);
        createComputePipeline(context);
    } catch(...) {
        cleanup();
        throw;
    }
}

ComputePipeline::~ComputePipeline()
{
    cleanup();
}

void ComputePipeline::createSimulationShaderModule(const WebGPUContext& context)
{
    WGPUShaderSourceWGSL source = {};
    source.chain.sType = WGPUSType_ShaderSourceWGSL;
    source.code = WGPUStringView{SIMULATION_SHADER_CODE, strlen(SIMULATION_SHADER_CODE)};
    
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.label = WGPUStringView{"Game of Life simulation shader", 32};
    shaderModuleDesc.nextInChain = &source.chain;

    simulationShaderModule = wgpuDeviceCreateShaderModule(context.getDevice(), &shaderModuleDesc);
    if (!simulationShaderModule) {
        throw Life::InitializationError("Failed to create simulation shader module");
    }
}

void ComputePipeline::createBindGroupLayout(const WebGPUContext& context)
{
    // Define binding layout entries for compute shader
    WGPUBindGroupLayoutEntry bindingLayouts[3];
    
    // Uniform buffer binding (binding = 0)
    bindingLayouts[0] = {};
    bindingLayouts[0].binding = 0;
    bindingLayouts[0].visibility = WGPUShaderStage_Compute;
    bindingLayouts[0].buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayouts[0].buffer.hasDynamicOffset = false;
    bindingLayouts[0].buffer.minBindingSize = 0;
    
    // Input storage buffer binding (binding = 1)  
    bindingLayouts[1] = {};
    bindingLayouts[1].binding = 1;
    bindingLayouts[1].visibility = WGPUShaderStage_Compute;
    bindingLayouts[1].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    bindingLayouts[1].buffer.hasDynamicOffset = false;
    bindingLayouts[1].buffer.minBindingSize = 0;
    
    // Output storage buffer binding (binding = 2)
    bindingLayouts[2] = {};
    bindingLayouts[2].binding = 2;
    bindingLayouts[2].visibility = WGPUShaderStage_Compute;
    bindingLayouts[2].buffer.type = WGPUBufferBindingType_Storage;
    bindingLayouts[2].buffer.hasDynamicOffset = false;
    bindingLayouts[2].buffer.minBindingSize = 0;
    
    // Create bind group layout descriptor
    WGPUBindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Compute Bind Group Layout", 26};
    layoutDesc.entryCount = 3;
    layoutDesc.entries = bindingLayouts;
    
    // Create the bind group layout
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(context.getDevice(), &layoutDesc);
    
    if (!bindGroupLayout) {
        throw Life::InitializationError("Failed to create compute bind group layout");
    }
}

void ComputePipeline::createComputePipeline(const WebGPUContext& context)
{
    // Create pipeline layout
    WGPUBindGroupLayout layouts[] = { bindGroupLayout };
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Compute Pipeline Layout", 24};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = layouts;
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(context.getDevice(), &layoutDesc);
    
    // Create compute pipeline descriptor
    WGPUComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.label = WGPUStringView{"Simulation Pipeline", 19};
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.compute.module = simulationShaderModule;
    pipelineDesc.compute.entryPoint = WGPUStringView{"computeMain", 11};
    
    // Create the compute pipeline
    simulationPipeline = wgpuDeviceCreateComputePipeline(context.getDevice(), &pipelineDesc);
    
    // Clean up intermediate layout
    wgpuPipelineLayoutRelease(pipelineLayout);
    
    if (!simulationPipeline) {
        throw Life::InitializationError("Failed to create simulation compute pipeline");
    }
}

void ComputePipeline::runSimulation(const WebGPUContext& context, WGPUBindGroup bindGroup)
{
    // Create command encoder
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = WGPUStringView{"Simulation Command Encoder", 27};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(context.getDevice(), &encoderDesc);

    // Begin compute pass
    WGPUComputePassDescriptor computePassDesc = {};
    computePassDesc.label = WGPUStringView{"Game of Life simulation", 24};
    WGPUComputePassEncoder computePass = wgpuCommandEncoderBeginComputePass(encoder, &computePassDesc);
    
    // Set pipeline and bind group
    wgpuComputePassEncoderSetPipeline(computePass, simulationPipeline);
    wgpuComputePassEncoderSetBindGroup(computePass, 0, bindGroup, 0, nullptr);
    
    // Dispatch workgroups
    const uint32_t workgroupCount = (GRID_SIZE + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE; // Ceiling division
    wgpuComputePassEncoderDispatchWorkgroups(computePass, workgroupCount, workgroupCount, 1);
    
    // End compute pass
    wgpuComputePassEncoderEnd(computePass);
    
    // Finish command buffer and submit
    WGPUCommandBufferDescriptor cmdDesc = {};
    cmdDesc.label = WGPUStringView{"Simulation Commands", 19};
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &cmdDesc);
    
    wgpuQueueSubmit(context.getQueue(), 1, &commandBuffer);
    
    // Clean up
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuComputePassEncoderRelease(computePass);
}

void ComputePipeline::cleanup()
{
    if (simulationPipeline) {
        wgpuComputePipelineRelease(simulationPipeline);
        simulationPipeline = nullptr;
    }
    
    if (bindGroupLayout) {
        wgpuBindGroupLayoutRelease(bindGroupLayout);
        bindGroupLayout = nullptr;
    }
    
    if (simulationShaderModule) {
        wgpuShaderModuleRelease(simulationShaderModule);
        simulationShaderModule = nullptr;
    }
}