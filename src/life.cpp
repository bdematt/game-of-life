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
}

Life::~Life()
{
    // Clean up WebGPU resources
    if (surface) wgpuSurfaceRelease(surface);
    if (queue) wgpuQueueRelease(queue);
    if (device) wgpuDeviceRelease(device);
    if (adapter) wgpuAdapterRelease(adapter);
    if (instance) wgpuInstanceRelease(instance);

    std::cout << "🔧 Life destroyed" << std::endl;
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
    colorAttachment.clearValue = {0.0f, 0.0f, 0.4f, 1.0f};
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    WGPURenderPassDescriptor passDesc = {};
    passDesc.label = WGPUStringView{"Life Render Pass", 16};
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;

    // Begin render pass
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
    
    // End render pass
    wgpuRenderPassEncoderEnd(pass);

    // Finish command encoder
    WGPUCommandBufferDescriptor cmdDesc = {};
    cmdDesc.label = WGPUStringView{"Life Commands", 13};
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &cmdDesc);

    // Submit command buffer to queue
    wgpuQueueSubmit(queue, 1, &commandBuffer);

    // Clean up resources
    wgpuTextureViewRelease(view);
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuRenderPassEncoderRelease(pass);
}