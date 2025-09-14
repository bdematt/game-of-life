#include "life.h"
#include <iostream>

Life::Life(
    WGPUInstance instance,
    WGPUAdapter adapter,
    WGPUDevice device,
    WGPUSurface surface,
    WGPUQueue queue
) : instance(instance), adapter(adapter), device(device), surface(surface), queue(queue)
{
    std::cout << "🔧 Life created" << std::endl;
    
    // Create vertex buffer with our square data
    createVertexBuffer();
    
    // Create uniform buffer with grid size
    createUniformBuffer();

    // Setup vertex layout description
    setupVertexLayout();
    
    // Create shader module
    createShaderModule();

    // Create cell state storage buffer
    createCellStateStorageBuffer();

    // Create bind group layout, specifying cell state storage and uniform buffers
    createBindGroupLayout();

    // Create render pipeline
    createRenderPipeline();

    // Create grid cell bind group
    createBindGroup();
}

Life::~Life()
{
    // Clean up render pipeline
    if (cellPipeline) wgpuRenderPipelineRelease(cellPipeline);
    
    // Clean up shader module
    if (cellShaderModule) wgpuShaderModuleRelease(cellShaderModule);
    
    // Clean up vertex buffer
    if (vertexBuffer) wgpuBufferRelease(vertexBuffer);
    
    // Clean up WebGPU resources
    if (surface) wgpuSurfaceRelease(surface);
    if (queue) wgpuQueueRelease(queue);
    if (device) wgpuDeviceRelease(device);
    if (adapter) wgpuAdapterRelease(adapter);
    if (instance) wgpuInstanceRelease(instance);

    std::cout << "🔧 Life destroyed" << std::endl;
}

void Life::createVertexBuffer()
{
    std::cout << "🔧 Creating vertex buffer..." << std::endl;
    
    // Create vertex buffer descriptor
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label = WGPUStringView{"Square Vertices", 15};
    bufferDesc.size = sizeof(VERTICES); // Size in bytes
    bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;

    // Create the buffer
    vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    
    if (!vertexBuffer) {
        std::cout << "❌ Failed to create vertex buffer!" << std::endl;
        return;
    }

    // Write vertex data to buffer
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, VERTICES, sizeof(VERTICES));
    
    std::cout << "✅ Vertex buffer created with " << VERTEX_COUNT << " VERTICES" << std::endl;
    std::cout << "   Buffer size: " << sizeof(VERTICES) << " bytes" << std::endl;
}

void Life::createUniformBuffer()
{
    std::cout << "🔧 Creating uniform buffer..." << std::endl;
    
    // Create vertex buffer descriptor
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label = WGPUStringView{"Grid Uniforms", 13};
    bufferDesc.size = sizeof(UNIFORM_ARRAY); // Size in bytes
    bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;

    // Create the buffer
    uniformBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    
    if (!uniformBuffer) {
        std::cout << "❌ Failed to create uniformBuffer buffer!" << std::endl;
        return;
    }

    // Write vertex data to buffer
    wgpuQueueWriteBuffer(queue, uniformBuffer, 0, UNIFORM_ARRAY, sizeof(UNIFORM_ARRAY));
    
    std::cout << "✅ Uniform buffer created with grize size " << GRID_SIZE << std::endl;
    std::cout << "   Buffer size: " << sizeof(UNIFORM_ARRAY) << " bytes" << std::endl;
}

// Then update your createBindGroup method:
void Life::createBindGroup()
{
    std::cout << "🔧 Creating bind groups..." << std::endl;
    
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

    bindGroups[0] = wgpuDeviceCreateBindGroup(device, &bindGroupDescA);

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

    bindGroups[1] = wgpuDeviceCreateBindGroup(device, &bindGroupDescB);

    // Check if both bind groups were created successfully
    if (!bindGroups[0] || !bindGroups[1]) {
        std::cout << "❌ Failed to create bind groups!" << std::endl;
        return;
    }
    
    std::cout << "✅ Both bind groups created successfully!" << std::endl;
}

void Life::createCellStateStorageBuffer()
{
    std::cout << "🔧 Creating storage buffer..." << std::endl;

    // Initialize Array to 0's
    cellStateArray.resize(GRID_SIZE * GRID_SIZE, 0);

    // Create both buffer descriptors first
    WGPUBufferDescriptor bufferADesc = {};
    bufferADesc.label = WGPUStringView{"Cell State A", 12};
    bufferADesc.size = cellStateArray.size() * sizeof(uint32_t);
    bufferADesc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
    bufferADesc.mappedAtCreation = false;
    cellStateStorageBufferA = wgpuDeviceCreateBuffer(device, &bufferADesc);

    WGPUBufferDescriptor bufferBDesc = {};
    bufferBDesc.label = WGPUStringView{"Cell State B", 12};
    bufferBDesc.size = cellStateArray.size() * sizeof(uint32_t);
    bufferBDesc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
    bufferBDesc.mappedAtCreation = false;
    cellStateStorageBufferB = wgpuDeviceCreateBuffer(device, &bufferBDesc);
    
    if (!cellStateStorageBufferA || !cellStateStorageBufferB) {
        std::cout << "❌ Failed to create cell state storage buffer!" << std::endl;
        return;
    }

    // Pattern A: odd indices (1, 3, 5...)
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = i % 2 == 1;
    }
    wgpuQueueWriteBuffer(queue, cellStateStorageBufferA, 0, 
                         cellStateArray.data(), 
                         cellStateArray.size() * sizeof(uint32_t));
    
    // Pattern B: even indices (0, 2, 4...)
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = i % 2 == 0;
    }
    wgpuQueueWriteBuffer(queue, cellStateStorageBufferB, 0, 
                         cellStateArray.data(), 
                         cellStateArray.size() * sizeof(uint32_t));
    
    std::cout << "✅ Cell state storage buffer created!" << std::endl;
}

