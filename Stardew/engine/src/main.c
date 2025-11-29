#include "main.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "DynArray.h"
#include "GameFramework.h"
#include "XMLUIGameLayer.h"
#include "ImageFileRegstry.h"
#include "Atlas.h"
#include "Widget.h"
#include "Scripting.h"
#include <string.h>
#include "PlatformDefs.h"
#include <libxml/parser.h>
#include "Log.h"

#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define TARGET_FPS 60

InputContext gInputContext;
DrawContext gDrawContext;
struct CommandLineArgs gCmdArgs;

DrawContext* GetDrawContext()
{
    return &gDrawContext;
}

InputContext* GetInputContext()
{
    return &gInputContext;
}

int Mn_GetScreenWidth()
{
    return gDrawContext.screenWidth;
}

int Mn_GetScreenHeight()
{
    return gDrawContext.screenHeight;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Dr_OnScreenDimsChange(&gDrawContext, width, height);
    In_FramebufferResize(&gInputContext, width, height);
    GF_OnWindowDimsChanged(width, height);
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    In_RecieveMouseMove(&gInputContext, xposIn, yposIn);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    In_RecieveScroll(&gInputContext, xoffset, yoffset);
}

void MouseBtnCallback(GLFWwindow* window, int button, int action, int mods)
{
    In_RecieveMouseButton(&gInputContext, button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    In_RecieveKeyboardKey(&gInputContext, key, scancode, action, mods);
}

void joystick_callback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        In_SetControllerPresent(jid);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        In_SetControllerPresent(-1);
    }
}


static void GLAPIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    //if (severity >= minimumLogSeverityIncluding) 
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    }
}

typedef void(*GameInitFn)(InputContext*,DrawContext*);

static void ParseCmdArgs(int argc, char** argv)
{
    gCmdArgs.role = GR_Singleplayer;
    gCmdArgs.serverAddress = "127.0.0.1:40000";
    gCmdArgs.clientAddress = "0.0.0.0";
    if(argc > 1)
    {
        for(int i=0; i <argc; i++)
        {
            if(strcmp(argv[i], "--role") == 0 || strcmp(argv[i], "-r") == 0)
            {
                assert(i + 1 < argc);
                i++;
                if(strcmp(argv[i], "server") == 0 || strcmp(argv[i], "s") == 0)
                {
                    gCmdArgs.role = GR_ClientServer;
                }
                if(strcmp(argv[i], "client") == 0 || strcmp(argv[i], "c") == 0)
                {
                    gCmdArgs.role = GR_Client;
                }
            }
            else if(strcmp(argv[i], "--server_address" == 0) || strcmp(argv[i], "-s" == 0))
            {
                assert(i + 1 < argc);
                i++;
                gCmdArgs.serverAddress = argv[i + 1];
            }
            else if(strcmp(argv[i], "--client_address" == 0) || strcmp(argv[i], "-c" == 0))
            {
                assert(i + 1 < argc);
                i++;
                gCmdArgs.serverAddress = argv[i + 1];
            }
        }
    }
}

int EngineStart(int argc, char** argv, GameInitFn init)
{
    Log_Init();

    Log_Verbose("testing libxml version...\n");
    LIBXML_TEST_VERSION
    Log_Verbose("hello world\n");
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    Log_Verbose("glfwInit\n");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Stardew Engine", NULL, NULL);
    if (window == NULL)
    {
        /*std::cout << "Failed to create GLFW window" << std::endl;*/
        Log_Error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync
    
    glfwJoystickPresent(GLFW_JOYSTICK_1);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseBtnCallback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    Log_Verbose("loading Opengl procs\n");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
    Log_Verbose("loading Opengl ES procs\n");
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
#endif


    // configure global opengl state
    // -----------------------------
    Log_Verbose("configuring global opengl state\n");
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // During init, enable debug output
    glEnable(GL_DEBUG_OUTPUT);

#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    glDebugMessageCallback(MessageCallback, 0);
#endif
    Log_Verbose("done\n");

    double accumulator = 0;
    double lastUpdate = 0;
    double slice = 1.0 / TARGET_FPS;

    Log_Verbose("initialising draw context\n");
    gDrawContext = Dr_InitDrawContext();
    Log_Verbose("done\n");
    Log_Verbose("initial screen dims change\n");
    Dr_OnScreenDimsChange(&gDrawContext, SCR_WIDTH, SCR_HEIGHT);
    Log_Verbose("done\n");
    Log_Verbose("initialising input context\n");
    gInputContext = In_InitInputContext();
    Log_Verbose("done\n");
    Log_Verbose("Initialising game framework\n");
    GF_InitGameFramework();
    Log_Verbose("done\n");
    Log_Verbose("initialising image registry\n");
    IR_InitImageRegistry(NULL);
    Log_Verbose("done\n");
    Log_Verbose("initialising atlas\n");
    At_Init();
    Log_Verbose("done\n");
    Log_Verbose("initialising UI\n");
    UI_Init();
    Log_Verbose("done\n");
    Log_Verbose("initialising scripting\n");
    Sc_InitScripting();
    Log_Verbose("done\n");

    init(&gInputContext, &gDrawContext);
    
    double frameTimeTotal = 0.0;
    int onCount = 0;
    int numCounts = 60;
    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double delta = time - lastUpdate;
        lastUpdate = time;
        accumulator += delta;
        while (accumulator > slice)
        {
            glfwPollEvents();
            GF_InputGameFramework(&gInputContext);
            GF_UpdateGameFramework((float)slice);
            In_EndFrame(&gInputContext);
            GF_EndFrame(&gDrawContext, &gInputContext);
            accumulator -= slice;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GF_DrawGameFramework(&gDrawContext);
        glfwSwapBuffers(window);
        
        frameTimeTotal += delta;
        onCount++;
        if(onCount == numCounts)
        {
            onCount = 0;
            //printf("frame time: %f\n", 1.0 / (frameTimeTotal / (double)numCounts));
            frameTimeTotal = 0.0f;
        }
    }

    Sc_DeInitScripting();
    IR_DestroyImageRegistry();
    GF_DestroyGameFramework();

    glfwTerminate();
    Log_DeInit();
}



void GameInit(InputContext* pIC, DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    options.bLoadImmediately = true;
    options.xmlPath = "./Assets/test.xml";
    options.pDc = pDC;
    Log_Verbose("making xml ui layer\n");
    XMLUIGameLayer_Get(&testLayer, &options);
    Log_Verbose("done\n");
    Log_Verbose("pushing framework layer\n");
    GF_PushGameFrameworkLayer(&testLayer);
    Log_Verbose("done\n");
}
