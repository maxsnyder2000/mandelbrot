////////////////////////////////////////////////////////////////////////
//
//   Harvard Computer Science
//   CS 175: Computer Graphics
//   Professor Steven Gortler
//
////////////////////////////////////////////////////////////////////////

#include <cmath>

#define GLEW_STATIC

#include "GL/glew.h"
#include "GL/glfw3.h"

#include "glsupport.h"

using namespace std;

static bool g_antialiasing = false;
static float g_centerDelta = 0.1;
static float g_centerX = 0.0;
static float g_centerY = 0.0;
static int g_colors = 20;
static int g_framesPerSecond = 30;
static double g_lastFrameClock = 0.0;
static int g_steps = 100;
static int g_threshold = 2;
static int g_windowWidth = 512;
static int g_windowHeight = 512;
static double g_wScale = 1;
static double g_hScale = 1;
static float g_zoom = -1.0;
static float g_zoomDamping = 0.99;
static float g_zoomDelta = 0.01;
static float g_zoomVelocity = 0.0;

GLFWwindow* g_window;

struct SquareShaderState {
    GlProgram program;
    GLint h_uAntialiasing;
    GLint h_uCenterX;
    GLint h_uCenterY;
    GLint h_uColors;
    GLint h_uSteps;
    GLint h_uThreshold;
    GLint h_uWindowX;
    GLint h_uWindowY;
    GLint h_uZoom;
    GLint h_aPosition;
};

static shared_ptr<SquareShaderState> g_squareShaderState;

struct GeometryPX {
    GlArrayObject vao;
    GlBufferObject posVbo;
};

static shared_ptr<GeometryPX> g_square;

static void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(g_square->vao);
    glUseProgram(g_squareShaderState->program);
    safe_glUniform1i(g_squareShaderState->h_uAntialiasing, (int)g_antialiasing);
    safe_glUniform1f(g_squareShaderState->h_uCenterX, g_centerX);
    safe_glUniform1f(g_squareShaderState->h_uCenterY, g_centerY);
    safe_glUniform1i(g_squareShaderState->h_uColors, g_colors);
    safe_glUniform1i(g_squareShaderState->h_uSteps, g_steps);
    safe_glUniform1i(g_squareShaderState->h_uThreshold, g_threshold);
    safe_glUniform1i(g_squareShaderState->h_uWindowX, g_windowWidth);
    safe_glUniform1i(g_squareShaderState->h_uWindowY, g_windowHeight);
    safe_glUniform1f(g_squareShaderState->h_uZoom, g_zoom);
    glBindBuffer(GL_ARRAY_BUFFER, g_square->posVbo);
    safe_glVertexAttribPointer(g_squareShaderState->h_aPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
    safe_glEnableVertexAttribArray(g_squareShaderState->h_aPosition);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    safe_glDisableVertexAttribArray(g_squareShaderState->h_aPosition);
    glBindVertexArray(0);
    checkGlErrors();
    glfwSwapBuffers(g_window);
    checkGlErrors();
}

static void reshape(GLFWwindow * window, const int w, const int h) {
    int width, height;
    glfwGetFramebufferSize(g_window, &width, &height); 
    glViewport(0, 0, width, height);
    g_windowWidth = w;
    g_windowHeight = h;
}

static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_A: {
                g_antialiasing = !g_antialiasing;
                break;
            }
            case GLFW_KEY_LEFT: {
                g_centerX -= g_centerDelta / pow(2, g_zoom);
                break;
            }
            case GLFW_KEY_RIGHT: {
                g_centerX += g_centerDelta / pow(2, g_zoom);
                break;
            }
            case GLFW_KEY_DOWN: {
                g_centerY -= g_centerDelta / pow(2, g_zoom);
                break;
            }
            case GLFW_KEY_UP: {
                g_centerY += g_centerDelta / pow(2, g_zoom);
                break;
            }
            case GLFW_KEY_MINUS: {
                g_colors -= 1;
                break;
            }
            case GLFW_KEY_EQUAL: {
                g_colors += 1;
                break;
            }
            case GLFW_KEY_BACKSPACE: {
                g_zoomVelocity -= g_zoomDelta;
                break;
            }
            case GLFW_KEY_SPACE: {
                g_zoomVelocity += g_zoomDelta;
                if (action == GLFW_PRESS) {
                    g_zoomVelocity += g_zoomDelta;
                }
                break;
            }
            case GLFW_KEY_Q: {
                exit(0);
            }
        }
    }
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void initGlfwState() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    g_window = glfwCreateWindow(g_windowWidth, g_windowHeight, "mandelbrot", NULL, NULL);
    if (!g_window) {
        fprintf(stderr, "Failed to create GLFW window or OpenGL context\n");
        exit(1);
    }
    glfwMakeContextCurrent(g_window);
    glewInit();
    glfwSwapInterval(1);
    glfwSetErrorCallback(error_callback);
    glfwSetWindowSizeCallback(g_window, reshape);
    glfwSetKeyCallback(g_window, keyboard);
    int screen_width, screen_height;
    glfwGetWindowSize(g_window, &screen_width, &screen_height);
    int pixel_width, pixel_height;
    glfwGetFramebufferSize(g_window, &pixel_width, &pixel_height);
    g_wScale = pixel_width / screen_width;
    g_hScale = pixel_height / screen_height;
}

static void initGLState() {
    glClearColor(0, 0, 0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initShaders() {
    g_squareShaderState.reset(new SquareShaderState);
    const GLuint h = g_squareShaderState->program;
    readAndCompileShader(g_squareShaderState->program, "shaders/mandelbrot.vshader", "shaders/mandelbrot.fshader");
    g_squareShaderState->h_uAntialiasing = safe_glGetUniformLocation(h, "uAntialiasing");
    g_squareShaderState->h_uCenterX = safe_glGetUniformLocation(h, "uCenterX");
    g_squareShaderState->h_uCenterY = safe_glGetUniformLocation(h, "uCenterY");
    g_squareShaderState->h_uColors = safe_glGetUniformLocation(h, "uColors");
    g_squareShaderState->h_uSteps = safe_glGetUniformLocation(h, "uSteps");
    g_squareShaderState->h_uThreshold = safe_glGetUniformLocation(h, "uThreshold");
    g_squareShaderState->h_uWindowX = safe_glGetUniformLocation(h, "uWindowX");
    g_squareShaderState->h_uWindowY = safe_glGetUniformLocation(h, "uWindowY");
    g_squareShaderState->h_uZoom = safe_glGetUniformLocation(h, "uZoom");
    g_squareShaderState->h_aPosition = safe_glGetAttribLocation(h, "aPosition");
    glBindFragDataLocation(h, 0, "fragColor");
    checkGlErrors();
}

static void initGeometry() {
    g_square.reset(new GeometryPX());
    GLfloat pos[12] = {-1, -1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1};
    glBindBuffer(GL_ARRAY_BUFFER, g_square->posVbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), pos, GL_STATIC_DRAW);
    checkGlErrors();
}

static void glfwLoop() {
    g_lastFrameClock = glfwGetTime();
    while (!glfwWindowShouldClose(g_window)) {
        double thisTime = glfwGetTime();
        if (thisTime - g_lastFrameClock >= 1. / g_framesPerSecond) {
            g_zoomVelocity *= g_zoomDamping;
            g_zoom += g_zoomVelocity;
            display();
            g_lastFrameClock = thisTime;
        }
        glfwPollEvents();
    }
}

int main(int argc, char **argv) {
    try {
        initGlfwState();
        initGLState();
        initShaders();
        initGeometry();
        glfwLoop();
        return 0;
    } catch (const runtime_error &e) {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }
}
