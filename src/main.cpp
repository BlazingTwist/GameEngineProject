#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/graphics/resources.hpp>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <iostream>

#endif
#endif

using namespace std::chrono_literals;

graphics::GeometryBuffer *createStripBuffer(const graphics::VertexAttribute *vertexAttribute, int numAttributes) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLE_STRIPE,
            vertexAttribute,
            numAttributes,
            0
    );
    return buffer;
}

graphics::GeometryBuffer *createTriangleBuffer(const graphics::VertexAttribute *vertexAttribute, int numAttributes) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLES,
            vertexAttribute,
            numAttributes,
            0
    );
    return buffer;
}

int main(int argc, char *argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //	_CrtSetBreakAlloc(2760);
#endif
#endif

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    graphics::Device::initialize(1366, 1366, false);
    GLFWwindow *window = graphics::Device::getWindow();
    input::InputManager::initialize(window);

    const std::array vertexAttribute = {
            graphics::VertexAttribute{graphics::PrimitiveFormat::FLOAT, 3, false, false}
    };

    std::array<glm::vec3, 3> triangles[] = {
            {
                    glm::vec3(-0.1f, -0.1f, 0.0f),
                    glm::vec3(-0.9f, -0.1f, 0.0f),
                    glm::vec3(-0.1f, -0.9f, 0.0f),
            },
            {
                    glm::vec3(0.1f, 0.1f, 0.0f),
                    glm::vec3(0.9f, 0.1f, 0.0f),
                    glm::vec3(0.1f, 0.9f, 0.0f),
            }
    };

    std::vector<graphics::GeometryBuffer> buffers = {};

    for (int i = 0; i < 1; ++i) {
        buffers.clear();
        
        for (const auto &triangle : triangles){
            graphics::GeometryBuffer *buffer = createTriangleBuffer(vertexAttribute.data(), vertexAttribute.size());
            buffer->setData(triangle.data(), triangle.size() * sizeof (glm::vec3));
            buffers.push_back(*buffer);
        }
    }

    graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
    graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

    graphics::Program program;
    program.attach(vertexShader);
    program.attach(fragmentShader);
    program.link();
    program.use();

    graphics::glCall(glClearColor, 0.f, 1.f, 0.f, 1.f);

    while (!glfwWindowShouldClose(window) && !input::InputManager::isKeyPressed(input::Key::ESCAPE)) {
        graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto &buffer: buffers) {
            buffer.draw();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(16ms);
    }

    utils::MeshLoader::clear();
    graphics::Device::close();
    return EXIT_SUCCESS;
}