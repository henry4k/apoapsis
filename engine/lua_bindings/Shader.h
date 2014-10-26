#ifndef __APOAPSIS_LUA_BINDINGS_SHADER__
#define __APOAPSIS_LUA_BINDINGS_SHADER__

struct lua_State;
struct Shader;
struct ShaderProgram;
struct ShaderProgramSet;

Shader* GetShaderFromLua( lua_State* l, int stackPosition );
Shader* CheckShaderFromLua( lua_State* l, int stackPosition );

ShaderProgram* GetShaderProgramFromLua( lua_State* l, int stackPosition );
ShaderProgram* CheckShaderProgramFromLua( lua_State* l, int stackPosition );

ShaderProgramSet* GetShaderProgramSetFromLua( lua_State* l, int stackPosition );
ShaderProgramSet* CheckShaderProgramSetFromLua( lua_State* l, int stackPosition );

bool RegisterShaderInLua();

#endif