#include "Common.h"
#include "OpenGL.h"
#include "Squirrel.h"
#include "Model.h"

bool CreateModel( Model* model, const Mesh* mesh )
{
    assert(model);
    assert(mesh);
    assert(mesh->vertexCount > 0);
    assert(mesh->vertices);

    model->primitiveType = GL_TRIANGLES; // Default to triangles (can be changed later)

    glGenBuffers(1, &model->vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, model->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount*sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

    if(mesh->indexCount > 0)
    {
        assert(mesh->indices);

        glGenBuffers(1, &model->indexBuffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexCount*sizeof(unsigned short), mesh->indices, GL_STATIC_DRAW);

        model->size = mesh->indexCount;
    }
    else
    {
        model->size = mesh->vertexCount;
    }

    return true;
}

bool LoadModel( Model* model, const char* mesh )
{
    Mesh m;
    if(!LoadMesh(&m, mesh))
        return false;

    bool r = CreateModel(model, &m);

    FreeMesh(&m);
    return r;
}

void FreeModel( const Model* model )
{
    glDeleteBuffers(1, &model->vertexBuffer);

    if(model->indexBuffer)
        glDeleteBuffers(1, &model->indexBuffer);
}

void DrawModel( const Model* model )
{
    glBindBuffer(GL_ARRAY_BUFFER, model->vertexBuffer);
    SetVertexAttributePointers(NULL);

    if(model->indexBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexBuffer);
        glDrawElements(model->primitiveType, model->size, GL_UNSIGNED_SHORT, 0);
    }
    else
    {
        glDrawArrays(model->primitiveType, 0, model->size);
    }
}



// --- Squirrel Bindings ---

SQInteger OnReleaseModel( void* userData, SQInteger size )
{
    FreeModel((Model*)userData); // Some compilers can't cast pointers directly to smaller data types.
    return 1;
}

SQInteger Squirrel_LoadModel( HSQUIRRELVM vm )
{
    const char* fileName = NULL;
    sq_getstring(vm, 2, &fileName);

	Model* model = (Model*)CreateUserDataInSquirrel(vm, sizeof(Model), OnReleaseModel);
	if(!LoadModel(model, fileName))
	{
		sq_pop(vm, 1);
		return 0;
	}
	else
	{
		return 1;
	}
}
RegisterStaticFunctionInSquirrel(LoadModel, 2, ".s");
