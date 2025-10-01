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
    -0.8f, -0.8f,
     0.8f, -0.8f,
     0.8f,  0.8f,
    
    -0.8f, -0.8f,
     0.8f,  0.8f,
    -0.8f,  0.8f,
};

struct AppState {
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::Adapter adapter;
    wgpu::Queue queue;
    wgpu::RenderPipeline cellPipeline;
    wgpu::Buffer vertexBuffer;
} state;

void render_frame() {
    wgpu::SurfaceTexture surfaceTexture {};
    state.surface.getCurrentTexture(&surfaceTexture);
    wgpu::Texture texture = surfaceTexture.texture;
    wgpu::TextureView view = texture.createView();

    wgpu::CommandEncoder encoder = state.device.createCommandEncoder();

    wgpu::RenderPassColorAttachment colorAttachment {};
    colorAttachment.view = view;
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color(0.0, 0.0, 0.4, 1.0);

    wgpu::RenderPassDescriptor renderPassDesc {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder pass = encoder.beginRenderPass(renderPassDesc);
    pass.setPipeline(state.cellPipeline);
    pass.setVertexBuffer(0, state.vertexBuffer, 0, sizeof(VERTICES));
    pass.draw(6, 1, 0, 0);
    pass.end();

    wgpu::CommandBuffer commandBuffer = encoder.finish();
    state.queue.submit(commandBuffer);
}

int main() {
    wgpu::Instance instance {};

    wgpu::RequestAdapterOptions adapterOptions {};
    adapterOptions.setDefault();
    state.adapter = instance.requestAdapter(adapterOptions);

    wgpu::DeviceDescriptor deviceDesc {};
    deviceDesc.setDefault();
    state.device = state.adapter.requestDevice(deviceDesc);

    state.queue = state.device.getQueue();

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector surfaceSelector {};
    surfaceSelector.setDefault();
    surfaceSelector.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc {};
    surfaceDesc.setDefault();
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&surfaceSelector);
    state.surface = instance.createSurface(surfaceDesc);

    wgpu::SurfaceConfiguration config {};
    config.setDefault();
    config.device = state.device;
    config.format = state.surface.getPreferredFormat(state.adapter);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = 800;
    config.height = 600;
    state.surface.configure(config);

    std::string shaderCode = loadShaderCode("/shaders/shader.wgsl");
    
    wgpu::ShaderModuleWGSLDescriptor wgslDesc {};
    wgslDesc.setDefault();
    wgslDesc.code = shaderCode.c_str();

    wgpu::ShaderModuleDescriptor shaderDesc {};
    shaderDesc.setDefault();
    shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgslDesc);
    wgpu::ShaderModule cellShaderModule = state.device.createShaderModule(shaderDesc);

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

    wgpu::PipelineLayoutDescriptor layoutDesc {};
    layoutDesc.setDefault();
    layoutDesc.bindGroupLayoutCount = 0;
    layoutDesc.bindGroupLayouts = nullptr;
    wgpu::PipelineLayout pipelineLayout = state.device.createPipelineLayout(layoutDesc);

    wgpu::RenderPipelineDescriptor pipelineDesc {};
    pipelineDesc.setDefault();
    pipelineDesc.layout = pipelineLayout;

    pipelineDesc.vertex.module = cellShaderModule;
    pipelineDesc.vertex.entryPoint = "vertexMain";
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    wgpu::ColorTargetState colorTarget {};
    colorTarget.setDefault();
    colorTarget.format = config.format;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState {};
    fragmentState.setDefault();
    fragmentState.module = cellShaderModule;
    fragmentState.entryPoint = "fragmentMain";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDesc.fragment = &fragmentState;

    state.cellPipeline = state.device.createRenderPipeline(pipelineDesc);

    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.setDefault();
    bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(VERTICES);
    
    state.vertexBuffer = state.device.createBuffer(bufferDesc);
    state.queue.writeBuffer(state.vertexBuffer, 0, VERTICES, sizeof(VERTICES));

    emscripten_set_main_loop(render_frame, 0, 1);
    
    return 0;
}