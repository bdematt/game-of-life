#include "Life.h"
#include "webgpu.hpp"
#include "Shader.h"

Life::Life()
{
    requestAdapter();
    requestDevice();
    createSurface();
    configureSurface();
    createBindGroupLayout();
    createRenderPipeline();
    createVertexBuffer();
    createUniformBuffer();
    createBindGroup();
}

Life::~Life()
{
    cleanup();
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

void Life::createBindGroupLayout()
{
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry {};
    bindGroupLayoutEntry.setDefault();
    bindGroupLayoutEntry.binding = 0;
    bindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bindGroupLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    bindGroupLayoutEntry.buffer.minBindingSize = sizeof(UNIFORM_ARRAY);

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc {};
    bindGroupLayoutDesc.setDefault();
    bindGroupLayoutDesc.label = "Cell bind group layout";
    bindGroupLayoutDesc.entryCount = 1;
    bindGroupLayoutDesc.entries = &bindGroupLayoutEntry;

    bindGroupLayout = getDevice().createBindGroupLayout(bindGroupLayoutDesc);
    if (!bindGroupLayout) throw Life::InitializationError("Failed to create bind group layout");   
}

void Life::createRenderPipeline()
{
    wgpu::ShaderModule cellShaderModule = Shader::loadModuleFromFile(
        getDevice(),
        "/shaders/shader.wgsl"
    );

    wgpu::PipelineLayoutDescriptor layoutDesc {};
    layoutDesc.setDefault();
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = reinterpret_cast<const WGPUBindGroupLayout*>(&getBindGroupLayout());
    wgpu::PipelineLayout pipelineLayout = getDevice().createPipelineLayout(layoutDesc);

    // Vertex setup
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

    // Pipeline descriptor
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

    // Clean up temporary resources
    pipelineLayout.release();
    cellShaderModule.release();
}

void Life::createVertexBuffer()
{
    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.setDefault();
    bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(VERTICES);
    
    vertexBuffer = getDevice().createBuffer(bufferDesc);
    if (!vertexBuffer) throw Life::InitializationError("Failed to create vertexBuffer buffer");

    constexpr uint64_t BUFFER_OFFSET = 0;
    getQueue().writeBuffer(vertexBuffer, BUFFER_OFFSET, VERTICES, sizeof(VERTICES));
}

void Life::createUniformBuffer()
{
    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.setDefault();
    bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(UNIFORM_ARRAY);
    
    uniformBuffer = getDevice().createBuffer(bufferDesc);
    if (!uniformBuffer) throw Life::InitializationError("Failed to create uniform buffer");

    constexpr uint64_t BUFFER_OFFSET = 0;
    getQueue().writeBuffer(uniformBuffer, BUFFER_OFFSET, UNIFORM_ARRAY, sizeof(UNIFORM_ARRAY));
}

void Life::createBindGroup()
{
    wgpu::BindGroupEntry bindGroupEntry {};
    bindGroupEntry.setDefault();
    bindGroupEntry.binding = 0;
    bindGroupEntry.buffer = getUniformBuffer();
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = sizeof(UNIFORM_ARRAY);

    wgpu::BindGroupDescriptor bindgroupDesc {};
    bindgroupDesc.setDefault();
    bindgroupDesc.label = "Cell renderer bind group";
    bindgroupDesc.layout = bindGroupLayout;
    bindgroupDesc.entryCount = 1;
    bindgroupDesc.entries = &bindGroupEntry;

    bindGroup = device.createBindGroup(bindgroupDesc);
    if (!bindGroup) throw Life::InitializationError("Failed to create bindGroup");
}

void Life::cleanup()
{
    if (bindGroup) bindGroup.release();
    if (bindGroupLayout) bindGroupLayout.release();
    if (uniformBuffer) uniformBuffer.release();
    if (vertexBuffer) vertexBuffer.release();
    if (renderPipeline) renderPipeline.release();
    if (surface) surface.release();
    if (queue) queue.release();
    if (device) device.release();
    if (adapter) adapter.release();
    if (instance) instance.release();
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
    pass.setBindGroup(0, getBindGroup(), 0, nullptr);
    constexpr uint32_t VERTEX_COUNT = sizeof(VERTICES) / sizeof(float) / 2;
    pass.draw(VERTEX_COUNT, GRID_SIZE * GRID_SIZE, 0, 0);
    pass.end();

    wgpu::CommandBuffer commandBuffer = encoder.finish();
    getQueue().submit(commandBuffer);
    
    view.release();
}