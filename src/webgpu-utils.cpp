#include <emscripten.h>
#include "webgpu-utils.h"
#include <cassert>
#include <iostream>

/**
 * Utility function to get a WebGPU adapter
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options)
{
    // A simple structure holding the local information shared with the
    // onAdapterRequestEnded callback.
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // Callback called by wgpuInstanceRequestAdapter when the request returns
    auto onAdapterRequestEnded = [](
        WGPURequestAdapterStatus status,
        WGPUAdapter adapter, 
        WGPUStringView message,
        void* userdata1,
        void* userdata2
    ) {
        UserData& userData = *reinterpret_cast<UserData*>(userdata1);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: ";
            if (message.data && message.length > 0) {
                std::cout << std::string(message.data, message.length);
            }
            std::cout << std::endl;
        }
        userData.requestEnded = true;
    };

    // Create the callback info structure for the new API
    WGPURequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.callback = onAdapterRequestEnded;
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.nextInChain = nullptr;
    callbackInfo.userdata1 = &userData;  // Pass userData as userdata1
    callbackInfo.userdata2 = nullptr;

    // Call to the WebGPU request adapter procedure with new API
    wgpuInstanceRequestAdapter(instance, options, callbackInfo);

    while (!userData.requestEnded) {
        emscripten_sleep(1); // Sleep for 1ms to yield control
    }

    assert(userData.requestEnded);

    return userData.adapter;
}
/**
 * Utility function to get a WebGPU device synchronously
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor)
{
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // Callback called when the device request returns
    auto onDeviceRequestEnded = [](
        WGPURequestDeviceStatus status,
        WGPUDevice device, 
        WGPUStringView message,
        void* userdata1,
        void* userdata2
    ) {
        UserData& userData = *reinterpret_cast<UserData*>(userdata1);
        if (status == WGPURequestDeviceStatus_Success) {  // Fixed: Use WGPURequestDeviceStatus_Success
            userData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: ";  // Fixed: Say "device" not "adapter"
            if (message.data && message.length > 0) {
                std::cout << std::string(message.data, message.length);
            }
            std::cout << std::endl;
        }
        userData.requestEnded = true;
    };

    // Create the callback info structure for the new API (same pattern as adapter)
    WGPURequestDeviceCallbackInfo callbackInfo = {};
    callbackInfo.callback = onDeviceRequestEnded;
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.nextInChain = nullptr;
    callbackInfo.userdata1 = &userData;
    callbackInfo.userdata2 = nullptr;

    // Call the WebGPU request device procedure with new API
    wgpuAdapterRequestDevice(adapter, descriptor, callbackInfo);

    // Wait for completion
    while (!userData.requestEnded) {
        #ifdef __EMSCRIPTEN__
        emscripten_sleep(1);  // 1ms is usually enough, 100ms might feel sluggish
        #endif
    }

    assert(userData.requestEnded);

    return userData.device;
}
/**
 * Utility function to create a WebGPU surface
 */
WGPUSurface createSurface(WGPUInstance instance)
{
    std::cout << "🔍 Creating surface..." << std::endl;

    struct CanvasSelectorChain {
        WGPUChainedStruct chain;
        const char *selector;
    };

    CanvasSelectorChain canvasChain = {};
    canvasChain.chain.next = nullptr;
    canvasChain.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasChain.selector = "#canvas";

    WGPUSurfaceDescriptor surfaceDesc = {};
    surfaceDesc.label = WGPUStringView{"Canvas Surface", 14};
    surfaceDesc.nextInChain = &canvasChain.chain;

    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}

/**
 * Utility function to create a WebGPU surface
 */
void configureSurface(WGPUDevice device, WGPUSurface surface)
{
    std::cout << "🔍 Configuring surface..." << std::endl;

    if (!surface) {
        std::cout << "❌ Cannot configure - surface is null!" << std::endl;
        return;
    }

    WGPUSurfaceConfiguration config = {};
    config.device = device;
    config.format = WGPUTextureFormat_BGRA8Unorm;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.width = 800;   // Use class member variables
    config.height = 600; // Use class member variables
    config.presentMode = WGPUPresentMode_Fifo;

    wgpuSurfaceConfigure(surface, &config);
    
    std::cout << "✅ Surface configured for " << config.width << "x" << config.height << std::endl;
}