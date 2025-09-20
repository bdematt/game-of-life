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
        emscripten_sleep(1);
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
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: "; 
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
        emscripten_sleep(1);
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