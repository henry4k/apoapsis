#ifndef __VERTEX__
#define __VERTEX__

#include "Math.h"
#include "OpenGL.h"

typedef unsigned short VertexIndex;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 tangent;
};

void EnableVertexArrays();
void BindVertexAttributes( GLuint programHandle );
void SetVertexAttributePointers( const void* data );


struct DebugVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

#endif
