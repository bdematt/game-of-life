#include "Life.h"
#include "webgpu.hpp"
#include "Shader.h"

Life::Life()
{
    requestAdapter();
    requestDevice();
    createSurface();
    configureSurface();
    createRenderPipeline();
    createBuffer();
}

Life::~Life()
{
}

void Life::requestAdapter()
{
    wgpu::RequestAdapterOptions adapterOptions {};
    adapterOptions.setDefault();
    adapter = instance.requestAdapter(adapterOptions);
    if (!adapter) throw Life::InitializationError("Failed to request adapter");
}

void Life::requestDevice()
{
    wgpu::DeviceDescriptor deviceDesc {};
    deviceDesc.setDefault();
    device = adapter.requestDevice(deviceDesc);
    if (!device) throw Life::InitializationError("Failed to request device");
    queue = device.getQueue();
    if (!queue) throw Life::InitializationError("Failed to get queue");

}

void Life::createSurface()
{
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector surfaceSelector {};
    surfaceSelector.setDefault();
    surfaceSelector.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc {};
    surfaceDesc.setDefault();
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&surfaceSelector);
    surface = instance.createSurface(surfaceDesc);

    if (!surface) throw Life::InitializationError("Failed to create surface");    
}

void Life::configureSurface()
{
    surfaceConfig.setDefault();
    surfaceConfig.device = device;
    surfaceConfig.format = getSurface().getPreferredFormat(adapter);
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    surfaceConfig.width = 800;
    surfaceConfig.height = 600;
    surface.configure(surfaceConfig);
}

void Life::createRenderPipeline()
{
    wgpu::ShaderModule cellShaderModule = Shader::loadModuleFromFile(
        getDevice(),
        "/shaders/shader.wgsl"
    );

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
    wgpu::PipelineLayout pipelineLayout = getDevice().createPipelineLayout(layoutDesc);

    wgpu::RenderPipelineDescriptor pipelineDesc {};
    pipelineDesc.setDefault();
    pipelineDesc.layout = pipelineLayout;

    pipelineDesc.vertex.module = cellShaderModule;
    pipelineDesc.vertex.entryPoint = "vertexMain";
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    wgpu::ColorTargetState colorTarget {};
    colorTarget.setDefault();
    colorTarget.format = surfaceConfig.format;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState {};
    fragmentState.setDefault();
    fragmentState.module = cellShaderModule;
    fragmentState.entryPoint = "fragmentMain";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDesc.fragment = &fragmentState;

    renderPipeline = getDevice().createRenderPipeline(pipelineDesc);
}

void Life::createBuffer()
{
    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.setDefault();
    bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(VERTICES);
    
    vertexBuffer = getDevice().createBuffer(bufferDesc);
    getQueue().writeBuffer(vertexBuffer, 0, VERTICES, sizeof(VERTICES));
}

void Life::renderFrame()
{
    wgpu::SurfaceTexture surfaceTexture {};
    getSurface().getCurrentTexture(&surfaceTexture);
    wgpu::Texture texture = surfaceTexture.texture;
    wgpu::TextureView view = texture.createView();

    wgpu::CommandEncoder encoder = getDevice().createCommandEncoder();

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
    pass.setPipeline(getRenderPipeline());
    pass.setVertexBuffer(0, getVertexBuffer(), 0, sizeof(VERTICES));
    pass.draw(6, 1, 0, 0);
    pass.end();

    wgpu::CommandBuffer commandBuffer = encoder.finish();
    getQueue().submit(commandBuffer);
}