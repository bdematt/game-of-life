#include "WebGPUContext.h"
#include "webgpu-utils.h"

WebGPUContext::WebGPUContext()
{
    try {
        instance = wgpuCreateInstance({});
        if (!instance) throw InitializationError("Failed to create instance");

        adapter = requestAdapterSync(instance, &ADAPTER_OPTIONS);
        if (!adapter) throw InitializationError("Failed to get adapter");

        device = requestDeviceSync(adapter, {});
        if (!device) throw InitializationError("Failed to get device");

        surface = createSurface(instance);
        if (!surface) throw InitializationError("Failed to create surface");
        
        surfaceConfig.device = device;
        wgpuSurfaceConfigure(surface, &surfaceConfig);

        queue = wgpuDeviceGetQueue(device);
        if (!queue) throw InitializationError("Failed to get queue");
    } catch (...) {
        cleanup();  // Clean up on any failure
        throw;      // Re-throw the exception
    }
}
WebGPUContext::~WebGPUContext()
{
    cleanup();
}
void WebGPUContext::cleanup()
{
    // Release resources in reverse order of creation
    if (queue) {
        wgpuQueueRelease(queue);
        queue = nullptr;
    }
    
    if (surface) {
        wgpuSurfaceRelease(surface);
        surface = nullptr;
    }
    
    if (device) {
        wgpuDeviceRelease(device);
        device = nullptr;
    }
    
    if (adapter) {
        wgpuAdapterRelease(adapter);
        adapter = nullptr;
    }
    
    if (instance) {
        wgpuInstanceRelease(instance);
        instance = nullptr;
    }
}