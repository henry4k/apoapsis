#ifndef __APOAPSIS_RENDER_MANAGER__
#define __APOAPSIS_RENDER_MANAGER__

#include "Math.h"
#include "Texture.h"
#include "Shader.h"

struct Mesh;
struct Model;

bool InitRenderManager();
void DestroyRenderManager();
void DrawModels( glm::mat4 mvpMatrix );

Model* CreateModel( ShaderProgram* program );
void FreeModel( Model* model );
void SetModelTransformation( Model* model, glm::mat4 transformation );
void SetModelMesh( Model* model, Mesh* mesh );

void SetModelUniform( Model* model, const char* name, UniformValue* value );
void UnsetModelUniform( Model* model, const char* name );

#endif