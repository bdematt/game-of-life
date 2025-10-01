#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"
#include "Life.h"

static constexpr int FPS = 0;
static constexpr bool SIMULATE_INFINITE_LOOP = true;

int main() {
    try {
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
    } catch(const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}