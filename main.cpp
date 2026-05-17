#include <GL/glew.h> // MUST be included before freeglut
#include <GL/freeglut.h>
#include <iostream>

void onKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        glutLeaveMainLoop();
        break;
    default:
        break;
    }
}

void idle()
{
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    GLclampf red = (GLclampf)(milliseconds % 1000) / 1000;
    glClearColor(red, 0.0, 0.0, 1.0);
    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    // init GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(400, 300);
    glutInitContextVersion(4, 6);
    glutInitContextFlags(GLUT_DEBUG);
    glutCreateWindow("OpenGL GLUT Example");

    // IMPORTANT: You must initialize GLEW after the window is created!
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    glutKeyboardFunc(onKeyboard);
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}