void Life::createBindGroupLayout()
{
        std::cout << "🔧 Creating bind group layout..." << std::endl;
    
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
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &layoutDesc);
    
    if (!bindGroupLayout) {
        std::cout << "❌ Failed to create bind group layout!" << std::endl;
        return;
    }
    
    std::cout << "✅ Bind group layout created with 2 bindings!" << std::endl;
}

void Life::setupVertexLayout()
{
    std::cout << "🔧 Setting up vertex layout..." << std::endl;
    
    // Define the vertex attribute (position)
    vertexAttribute.format = WGPUVertexFormat_Float32x2;  // Two 32-bit floats (X, Y)
    vertexAttribute.offset = 0;                           // Start at beginning of vertex
    vertexAttribute.shaderLocation = 0;                   // Links to vertex shader input at location 0
    
    // Define the vertex buffer layout
    vertexBufferLayout.arrayStride = 8;                   // 8 bytes per vertex (2 floats × 4 bytes each)
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex; // Step per vertex (not per instance)
    vertexBufferLayout.attributeCount = 1;                // We have one attribute (position)
    vertexBufferLayout.attributes = &vertexAttribute;      // Point to our attribute
    
    std::cout << "✅ Vertex layout configured:" << std::endl;
    std::cout << "   Array stride: " << vertexBufferLayout.arrayStride << " bytes" << std::endl;
    std::cout << "   Attribute format: float32x2 at shader location " << vertexAttribute.shaderLocation << std::endl;
}

void Life::createShaderModule()
{
    std::cout << "🔧 Creating shader module..." << std::endl;
    
    // WGSL shader code as a string
    const char* shaderCode = R"(
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
    
    // Create shader module descriptor
    WGPUShaderSourceWGSL source = {};
    source.chain.sType = WGPUSType_ShaderSourceWGSL;
    source.code = WGPUStringView{shaderCode, strlen(shaderCode)};
    
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.label = WGPUStringView{"Cell Shader", 11};
    shaderModuleDesc.nextInChain = &source.chain;
    
    // Create the shader module
    cellShaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    
    if (!cellShaderModule) {
        std::cout << "❌ Failed to create shader module!" << std::endl;
        return;
    }
    
    std::cout << "✅ Shader module created with vertex and fragment shaders!" << std::endl;
}

void Life::createRenderPipeline()
{
    std::cout << "🔧 Creating render pipeline..." << std::endl;
    
    // Create render pipeline descriptor
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.label = WGPUStringView{"Cell Pipeline", 13};
    
    // Vertex state
    WGPUVertexState vertexState = {};
    vertexState.module = cellShaderModule;
    vertexState.entryPoint = WGPUStringView{"vertexMain", 10};
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;
    
    // Fragment state  
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm; // Match surface format
    colorTarget.blend = nullptr; // No blending for now
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    WGPUFragmentState fragmentState = {};
    fragmentState.module = cellShaderModule;
    fragmentState.entryPoint = WGPUStringView{"fragmentMain", 12};
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // Create pipeline layout with our explicit bind group layout
    WGPUBindGroupLayout layouts[] = { bindGroupLayout };
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Cell Pipeline Layout", 20};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = layouts; // Use our explicit layout
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);
    
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
    cellPipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
    
    // Clean up intermediate layout
    wgpuPipelineLayoutRelease(pipelineLayout);
    
    if (!cellPipeline) {
        std::cout << "❌ Failed to create render pipeline!" << std::endl;
        return;
    }
    
    std::cout << "✅ Render pipeline created!" << std::endl;
}

void Life::tick()
{
    frameCount++;
    uint32_t bindGroupIndex = (frameCount / 120) % 2;

    // Get current surface texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
    
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
        std::cout << "❌ Failed to get surface texture, status: " << surfaceTexture.status << std::endl;
        return;
    }

    // Create texture view
    WGPUTextureViewDescriptor viewDesc = {};
    viewDesc.format = WGPUTextureFormat_BGRA8Unorm;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    
    WGPUTextureView view = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);

    // Create command encoder
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = WGPUStringView{"Life Command Encoder", 21};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

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
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0, sizeof(VERTICES));
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroups[bindGroupIndex], 0, nullptr);
    wgpuRenderPassEncoderDraw(pass, VERTEX_COUNT, INSTANCE_COUNT, 0, 0);
    
    // End render pass
    wgpuRenderPassEncoderEnd(pass);

    // Finish command encoder
    WGPUCommandBufferDescriptor cmdDesc = {};
    cmdDesc.label = WGPUStringView{"Life Commands", 13};
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &cmdDesc);

    // Submit command buffer to queue
    wgpuQueueSubmit(queue, 1, &commandBuffer);

    // Note: No need to call wgpuSurfacePresent() when using emscripten_set_main_loop
    // Emscripten handles presentation automatically via requestAnimationFrame

    // Clean up resources
    wgpuTextureViewRelease(view);
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuRenderPassEncoderRelease(pass);
}