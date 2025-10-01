#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"
#include "Life.h"

static constexpr int FPS = 0;
static constexpr bool SIMULATE_INFINITE_LOOP = true;

int main() {
    Life life {};

    auto renderLoop = [&life]() {
        life.renderFrame();
    };
    
    emscripten_set_main_loop_arg(
        [](void* arg) {
            auto* loop = static_cast<decltype(renderLoop)*>(arg);
            (*loop)();
        },
        &renderLoop,
        FPS,
        SIMULATE_INFINITE_LOOP
    );
    return 0;
}