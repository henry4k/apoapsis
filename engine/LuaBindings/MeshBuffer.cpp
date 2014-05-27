#include "../Lua.h"
#include "../MeshBuffer.h"
#include "Math.h"
#include "MeshBuffer.h"

using namespace glm;


const char* MESH_BUFFER_TYPE = "MeshBuffer";

int Lua_MeshBuffer_destructor( lua_State* l )
{
    MeshBuffer* buffer = CheckMeshBufferFromLua(l, 1);
    FreeMeshBuffer(buffer);
    return 0;
}

int Lua_CreateMeshBuffer( lua_State* l )
{
    MeshBuffer* buffer = new MeshBuffer();
    CopyUserDataToLua(l, MESH_BUFFER_TYPE, sizeof(buffer), &buffer);
    return 1;
}

int Lua_TransformMeshBuffer( lua_State* l )
{
    MeshBuffer* buffer = CheckMeshBufferFromLua(l, 1);
    const mat4* transformation = CheckMatrix4FromLua(l, 2);

    TransformMeshBuffer(buffer, transformation);
    return 0;
}

int Lua_AppendMeshBuffer( lua_State* l )
{
    MeshBuffer* targetBuffer = CheckMeshBufferFromLua(l, 1);
    const MeshBuffer* sourceBuffer = CheckMeshBufferFromLua(l, 2);

    const mat4* transformation = NULL;
    if(lua_gettop(l) >= 3)
        transformation = CheckMatrix4FromLua(l, 3);

    AppendMeshBuffer(targetBuffer, sourceBuffer, transformation);
    return 0;
}

int Lua_AppendIndexToMeshBuffer( lua_State* l )
{
    MeshBuffer* targetBuffer = CheckMeshBufferFromLua(l, 1);
    const VertexIndex index = luaL_checkinteger(l, 2);
    targetBuffer->indices.push_back(index);
    return 0;
}

int Lua_AppendVertexToMeshBuffer( lua_State* l )
{
    MeshBuffer* targetBuffer = CheckMeshBufferFromLua(l, 1);

    int i = 2;
    Vertex vertex;

    vertex.position[0] = luaL_checknumber(l, i++);
    vertex.position[1] = luaL_checknumber(l, i++);
    vertex.position[2] = luaL_checknumber(l, i++);

    vertex.color[0] = luaL_checknumber(l, i++);
    vertex.color[1] = luaL_checknumber(l, i++);
    vertex.color[2] = luaL_checknumber(l, i++);

    vertex.texCoord[0] = luaL_checknumber(l, i++);
    vertex.texCoord[1] = luaL_checknumber(l, i++);

    vertex.normal[0] = luaL_checknumber(l, i++);
    vertex.normal[1] = luaL_checknumber(l, i++);
    vertex.normal[2] = luaL_checknumber(l, i++);

    vertex.tangent[0] = luaL_checknumber(l, i++);
    vertex.tangent[1] = luaL_checknumber(l, i++);
    vertex.tangent[2] = luaL_checknumber(l, i++);
    vertex.tangent[3] = luaL_checknumber(l, i++);

    targetBuffer->vertices.push_back(vertex);
    return 0;
}

bool RegisterMeshBufferInLua()
{
    if(!RegisterUserDataTypeInLua(MESH_BUFFER_TYPE, Lua_MeshBuffer_destructor))
        return false;

    return
        RegisterFunctionInLua("CreateMeshBuffer", Lua_CreateMeshBuffer) &&
        RegisterFunctionInLua("TransformMeshBuffer", Lua_TransformMeshBuffer) &&
        RegisterFunctionInLua("AppendMeshBuffer", Lua_AppendMeshBuffer) &&
        RegisterFunctionInLua("AppendIndexToMeshBuffer", Lua_AppendIndexToMeshBuffer) &&
        RegisterFunctionInLua("AppendVertexToMeshBuffer", Lua_AppendVertexToMeshBuffer);
}

MeshBuffer* GetMeshBufferFromLua( lua_State* l, int stackPosition )
{
    return *(MeshBuffer**)GetUserDataFromLua(l, stackPosition, MESH_BUFFER_TYPE);
}

MeshBuffer* CheckMeshBufferFromLua( lua_State* l, int stackPosition )
{
    return *(MeshBuffer**)CheckUserDataFromLua(l, stackPosition, MESH_BUFFER_TYPE);
}