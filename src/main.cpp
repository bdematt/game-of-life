#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <webgpu/webgpu.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"

#include <fstream>
#include <sstream>
#include <string>

std::string loadShaderCode(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static constexpr float VERTICES[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
    
    -0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f,
};

// Global state
struct AppState {
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::Adapter adapter;
    wgpu::Queue queue;
    wgpu::RenderPipeline cellPipeline;
    wgpu::Buffer vertexBuffer;
} state;

void render_frame() {
    // Get the current texture from the surface (this MUST happen every frame)
    wgpu::SurfaceTexture surfaceTexture {};
    state.surface.getCurrentTexture(&surfaceTexture);
    wgpu::Texture texture = surfaceTexture.texture;
    wgpu::TextureView view = texture.createView();

    // Create command encoder (lightweight, per-frame)
    wgpu::CommandEncoder encoder = state.device.createCommandEncoder();

    // Set up the color attachment (lightweight struct setup)
    wgpu::RenderPassColorAttachment colorAttachment {};
    colorAttachment.view = view;
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color(0.0, 0.0, 0.4, 1.0);

    // Set up the render pass descriptor (lightweight struct setup)
    wgpu::RenderPassDescriptor renderPassDesc {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    // Encode rendering commands
    wgpu::RenderPassEncoder pass = encoder.beginRenderPass(renderPassDesc);
    pass.setPipeline(state.cellPipeline);              // Set the pipeline
    pass.setVertexBuffer(0, state.vertexBuffer, 0, 48);// Set vertex buffer
    pass.draw(6, 1, 0, 0);                             // Draw 6 vertices (2 triangles)
    pass.end();

    // Submit commands
    wgpu::CommandBuffer commandBuffer = encoder.finish();
    state.queue.submit(commandBuffer);
}

int main() {
    std::cout << "🚀 Starting WebGPU application..." << std::endl;
    
    wgpu::Instance instance {};

    wgpu::RequestAdapterOptions adapterOptions {};
    adapterOptions.setDefault();
    state.adapter = instance.requestAdapter(adapterOptions);

    wgpu::DeviceDescriptor deviceDesc {};
    deviceDesc.setDefault();
    state.device = state.adapter.requestDevice(deviceDesc);
    // Set up error callback
    auto errorCallback = state.device.setUncapturedErrorCallback([](wgpu::ErrorType type, const char* message) {
        std::cerr << "❌ WebGPU Error (" << static_cast<int>(type) << "): " << message << std::endl;
    });

    // Get queue once during initialization
    state.queue = state.device.getQueue();

    // Set up the canvas selector
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector surfaceSelector {};
    surfaceSelector.setDefault();
    surfaceSelector.selector = "#canvas"; 

    // Chain the canvas selector to the surface descriptor
    wgpu::SurfaceDescriptor surfaceDesc {};
    surfaceDesc.setDefault();
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&surfaceSelector);
    state.surface = instance.createSurface(surfaceDesc);

    // Configure the surface with proper usage flags
    wgpu::SurfaceConfiguration config {};
    config.setDefault();
    config.device = state.device;
    config.format = state.surface.getPreferredFormat(state.adapter);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = 800;
    config.height = 600;
    state.surface.configure(config);

    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.setDefault();
    bufferDesc.label = "Square Vertices";
    bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(VERTICES);
    
    wgpu::VertexAttribute vertexAttribute {};
    vertexAttribute.setDefault();
    vertexAttribute.format = wgpu::VertexFormat::Float32x2;
    vertexAttribute.offset = 0;
    vertexAttribute.shaderLocation = 0;

    wgpu::VertexBufferLayout vertexBufferLayout {};
    vertexBufferLayout.setDefault();
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    vertexBufferLayout.arrayStride = 8;
    vertexBufferLayout.attributeCount = 1;
    vertexBufferLayout.attributes = &vertexAttribute;

    wgpu::Buffer buffer = state.device.createBuffer(bufferDesc);
    state.queue.writeBuffer(buffer, 0, VERTICES, sizeof(VERTICES));

    std::string shaderCode = loadShaderCode("/shaders/shader.wgsl");
    std::cout << "Shader code loaded, length: " << shaderCode.length() << std::endl;

    // Create the WGSL descriptor with the shader code
    wgpu::ShaderModuleWGSLDescriptor wgslDesc {};
    wgslDesc.setDefault();
    wgslDesc.code = shaderCode.c_str();

    // Create the shader module descriptor and chain the WGSL descriptor
    wgpu::ShaderModuleDescriptor shaderDesc {};
    shaderDesc.setDefault();
    shaderDesc.label = "Cell Shader";
    shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgslDesc);

    // Create the shader module
    wgpu::ShaderModule cellShaderModule = state.device.createShaderModule(shaderDesc);


    // Create the render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc {};
    pipelineDesc.setDefault();
    pipelineDesc.label = "Cell pipeline";


    // Vertex state
    pipelineDesc.vertex.module = cellShaderModule;
    pipelineDesc.vertex.entryPoint = "vertexMain";
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    // Fragment state
    wgpu::ColorTargetState colorTarget {};
    colorTarget.setDefault();
    colorTarget.format = config.format;  // Use the same format as the surface

    wgpu::FragmentState fragmentState {};
    fragmentState.setDefault();
    fragmentState.module = cellShaderModule;
    fragmentState.entryPoint = "fragmentMain";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDesc.fragment = &fragmentState;

    // Create the pipeline
    state.cellPipeline = state.device.createRenderPipeline(pipelineDesc);
    state.vertexBuffer = buffer;

    // Use emscripten_set_main_loop for the render loop
    emscripten_set_main_loop(render_frame, 0, 1);
    
    return 0;
}