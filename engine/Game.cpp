#include "Common.h"
#include "Math.h"
#include "Config.h"
#include "Window.h"
#include "Controls.h"
#include "OpenGL.h"
#include "Vertex.h"
#include "Audio.h"
#include "Player.h"
#include "Math.h"
#include "Lua.h"
#include "PhysicsManager.h"
#include "RenderManager.h"
#include "ModelManager.h"
#include "Game.h"

#include "LuaBindings/Audio.h"
#include "LuaBindings/Config.h"
#include "LuaBindings/Controls.h"
#include "LuaBindings/Math.h"
#include "LuaBindings/Mesh.h"
#include "LuaBindings/MeshBuffer.h"
#include "LuaBindings/Player.h"
#include "LuaBindings/RenderManager.h"
#include "LuaBindings/ModelManager.h"
#include "LuaBindings/PhysicsManager.h"
#include "LuaBindings/Shader.h"
#include "LuaBindings/Texture.h"


static void OnExitKey( const char* name, bool pressed, void* context );
static bool RegisterAllModulesInLua();

bool InitGame( const int argc, char** argv )
{
    Log("----------- Config ------------");
    if(!InitConfig(argc, argv))
        return false;

    Log("------------- Lua -------------");
    if(!InitLua())
        return false;

    Log("----------- Window ------------");
    if(!InitWindow())
        return false;

    Log("------------ Audio ------------");
    if(!InitAudio())
        return false;

    Log("---------- Controls -----------");
    if(!InitControls())
        return false;

    if(!RegisterKeyControl("exit", OnExitKey, NULL, NULL))
        return false;

    Log("--------- Physics Manager ----------");
    if(!InitPhysicsManager())
        return false;

    Log("--------- Render Manager ----------");
    if(!InitRenderManager())
        return false;

    Log("--------- Model Manager ----------");
    if(!InitModelManager())
        return false;

    Log("----------- Player ------------");
    if(!InitPlayer())
        return false;

    Log("--- Registering Lua modules ----");
    if(!RegisterAllModulesInLua())
        return false;

    Log("-------------------------------");

    return RunLuaScript(GetLuaState(), "core/init.lua");
}

static bool RegisterAllModulesInLua()
{
    return
        RegisterAudioInLua() &&
        RegisterConfigInLua() &&
        RegisterControlsInLua() &&
        RegisterMathInLua() &&
        RegisterMeshInLua() &&
        RegisterMeshBufferInLua() &&
        RegisterPlayerInLua() &&
        RegisterRenderManagerInLua() &&
        RegisterModelManagerInLua() &&
        RegisterPhysicsManagerInLua() &&
        RegisterShaderInLua() &&
        RegisterTextureInLua();
}

void DestroyGame()
{
    DestroyLua();
    DestroyPlayer();
    DestroyModelManager();
    DestroyRenderManager();
    DestroyPhysicsManager();
    DestroyControls();
    DestroyAudio();
    DestroyWindow();
    DestroyConfig();
}

void RunGame()
{
    using namespace glm;

    double lastTime = glfwGetTime();
    while(!WindowShouldClose())
    {
        // Simulation
        const double curTime = glfwGetTime();
        const double timeDelta = curTime-lastTime;
        //UpdateLua();
        UpdateAudio();
        UpdateControls(timeDelta);
        UpdatePlayer(timeDelta);
        UpdatePhysicsManager(timeDelta);
        lastTime = curTime;

        SetCameraViewTransformation(GetPlayerViewMatrix());
        RenderScene();
    }
}

static void OnExitKey( const char* name, bool pressed, void* context )
{
    if(pressed)
        FlagWindowForClose();
}

