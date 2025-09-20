#include "RenderPipeline.h"
#include "Life.h"

RenderPipeline::RenderPipeline(const WebGPUContext& context, const Geometry& geometry)
{
    try {
        createCellShaderModule(context);
        createUniformBuffer(context);
        createCellStateStorageBuffer(context);
        createBindGroupLayout(context);
        createRenderPipeline(context, geometry);
        createBindGroup(context);
    } catch(...) {
        cleanup();  // Clean up on any failure
        throw;      // Re-throw the exception
    }
}

RenderPipeline::~RenderPipeline()
{
    // Release in reverse order of creation
    if (bindGroups[1]) wgpuBindGroupRelease(bindGroups[1]);
    if (bindGroups[0]) wgpuBindGroupRelease(bindGroups[0]);
    if (bindGroupLayout) wgpuBindGroupLayoutRelease(bindGroupLayout);
    if (cellPipeline) wgpuRenderPipelineRelease(cellPipeline);
    if (cellStateStorageBufferB) wgpuBufferRelease(cellStateStorageBufferB);
    if (cellStateStorageBufferA) wgpuBufferRelease(cellStateStorageBufferA);
    if (uniformBuffer) wgpuBufferRelease(uniformBuffer);
    if (cellShaderModule) wgpuShaderModuleRelease(cellShaderModule);
}
void RenderPipeline::createCellShaderModule(const WebGPUContext& context)
{
    WGPUShaderSourceWGSL source = {};
    source.chain.sType = WGPUSType_ShaderSourceWGSL;
    source.code = WGPUStringView{SHADER_CODE, strlen(SHADER_CODE)};
    
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.label = WGPUStringView{"Cell Shader", 11};
    shaderModuleDesc.nextInChain = &source.chain;

    cellShaderModule = wgpuDeviceCreateShaderModule(context.getDevice(), &shaderModuleDesc);
    if (!cellShaderModule) throw Life::InitializationError("Failed to create shader module");
}

void RenderPipeline::createUniformBuffer(const WebGPUContext& context)
{
    
    // Create vertex buffer descriptor
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label = WGPUStringView{"Grid Uniforms", 13};
    bufferDesc.size = sizeof(UNIFORM_ARRAY); // Size in bytes
    bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;

    // Create the buffer
    uniformBuffer = wgpuDeviceCreateBuffer(context.getDevice(), &bufferDesc);
    if (!uniformBuffer) throw Life::InitializationError("Failed to create uniform buffer");

    // Write vertex data to buffer
    wgpuQueueWriteBuffer(context.getQueue(), uniformBuffer, 0, UNIFORM_ARRAY, sizeof(UNIFORM_ARRAY));
    
}

// Then update your createBindGroup method:
void RenderPipeline::createBindGroup(const WebGPUContext& context)
{
    
    // Create uniform bind group entry (same for both bind groups)
    WGPUBindGroupEntry uniformBindGroupEntry = {};
    uniformBindGroupEntry.binding = 0;
    uniformBindGroupEntry.buffer = uniformBuffer;
    uniformBindGroupEntry.offset = 0;
    uniformBindGroupEntry.size = sizeof(UNIFORM_ARRAY);

    // Create bind group A (uses storage buffer A)
    WGPUBindGroupEntry storageBindGroupEntryA = {};
    storageBindGroupEntryA.binding = 1;
    storageBindGroupEntryA.buffer = cellStateStorageBufferA;
    storageBindGroupEntryA.offset = 0;
    storageBindGroupEntryA.size = cellStateArray.size() * sizeof(uint32_t);

    WGPUBindGroupEntry bindGroupEntriesA[] = {
        uniformBindGroupEntry,
        storageBindGroupEntryA
    };

    WGPUBindGroupDescriptor bindGroupDescA = {};
    bindGroupDescA.label = WGPUStringView{"Cell renderer bind group A", 28};
    bindGroupDescA.layout = bindGroupLayout;
    bindGroupDescA.entryCount = 2;
    bindGroupDescA.entries = bindGroupEntriesA;

    bindGroups[0] = wgpuDeviceCreateBindGroup(context.getDevice(), &bindGroupDescA);

    // Create bind group B (uses storage buffer B)
    WGPUBindGroupEntry storageBindGroupEntryB = {};
    storageBindGroupEntryB.binding = 1;
    storageBindGroupEntryB.buffer = cellStateStorageBufferB;
    storageBindGroupEntryB.offset = 0;
    storageBindGroupEntryB.size = cellStateArray.size() * sizeof(uint32_t);

    WGPUBindGroupEntry bindGroupEntriesB[] = {
        uniformBindGroupEntry,
        storageBindGroupEntryB
    };

    WGPUBindGroupDescriptor bindGroupDescB = {};
    bindGroupDescB.label = WGPUStringView{"Cell renderer bind group B", 28};
    bindGroupDescB.layout = bindGroupLayout;
    bindGroupDescB.entryCount = 2;
    bindGroupDescB.entries = bindGroupEntriesB;

    bindGroups[1] = wgpuDeviceCreateBindGroup(context.getDevice(), &bindGroupDescB);

    // Check if both bind groups were created successfully
    if (!bindGroups[0] || !bindGroups[1]) throw Life::InitializationError("Failed to create bind groups");
    
}

