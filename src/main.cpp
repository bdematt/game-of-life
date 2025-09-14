#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <memory>
#include <utility>
#include "webgpu-utils.h"
#include "life.h"
#include "WebGPUContext.h"

// Globals
std::unique_ptr<Life> g_life;

void main_loop() {
    g_life->tick();
}

int main() {
    std::cout << "🚀 Starting WebGPU application..." << std::endl;
    try {
        auto context = std::make_unique<WebGPUContext>();
        g_life = std::make_unique<Life>(std::move(context));

        emscripten_set_main_loop(main_loop, 0, 1);
        
        return 0;
    }
    catch (const WebGPUContext::InitializationError& e) {
        std::cerr << "❌ " << e.what() << std::endl;
        return -1;
    }
    return 0;
}