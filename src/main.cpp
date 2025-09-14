#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include "webgpu-utils.h"
#include "Life.h"
#include "WebGPUContext.h"

// Globals
std::unique_ptr<Life> g_life;

void main_loop() {
    g_life->tick();
}

int main() {
    std::cout << "🚀 Starting WebGPU application..." << std::endl;
    try {
        g_life = std::make_unique<Life>();

        emscripten_set_main_loop(main_loop, 0, 1);
        
        return 0;
    }
    catch (const WebGPUContext::InitializationError& e) {
        std::cerr << "❌ " << e.what() << std::endl;
        return -1;
    }
    return 0;
}