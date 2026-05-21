#include <GL/glew.h> // MUST be included before freeglut
#include <GL/freeglut.h>
#include <iostream>
#include <memory>
#include "renderer/RayTracer.hpp"
#include "scene/Scene.hpp"

#define RT_DEFAULT_WIDTH 640
#define RT_DEFAULT_HEIGHT 360

using namespace std::literals;

// Global pointer to the RayTracer instance
std::unique_ptr<RayTracer> rayTracer;
std::unique_ptr<Scene> scene;

/**
 * @brief Callback for keyboard events.
 * Handles ASDWQE keys for camera movement.
 *
 * @param key The key pressed.
 * @param x The x-coordinate of the mouse.
 * @param y The y-coordinate of the mouse.
 */
void onKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // Escape key
        glutLeaveMainLoop();
        break;
    case 'w':
    case 'W':
        scene->moveCamera(CameraMovement::FORWARD);
        break;
    case 's':
    case 'S':
        scene->moveCamera(CameraMovement::BACKWARD);
        break;
    case 'a':
    case 'A':
        scene->moveCamera(CameraMovement::LEFT);
        break;
    case 'd':
    case 'D':
        scene->moveCamera(CameraMovement::RIGHT);
        break;
    case 'q':
    case 'Q':
        scene->moveCamera(CameraMovement::DOWN);
        break;
    case 'e':
    case 'E':
        scene->moveCamera(CameraMovement::UP);
        break;
    default:
        break;
    }
}

/**
 * @brief Callback for special keyboard events.
 * Handles arrow keys for camera rotation.
 *
 * @param key The special key pressed.
 * @param x The x-coordinate of the mouse.
 * @param y The y-coordinate of the mouse.
 */
void onSpecialKeyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        scene->rotateCamera(0.0f, 1.0f);
        break;
    case GLUT_KEY_DOWN:
        scene->rotateCamera(0.0f, -1.0f);
        break;
    case GLUT_KEY_LEFT:
        scene->rotateCamera(-1.0f, 0.0f);
        break;
    case GLUT_KEY_RIGHT:
        scene->rotateCamera(1.0f, 0.0f);
        break;
    default:
        break;
    }
}

/**
 * @brief Idle callback for continuous rendering.
 * Dispatches the compute shader and requests a redisplay.
 */
void idle()
{
    scene->sendData();
    rayTracer->dispatchCompute();
    glutPostRedisplay();
}

/**
 * @brief Display callback to render the result on screen.
 */
void display()
{
    rayTracer->displayScreen();
    glutSwapBuffers();
}

/**
 * @brief Callback for window resizing.
 *
 * @param w New window width.
 * @param h New window height.
 */
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    rayTracer->setWindowSize(w, h);
    scene->setCameraAspectRatio(w, h);
}

/**
 * @brief Entry point of the application.
 */
int main(int argc, char **argv)
{
    // Initialize GLUT for window management and event handling
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(RT_DEFAULT_WIDTH, RT_DEFAULT_HEIGHT);
    glutInitContextVersion(4, 6);
    glutInitContextFlags(GLUT_DEBUG);
    glutCreateWindow("Ray Tracer Bridge");

    // IMPORTANT: You must initialize GLEW after the window is created!
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // init GL interface
    rayTracer = std::make_unique<RayTracer>("shader/ray_tracing.glsl"s, RT_DEFAULT_WIDTH, RT_DEFAULT_HEIGHT);
    scene = std::make_unique<Scene>(rayTracer->getShaderProgram());

    // register events
    glutKeyboardFunc(onKeyboard);
    glutSpecialFunc(onSpecialKeyboard);
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    // start the rendering loop
    glutMainLoop();
    return 0;
}
