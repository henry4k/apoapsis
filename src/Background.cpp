#include "OpenGL.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "Background.h"

GLuint  g_SkyboxShader;
GLuint  g_SkyboxTexture;
Model   g_SkyboxModel;

bool InitBackground()
{
    g_SkyboxShader = LoadShader("Shaders/Skybox.vert", "Shaders/Skybox.frag");
	if(!g_SkyboxShader)
		return false;

    g_SkyboxTexture = LoadCubeTexture(TEX_MIPMAP|TEX_FILTER, "Textures/Debug/%s.png");
	if(!g_SkyboxTexture)
		return false;

    if(!LoadModel(&g_SkyboxModel, "Meshes/Skybox.ply"))
		return false;

	return true;
}

void FreeBackground()
{
    FreeModel(&g_SkyboxModel);
    FreeTexture(g_SkyboxTexture);
    FreeShader(g_SkyboxShader);
}

void DrawBackground()
{
    glDepthMask(false);
    glPushMatrix();
    glScalef(25,25,25);

    BindShader(g_SkyboxShader);
    BindTexture(GL_TEXTURE_CUBE_MAP, g_SkyboxTexture, 0);
    DrawModel(&g_SkyboxModel);

    glPopMatrix();
    glDepthMask(true);
}