void RenderPipeline::renderFrame(const WebGPUContext &context, const Geometry &geometry)
{
    frameCount++;
    uint32_t bindGroupIndex = (frameCount / 120) % 2;

    // Get current surface texture
    const WGPUSurfaceTexture surfaceTexture = context.getCurentSurfaceTexture();
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
        throw Life::InitializationError("❌ Failed to get surface texture");

    WGPUTextureView view = context.createCurrentTextureView(surfaceTexture.texture);

    // Create command encoder
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = WGPUStringView{"Life Command Encoder", 21};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(context.getDevice(), &encoderDesc);

    // Create render pass with color attachment
    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = {0.0f, 0.0f, 0.4f, 1.0f}; // Dark blue background
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    WGPURenderPassDescriptor passDesc = {};
    passDesc.label = WGPUStringView{"Life Render Pass", 16};
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;

    // Begin render pass
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
    
    // Set the render pipeline and draw the square!
    wgpuRenderPassEncoderSetPipeline(pass, cellPipeline);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, geometry.getVertexBuffer(), 0, geometry.getSizeOfVertices());
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroups[bindGroupIndex], 0, nullptr);
    wgpuRenderPassEncoderDraw(pass, geometry.getVertexCount(), INSTANCE_COUNT, 0, 0);
    
    // End render pass
    wgpuRenderPassEncoderEnd(pass);

    // Finish command encoder
    WGPUCommandBufferDescriptor cmdDesc = {};
    cmdDesc.label = WGPUStringView{"Life Commands", 13};
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &cmdDesc);

    // Submit command buffer to queue
    wgpuQueueSubmit(context.getQueue(), 1, &commandBuffer);

    // Note: No need to call wgpuSurfacePresent() when using emscripten_set_main_loop
    // Emscripten handles presentation automatically via requestAnimationFrame

    // Clean up resources
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuRenderPassEncoderRelease(pass);
    wgpuTextureViewRelease(view);
}

void RenderPipeline::createCellStateStorageBuffer(const WebGPUContext& context)
{

    // Initialize Array to 0's
    cellStateArray.resize(GRID_SIZE * GRID_SIZE, 0);

    // Create both buffer descriptors first
    WGPUBufferDescriptor bufferADesc = {};
    bufferADesc.label = WGPUStringView{"Cell State A", 12};
    bufferADesc.size = cellStateArray.size() * sizeof(uint32_t);
    bufferADesc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
    bufferADesc.mappedAtCreation = false;
    cellStateStorageBufferA = wgpuDeviceCreateBuffer(context.getDevice(), &bufferADesc);

    WGPUBufferDescriptor bufferBDesc = {};
    bufferBDesc.label = WGPUStringView{"Cell State B", 12};
    bufferBDesc.size = cellStateArray.size() * sizeof(uint32_t);
    bufferBDesc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
    bufferBDesc.mappedAtCreation = false;
    cellStateStorageBufferB = wgpuDeviceCreateBuffer(context.getDevice(), &bufferBDesc);
    
    if (!cellStateStorageBufferA || !cellStateStorageBufferB) throw Life::InitializationError("Failed to create storage buffers");

    // Pattern A: odd indices (1, 3, 5...)
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = i % 2 == 1;
    }
    wgpuQueueWriteBuffer(context.getQueue(), cellStateStorageBufferA, 0, 
                         cellStateArray.data(), 
                         cellStateArray.size() * sizeof(uint32_t));
    
    // Pattern B: even indices (0, 2, 4...)
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = i % 2 == 0;
    }
    wgpuQueueWriteBuffer(context.getQueue(), cellStateStorageBufferB, 0, 
                         cellStateArray.data(), 
                         cellStateArray.size() * sizeof(uint32_t));
    
}

