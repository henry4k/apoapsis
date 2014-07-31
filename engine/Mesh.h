#ifndef __APOAPSIS_MESH__
#define __APOAPSIS_MESH__

#include "OpenGL.h"


struct MeshBuffer;
struct Mesh;


Mesh* CreateMesh( const MeshBuffer* buffer );
void DrawMesh( const Mesh* mesh );

void ReferenceMesh( Mesh* mesh );
void ReleaseMesh( Mesh* mesh );

#endif
