#include "Common.h"
#include "OpenGL.h"
#include "Window.h"


/****** Variables *******/

GLFWwindow* g_Window = NULL;
int g_WindowWidth  = 0;
int g_WindowHeight = 0;
int g_FramebufferWidth  = 0;
int g_FramebufferHeight = 0;
double g_LastCursorX = 0;
double g_LastCursorY = 0;
FramebufferResizeFn g_FramebufferResizeFn = NULL;
CursorMoveFn        g_CursorMoveFn = NULL;
MouseButtonActionFn g_MouseButtonActionFn = NULL;
MouseScrollFn       g_MouseScrollFn = NULL;
KeyActionFn         g_KeyActionFn = NULL;


/****** General *******/

void OnGLFWError( int code, const char* description );
void OnDebugEvent( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, void* userParam );
void OnWindowResize( GLFWwindow* window, int width, int height );
void OnFramebufferResize( GLFWwindow* window, int width, int height );
void OnMouseButtonAction( GLFWwindow* window, int button, int action, int mods );
void OnMouseScroll( GLFWwindow* window, double xoffset, double yoffset );
void OnCursorMove( GLFWwindow* window, double x, double y );
void OnKeyAction( GLFWwindow* window, int key, int scancode, int action, int mods );

bool InitWindow( int width, int height, const char* title )
{
    assert(g_Window == NULL);
    glfwSetErrorCallback(OnGLFWError);
    if(!glfwInit())
	{
        Error("GLFW init failed.");
		return false;
	}

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    g_Window = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!g_Window)
	{
        Error("Window creation failed.");
		return false;
	}

    glfwGetWindowSize(g_Window, &g_WindowWidth, &g_WindowHeight);
    glfwGetFramebufferSize(g_Window, &g_FramebufferWidth, &g_FramebufferHeight);
    glfwGetCursorPos(g_Window, &g_LastCursorX, &g_LastCursorY);
    g_LastCursorX *= g_FramebufferWidth / g_WindowWidth;
    g_LastCursorX *= g_FramebufferHeight / g_WindowHeight;

    glfwMakeContextCurrent(g_Window);

#if defined(_WIN32)
    if(glfwExtensionSupported("WGL_EXT_swap_control_tear"))
#else
    if(glfwExtensionSupported("GLX_EXT_swap_control_tear"))
#endif
        glfwSwapInterval(-1); // enable vsync (allow the driver to swap even if a frame arrives a little bit late)
    else
        glfwSwapInterval(1); // enable vsync

    glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(g_Window, GLFW_STICKY_KEYS, GL_FALSE);
    glfwSetInputMode(g_Window, GLFW_STICKY_MOUSE_BUTTONS, GL_FALSE);

    if(GLEW_ARB_seamless_cube_map)
    {
        Log("Seamless cubemap filtering supported");
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    if(GLEW_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(OnDebugEvent, NULL);
        Log("Debug output supported! You may receive debug messages from your OpenGL driver.");
    }

    glfwSetWindowSizeCallback(g_Window, OnWindowResize);
    glfwSetFramebufferSizeCallback(g_Window, OnFramebufferResize);
    glfwSetMouseButtonCallback(g_Window, OnMouseButtonAction);
    glfwSetScrollCallback(g_Window, OnMouseScroll);
    glfwSetCursorPosCallback(g_Window, OnCursorMove);
    glfwSetKeyCallback(g_Window, OnKeyAction);

    return true;
}

void FreeWindow()
{
    assert(g_Window != NULL);
    glfwDestroyWindow(g_Window);
    glfwTerminate();
}

void SwapBuffers()
{
    glfwSwapBuffers(g_Window);
    glfwPollEvents();
}

void SetWindowTitle( const char* title )
{
    glfwSetWindowTitle(g_Window, title);
}

void FlagWindowForClose()
{
    glfwSetWindowShouldClose(g_Window, GL_TRUE);
}

bool WindowShouldClose()
{
    return glfwWindowShouldClose(g_Window) == GL_TRUE;
}