void RenderPipeline::createBindGroupLayout(const WebGPUContext& context)
{
    
    // Define binding layout entries
    WGPUBindGroupLayoutEntry bindingLayouts[2];
    
    // Uniform buffer binding (binding = 0)
    bindingLayouts[0] = {};
    bindingLayouts[0].binding = 0;
    bindingLayouts[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    bindingLayouts[0].buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayouts[0].buffer.hasDynamicOffset = false;
    bindingLayouts[0].buffer.minBindingSize = 0; // Use actual buffer size
    
    // Storage buffer binding (binding = 1)  
    bindingLayouts[1] = {};
    bindingLayouts[1].binding = 1;
    bindingLayouts[1].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    bindingLayouts[1].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    bindingLayouts[1].buffer.hasDynamicOffset = false;
    bindingLayouts[1].buffer.minBindingSize = 0; // Use actual buffer size
    
    // Create bind group layout descriptor
    WGPUBindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Cell Bind Group Layout", 23};
    layoutDesc.entryCount = 2;
    layoutDesc.entries = bindingLayouts;
    
    // Create the bind group layout
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(context.getDevice(), &layoutDesc);
    
    if (!bindGroupLayout) throw Life::InitializationError("Failed to create bind group layout");
    
}

void RenderPipeline::createRenderPipeline(const WebGPUContext& context, const Geometry& geometry)
{
    
    // Create render pipeline descriptor
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.label = WGPUStringView{"Cell Pipeline", 13};
    
    // Vertex state
    WGPUVertexState vertexState = {};
    vertexState.module = getCellShaderModule();
    vertexState.entryPoint = WGPUStringView{"vertexMain", 10};
    vertexState.bufferCount = 1;
    auto layout = geometry.getVertexBufferLayout();
    vertexState.buffers = &layout;
    
    // Fragment state  
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm; // Match surface format
    colorTarget.blend = nullptr; // No blending for now
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    WGPUFragmentState fragmentState = {};
    fragmentState.module = getCellShaderModule();
    fragmentState.entryPoint = WGPUStringView{"fragmentMain", 12};
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // Create pipeline layout with our explicit bind group layout
    WGPUBindGroupLayout layouts[] = { bindGroupLayout };
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Cell Pipeline Layout", 20};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = layouts; // Use our explicit layout
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(context.getDevice(), &layoutDesc);
    
    // Complete pipeline descriptor
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    
    // Create the pipeline
    cellPipeline = wgpuDeviceCreateRenderPipeline(context.getDevice(), &pipelineDesc);
    
    // Clean up intermediate layout
    wgpuPipelineLayoutRelease(pipelineLayout);
    
    if (!cellPipeline) throw Life::InitializationError("Failed to create cell pipeline");
    
}
void RenderPipeline::cleanup()
{
    if (bindGroups[1]) wgpuBindGroupRelease(bindGroups[1]);
    if (bindGroups[0]) wgpuBindGroupRelease(bindGroups[0]);
    if (bindGroupLayout) wgpuBindGroupLayoutRelease(bindGroupLayout);
    if (cellPipeline) wgpuRenderPipelineRelease(cellPipeline);
    if (cellStateStorageBufferB) wgpuBufferRelease(cellStateStorageBufferB);
    if (cellStateStorageBufferA) wgpuBufferRelease(cellStateStorageBufferA);
    if (uniformBuffer) wgpuBufferRelease(uniformBuffer);
    if (cellShaderModule) wgpuShaderModuleRelease(cellShaderModule);
}