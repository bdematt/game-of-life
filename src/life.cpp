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
    
    // Setup vertex layout description
    setupVertexLayout();
    
    // Create shader module
    createShaderModule();
    
    // Create render pipeline
    createRenderPipeline();
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
    bufferDesc.size = sizeof(vertices); // Size in bytes
    bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;

    // Create the buffer
    vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    
    if (!vertexBuffer) {
        std::cout << "❌ Failed to create vertex buffer!" << std::endl;
        return;
    }

    // Write vertex data to buffer
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertices, sizeof(vertices));
    
    std::cout << "✅ Vertex buffer created with " << vertexCount << " vertices" << std::endl;
    std::cout << "   Buffer size: " << sizeof(vertices) << " bytes" << std::endl;
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
        @vertex
        fn vertexMain(@location(0) pos: vec2f) -> @builtin(position) vec4f {
            return vec4f(pos, 0, 1);
        }
        
        @fragment
        fn fragmentMain() -> @location(0) vec4f {
            return vec4f(1, 0, 0, 1); // (Red, Green, Blue, Alpha)
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
    
    // Pipeline layout (auto)
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.label = WGPUStringView{"Cell Pipeline Layout", 20};
    layoutDesc.bindGroupLayoutCount = 0;
    layoutDesc.bindGroupLayouts = nullptr;
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
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0, sizeof(vertices));
    wgpuRenderPassEncoderDraw(pass, vertexCount, 1, 0, 0);
    
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