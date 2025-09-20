#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <webgpu/webgpu.h>

class WebGPUContext
{
public:
    WebGPUContext();
    ~WebGPUContext();

    // WebGPU Context Getters
    WGPUInstance getInstance() const { return instance; }
    WGPUAdapter getAdapter() const { return adapter; }
    WGPUDevice getDevice() const { return device; }
    WGPUSurface getSurface() const { return surface; }
    WGPUQueue getQueue() const { return queue; }
    WGPUSurfaceTexture getCurentSurfaceTexture() const {
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(getSurface(), &surfaceTexture);
        return surfaceTexture;
    }

    // Surface configuration getters
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    const WGPUSurfaceConfiguration& getSurfaceConfig() const { return surfaceConfig; }

    WGPUTextureView createCurrentTextureView(WGPUTexture texture) const {
        return wgpuTextureCreateView(texture, &TEXTURE_VIEW_DESC);
    }

private:
    // WebGPU Context
    WGPUInstance instance = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUSurface surface = nullptr;
    WGPUQueue queue = nullptr;
    
    // Surface Config
    uint32_t width = 800;
    uint32_t height = 800;

    // Descriptors, Options, Configs
    static constexpr WGPURequestAdapterOptions ADAPTER_OPTIONS = {
        .powerPreference = WGPUPowerPreference_HighPerformance
    };
    static constexpr WGPUTextureViewDescriptor TEXTURE_VIEW_DESC = {
        .format = WGPUTextureFormat_BGRA8Unorm,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1
    };
    WGPUSurfaceConfiguration surfaceConfig = {
        .format = WGPUTextureFormat_BGRA8Unorm,
        .usage = WGPUTextureUsage_RenderAttachment,
        .width = 800,
        .height = 600,
        .presentMode = WGPUPresentMode_Fifo
    };

    void cleanup();

};