void* GetGLFWwindow()
{
    return g_Window;
}


/****** Input ******/

bool IsKeyPressed( int key )
{
    return glfwGetKey(g_Window, key) != GLFW_RELEASE;
}

bool IsMouseButtonPressed( int button )
{
    return glfwGetMouseButton(g_Window, button) != GLFW_RELEASE;
}

double GetCursorX()
{
    return g_LastCursorX;
}

double GetCursorY()
{
    return g_LastCursorY;
}

void SetCursorMode( int mode )
{
    glfwSetInputMode(g_Window, GLFW_CURSOR, mode);
}


/******* Callback Setter *******/

void SetFrambufferFn( FramebufferResizeFn fn )
{
    g_FramebufferResizeFn = fn;
    OnFramebufferResize(NULL, g_FramebufferWidth, g_FramebufferHeight);
}

void SetCursorMoveFn( CursorMoveFn fn )
{
    g_CursorMoveFn = fn;
}

void SetMouseButtonActionFn( MouseButtonActionFn fn )
{
    g_MouseButtonActionFn = fn;
}

void SetMouseScrollFn( MouseScrollFn fn )
{
    g_MouseScrollFn = fn;
}

void SetKeyActionFn( KeyActionFn fn )
{
    g_KeyActionFn = fn;
}


/******* Callbacks ********/

void OnGLFWError( int code, const char* description )
{
    Error("GLFW error %d: %s", code, description);
}

void OnDebugEvent( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, void* userParam )
{
    const char* sourceName = "";
    switch(source)
    {
        case GL_DEBUG_SOURCE_API_ARB: sourceName = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: sourceName = "window system"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: sourceName = "shader"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: sourceName = "third party"; break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB: sourceName = "application"; break;
        case GL_DEBUG_SOURCE_OTHER_ARB: sourceName = ""; break;
    }

    const char* typeName = "unknown issue";
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR_ARB: typeName = "error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: typeName = "deprecated behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: typeName = "undefined behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB: typeName = "partability issue"; break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB: typeName = "performance issue"; break;
        case GL_DEBUG_TYPE_OTHER_ARB: typeName = "unknown issue"; break;
    }

    const char* severityName = "";
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB: severityName = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB: severityName = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW_ARB: severityName = "Low"; break;
    }

    Error("%s severity %s %s %d, %s\n", severityName, sourceName, typeName, id, message);
}

void OnWindowResize( GLFWwindow* window, int width, int height )
{
    Log("window resize %dx%d", width, height);
    g_WindowWidth = width;
    g_WindowHeight = height;
    // TODO: Update cursor position
}

void OnFramebufferResize( GLFWwindow* window, int width, int height )
{
    Log("framebuffer resize %dx%d", width, height);
    g_FramebufferWidth = width;
    g_FramebufferHeight = height;
    // TODO: Update cursor position
    if(g_FramebufferResizeFn)
        g_FramebufferResizeFn(width, height);
}

void OnMouseButtonAction( GLFWwindow* window, int button, int action, int mods )
{
    if(g_MouseButtonActionFn)
        g_MouseButtonActionFn(button, action==GLFW_PRESS);
}

void OnMouseScroll( GLFWwindow* window, double xoffset, double yoffset )
{
    if(g_MouseScrollFn)
        g_MouseScrollFn(xoffset, yoffset);
}

void OnCursorMove( GLFWwindow* window, double x, double y )
{
    x *= g_FramebufferWidth / g_WindowWidth;
    y *= g_FramebufferHeight / g_WindowHeight;

    const double dx = x - g_LastCursorX;
    const double dy = y - g_LastCursorY;

    g_LastCursorX = x;
    g_LastCursorY = y;

    if(g_CursorMoveFn)
        g_CursorMoveFn(x, y, dx, dy);
}

void OnKeyAction( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if(action != GLFW_REPEAT && g_KeyActionFn)
        g_KeyActionFn(key, scancode, action==GLFW_PRESS);
}
