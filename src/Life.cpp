#include "Life.h"
#include "webgpu.hpp"
#include "Shader.h"

Life::Life()
    : cellStateArray(GRID_SIZE * GRID_SIZE)
    , lastFrameTime(std::chrono::steady_clock::now())
{
    requestAdapter();
    requestDevice();
    createSurface();
    configureSurface();
    createBindGroupLayout();
    createRenderPipeline();
    createVertexBuffer();
    createStorageBuffers();
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
    std::array<wgpu::BindGroupLayoutEntry, 2> entries;

    wgpu::BindGroupLayoutEntry uniformBindGroupLayoutEntry {};
    uniformBindGroupLayoutEntry.setDefault();
    uniformBindGroupLayoutEntry.binding = 0;
    uniformBindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    uniformBindGroupLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    uniformBindGroupLayoutEntry.buffer.minBindingSize = sizeof(UNIFORM_ARRAY);
    entries[0] = uniformBindGroupLayoutEntry;

    wgpu::BindGroupLayoutEntry storageBindGroupLayoutEntry {};
    storageBindGroupLayoutEntry.setDefault();
    storageBindGroupLayoutEntry.binding = 1;
    storageBindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    storageBindGroupLayoutEntry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    storageBindGroupLayoutEntry.buffer.minBindingSize = cellStateArray.size() * sizeof(uint32_t);
    entries[1] = storageBindGroupLayoutEntry;

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc {};
    bindGroupLayoutDesc.setDefault();
    bindGroupLayoutDesc.label = "Cell bind group layout";
    bindGroupLayoutDesc.entryCount = 2;
    bindGroupLayoutDesc.entries = entries.data();

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

void Life::createStorageBuffers()
{
    // Initialize cell state with a pattern
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = i % 2 == 1;
    }
    
    wgpu::BufferDescriptor bufferDesc {};
    bufferDesc.label = "Cell State Storage";
    bufferDesc.size = cellStateArray.size() * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst; 
    
    // Create read buffer
    cellBuffers.read = device.createBuffer(bufferDesc);
    if (!cellBuffers.read) throw Life::InitializationError("Failed to create read storage buffer");
    
    // Create write buffer
    cellBuffers.write = device.createBuffer(bufferDesc);
    if (!cellBuffers.write) throw Life::InitializationError("Failed to create write storage buffer");
    
    // Initialize both buffers with the same data
    constexpr uint64_t BUFFER_OFFSET = 0;
    queue.writeBuffer(cellBuffers.read, BUFFER_OFFSET, cellStateArray.data(), 
                     cellStateArray.size() * sizeof(uint32_t));
    queue.writeBuffer(cellBuffers.write, BUFFER_OFFSET, cellStateArray.data(), 
                     cellStateArray.size() * sizeof(uint32_t));
}

void Life::createBindGroup()
{
    // Create bind group for READ buffer
    std::array<wgpu::BindGroupEntry, 2> readEntries;

    readEntries[0].setDefault();
    readEntries[0].binding = 0;
    readEntries[0].buffer = getUniformBuffer();
    readEntries[0].offset = 0;
    readEntries[0].size = sizeof(UNIFORM_ARRAY);

    readEntries[1].setDefault();
    readEntries[1].binding = 1;
    readEntries[1].buffer = cellBuffers.read;
    readEntries[1].offset = 0;
    readEntries[1].size = cellStateArray.size() * sizeof(uint32_t);

    wgpu::BindGroupDescriptor readBindGroupDesc {};
    readBindGroupDesc.setDefault();
    readBindGroupDesc.label = "Cell read bind group";
    readBindGroupDesc.layout = bindGroupLayout;
    readBindGroupDesc.entryCount = readEntries.size();
    readBindGroupDesc.entries = readEntries.data();

    cellBuffers.readBindGroup = device.createBindGroup(readBindGroupDesc);
    if (!cellBuffers.readBindGroup) throw Life::InitializationError("Failed to create read bindGroup");

    // Create bind group for WRITE buffer
    std::array<wgpu::BindGroupEntry, 2> writeEntries;

    writeEntries[0].setDefault();
    writeEntries[0].binding = 0;
    writeEntries[0].buffer = getUniformBuffer();
    writeEntries[0].offset = 0;
    writeEntries[0].size = sizeof(UNIFORM_ARRAY);

    writeEntries[1].setDefault();
    writeEntries[1].binding = 1;
    writeEntries[1].buffer = cellBuffers.write;
    writeEntries[1].offset = 0;
    writeEntries[1].size = cellStateArray.size() * sizeof(uint32_t);

    wgpu::BindGroupDescriptor writeBindGroupDesc {};
    writeBindGroupDesc.setDefault();
    writeBindGroupDesc.label = "Cell write bind group";
    writeBindGroupDesc.layout = bindGroupLayout;
    writeBindGroupDesc.entryCount = writeEntries.size();
    writeBindGroupDesc.entries = writeEntries.data();

    cellBuffers.writeBindGroup = device.createBindGroup(writeBindGroupDesc);
    if (!cellBuffers.writeBindGroup) throw Life::InitializationError("Failed to create write bindGroup");
}

void Life::cleanup()
{
    if (bindGroup) bindGroup.release();
    if (cellBuffers.writeBindGroup) cellBuffers.writeBindGroup.release();
    if (cellBuffers.readBindGroup) cellBuffers.readBindGroup.release();
    if (cellBuffers.write) cellBuffers.write.release();
    if (cellBuffers.read) cellBuffers.read.release();
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
    if (!shouldUpdateCells()) {
        return;
    }
    
    updateCellState();

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
    pass.setBindGroup(0, cellBuffers.readBindGroup, 0, nullptr);  // Use READ buffer
    constexpr uint32_t VERTEX_COUNT = sizeof(VERTICES) / sizeof(float) / 2;
    pass.draw(VERTEX_COUNT, GRID_SIZE * GRID_SIZE, 0, 0);
    pass.end();

    wgpu::CommandBuffer commandBuffer = encoder.finish();
    getQueue().submit(commandBuffer);
    
    view.release();
    cellBuffers.swap();
}

void Life::updateCellState()
{
    // Alternate between even and odd cell patterns
    static bool showEvenCells = true;
    
    for (size_t i = 0; i < cellStateArray.size(); i++) {
        cellStateArray[i] = (i % 2 == (showEvenCells ? 0 : 1)) ? 1 : 0;
    }
    
    // Write the new pattern to the WRITE buffer
    constexpr uint64_t BUFFER_OFFSET = 0;
    queue.writeBuffer(cellBuffers.write, BUFFER_OFFSET, cellStateArray.data(), 
                     cellStateArray.size() * sizeof(uint32_t));
    
    // Toggle for next frame
    showEvenCells = !showEvenCells;
}
bool Life::shouldUpdateCells() {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
    lastFrameTime = now;
    
    // Cap deltaTime to avoid huge jumps from tab switching
    constexpr float MAX_DELTA_TIME = UPDATE_INTERVAL_SECONDS * 2.0f;
    deltaTime = std::min(deltaTime, MAX_DELTA_TIME);
    
    accumulatedTime += deltaTime;
    
    if (accumulatedTime >= UPDATE_INTERVAL_SECONDS) {
        accumulatedTime -= UPDATE_INTERVAL_SECONDS;
        return true;
    }
    return false;
}