#include <string.h> // memset
#include <stdlib.h> // realloc, free

#include "Common.h"
#include "List.h"
#include "Mesh.h"
#include "MeshBuffer.h"
#include "Reference.h"
#include "VoxelVolume.h"
#include "MeshChunkGenerator.h"

#include <glm/gtc/matrix_transform.hpp> // translate, rotate


using namespace glm;


static const int MAX_VOXEL_MESHES = 8;

enum VoxelMeshType
{
    BLOCK_VOXEL_MESH
};

enum MooreNeighbourhood
{
    MOORE_POSITIVE_X = 1<<0,
    MOORE_NEGATIVE_X = 1<<1,
    MOORE_POSITIVE_Y = 1<<2,
    MOORE_NEGATIVE_Y = 1<<3,
    MOORE_POSITIVE_Z = 1<<4,
    MOORE_NEGATIVE_Z = 1<<5
};


struct BlockVoxelMesh
{
    bool transparent;
    MeshBuffer* meshBuffers[BLOCK_VOXEL_MATERIAL_BUFFER_COUNT];
    char    transformations[BLOCK_VOXEL_MATERIAL_BUFFER_COUNT*sizeof(mat4)];
    // TODO: mat4 is a class, which would prevent this to be used in unions.
    // Therefore we need to use this dirty hack here.  :/
};

struct VoxelMesh
{
    VoxelMeshType type;
    int materialId;
    union data_
    {
        BlockVoxelMesh block;
    } data;
};

struct MeshChunkGenerator
{
    ReferenceCounter refCounter;
    VoxelMesh voxelMeshes[MAX_VOXEL_MESHES];
    int voxelMeshCount;
    BitConditionSolver* meshConditions;
};

struct ChunkEnvironment
{
    int w, h, d;
    const Voxel* voxels;
    List** meshLists;
    const bool* transparentVoxels;
    const int*  transparentNeighbours;
    MeshBuffer** materialMeshBuffers;
    int*         materialIds;
    int          materialCount;
};


static void FreeVoxelMesh( VoxelMesh* mesh );

MeshChunkGenerator* CreateMeshChunkGenerator()
{
    MeshChunkGenerator* generator = new MeshChunkGenerator;
    memset(generator, 0, sizeof(MeshChunkGenerator));
    InitReferenceCounter(&generator->refCounter);
    generator->meshConditions = CreateBitConditionSolver();
    return generator;
}

static void FreeMeshChunkGenerator( MeshChunkGenerator* generator )
{
    for(int i = 0; i < generator->voxelMeshCount; i++)
    {
        VoxelMesh* mesh = &generator->voxelMeshes[i];
        FreeVoxelMesh(mesh);
    }
    FreeBitConditionSolver(generator->meshConditions);
    delete generator;
}

void ReferenceMeshChunkGenerator( MeshChunkGenerator* generator )
{
    Reference(&generator->refCounter);
}

void ReleaseMeshChunkGenerator( MeshChunkGenerator* generator )
{
    Release(&generator->refCounter);
    if(!HasReferences(&generator->refCounter))
        FreeMeshChunkGenerator(generator);
}

static void* CreateVoxelMesh( MeshChunkGenerator* generator,
                              VoxelMeshType type,
                              int materialId,
                              const BitCondition* conditions,
                              int conditionCount )
{
    if(generator->voxelMeshCount < MAX_VOXEL_MESHES)
    {
        const int i = generator->voxelMeshCount;
        VoxelMesh* mesh = &generator->voxelMeshes[i];
        mesh->type = type;
        mesh->materialId = materialId;
        AddBitConditions(generator->meshConditions,
                         conditions,
                         conditionCount,
                         mesh);
        generator->voxelMeshCount++;
        return (void*)&mesh->data;
    }
    else
    {
        Error("Can't create more voxel meshes.");
        return NULL;
    }
}

bool CreateBlockVoxelMesh( MeshChunkGenerator* generator,
                           int materialId,
                           const BitCondition* conditions,
                           int conditionCount,
                           bool transparent,
                           MeshBuffer** meshBuffers,
                           mat4* transformations)
{
    BlockVoxelMesh* mesh = (BlockVoxelMesh*)CreateVoxelMesh(generator,
                                                            BLOCK_VOXEL_MESH,
                                                            materialId,
                                                            conditions,
                                                            conditionCount);
    if(mesh)
    {
        mesh->transparent = transparent;

        memcpy(mesh->meshBuffers, meshBuffers,
               sizeof(MeshBuffer*) * BLOCK_VOXEL_MATERIAL_BUFFER_COUNT);

        memcpy(mesh->transformations, transformations,
               sizeof(mat4) * BLOCK_VOXEL_MATERIAL_BUFFER_COUNT);

        for(int i = 0; i < BLOCK_VOXEL_MATERIAL_BUFFER_COUNT; i++)
            if(mesh->meshBuffers[i])
                ReferenceMeshBuffer(mesh->meshBuffers[i]);

        return true;
    }
    else
    {
        return false;
    }
}

static void FreeBlockVoxelMesh( BlockVoxelMesh* mesh )
{
    for(int i = 0; i < BLOCK_VOXEL_MATERIAL_BUFFER_COUNT; i++)
        ReleaseMeshBuffer(mesh->meshBuffers[i]);
}

static void FreeVoxelMesh( VoxelMesh* mesh )
{
    switch(mesh->type)
    {
        case BLOCK_VOXEL_MESH:
            FreeBlockVoxelMesh((BlockVoxelMesh*)&mesh->data);
            return;
    }
    FatalError("Unknown voxel mesh type.");
}

static bool IsVoxelMeshTransparent( const VoxelMesh* mesh )
{
    switch(mesh->type)
    {
        case BLOCK_VOXEL_MESH:
            return ((BlockVoxelMesh*)&mesh->data)->transparent;
    }
    FatalError("Unknown voxel mesh type.");
}

// ----------------------------------------------------------------------

static int Get3DArrayIndex( int x, int y, int z,
                            int w, int h, int d )
{
    assert(x >= 0 &&
           y >= 0 &&
           z >= 0 &&
           x < w &&
           y < h &&
           z < d);
    return z*h*w + y*w + x;
}

static Voxel* ReadVoxelChunk( VoxelVolume* volume,
                              int sx, int sy, int sz,
                              int w, int h, int d )
{
    const int voxelCount = w*h*d;
    Voxel* voxels = new Voxel[voxelCount];
    memset(voxels, 0, sizeof(Voxel)*voxelCount);

    for(int z = 0; z < d; z++)
    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
        ReadVoxelData(volume, sx+x, sy+y, sz+z, &voxels[Get3DArrayIndex(x,y,z,w,h,d)]);

    return voxels;
}

static List* GetVoxelMeshList( MeshChunkGenerator* generator,
                               const Voxel* voxel )
{
    return GatherPayloadFromBitField(generator->meshConditions,
                                     voxel,
                                     sizeof(Voxel));
}

static List** GatherVoxelMeshListsForChunk( MeshChunkGenerator* generator,
                                            const Voxel* voxels,
                                            int w, int h, int d )
{
    const int voxelCount = w*h*d;
    List** meshLists = new List*[voxelCount];
    for(int i = 0; i < voxelCount; i++)
        meshLists[i] = GetVoxelMeshList(generator, &voxels[i]);
    return meshLists;
}

static bool* GatherTransparentVoxelsForChunk( List** meshLists,
                                              int w, int h, int d )
{
    const int voxelCount = w*h*d;
    bool* transparentVoxels = new bool[voxelCount];
    for(int i = 0; i < voxelCount; i++)
    {
        const List* meshList = meshLists[i];
        bool transparent = true;
        const int meshCount = GetListLength(meshList);
        for(int j = 0; j < meshCount; j++)
        {
            const VoxelMesh* mesh =
                *(const VoxelMesh**)GetConstListEntry(meshList, j);
            if(!IsVoxelMeshTransparent(mesh))
            {
                transparent = false;
                break;
            }
        }
        transparentVoxels[i] = transparent;
    }
    return transparentVoxels;
}

static int GetTransparentMooreNeighbourhood( const bool* transparentVoxels,
                                             int x, int y, int z,
                                             int w, int h, int d )
{
    assert(x >= 1 && x <= w-2);
    assert(y >= 1 && y <= h-2);
    assert(z >= 1 && z <= d-2);
    int transparentNeighbours = 0;
#define TEST(xo, yo, zo, flag) \
    if(transparentVoxels[Get3DArrayIndex(x+(xo),y+(yo),z+(zo),w,h,d)]) \
        transparentNeighbours |= (flag);
    TEST(+1,0,0,MOORE_POSITIVE_X)
    TEST(-1,0,0,MOORE_NEGATIVE_X)
    TEST(0,+1,0,MOORE_POSITIVE_Y)
    TEST(0,-1,0,MOORE_NEGATIVE_Y)
    TEST(0,0,+1,MOORE_POSITIVE_Z)
    TEST(0,0,-1,MOORE_NEGATIVE_Z)
#undef TEST
    return transparentNeighbours;
}

static int* GatherTransparentNeighboursForChunk( const bool* transparentVoxels,
                                                 int w, int h, int d )
{
    const int voxelCount = w*h*d;
    int* transparentNeighbours = new int[voxelCount];
    memset(transparentNeighbours, 0, sizeof(int)*voxelCount);
    for(int z = 1; z < d-1; z++)
    for(int y = 1; y < h-1; y++)
    for(int x = 1; x < w-1; x++)
        transparentNeighbours[Get3DArrayIndex(x,y,z,w,h,d)] =
            GetTransparentMooreNeighbourhood(transparentVoxels,
                                             x, y, z,
                                             w, h, d);
    return transparentNeighbours;
}

static MeshBuffer* GetMeshBufferForMaterial( ChunkEnvironment* env,
                                             int materialId )
{
    int materialCount = env->materialCount;

    // Look if the material has already been used:
    for(int i = 0; i < materialCount; i++)
        if(env->materialIds[i] == materialId)
            return env->materialMeshBuffers[i];

    // Create a new entry:

    // Extend the arrays by one:
    materialCount++;
    env->materialCount = materialCount;
    env->materialMeshBuffers =
        (MeshBuffer**)realloc(env->materialMeshBuffers,
                              sizeof(MeshBuffer*)*materialCount);
    env->materialIds =
        (int*)realloc(env->materialIds,
                      sizeof(int)*materialCount);

    // Insert the material:
    MeshBuffer* buffer = CreateMeshBuffer();
    ReferenceMeshBuffer(buffer);
    env->materialIds[materialCount-1] = materialId;
    env->materialMeshBuffers[materialCount-1] = buffer;

    return buffer;
}

static const float HalfPI = (float)(PI/2.0);



static void ProcessBlockVoxelMesh( ChunkEnvironment* env,
                                   int transparentNeighbours,
                                   MeshBuffer* materialMeshBuffer,
                                   float x, float y, float z,
                                   const BlockVoxelMesh* mesh )
{
    static const int dirCount = 6;
    static const int mooreDirs[dirCount] =
    {
        MOORE_POSITIVE_X,
        MOORE_NEGATIVE_X,
        MOORE_POSITIVE_Y,
        MOORE_NEGATIVE_Y,
        MOORE_POSITIVE_Z,
        MOORE_NEGATIVE_Z
    };
    static const BlockVoxelMeshBuffers dirs[dirCount] =
    {
        POSITIVE_X,
        NEGATIVE_X,
        POSITIVE_Y,
        NEGATIVE_Y,
        POSITIVE_Z,
        NEGATIVE_Z
    };
    static const mat4 dirTransforms[dirCount] =
    {
        rotate(mat4(1), +HalfPI, vec3(0,1,0)), // +x
        rotate(mat4(1), -HalfPI, vec3(0,1,0)), // -x
        rotate(mat4(1), +HalfPI, vec3(1,0,0)), // +y
        rotate(mat4(1), -HalfPI, vec3(1,0,0)), // -y
        mat4(1), // +z
        rotate(mat4(1), (float)PI, vec3(0,1,0)) // -z
    };

    const MeshBuffer* const* meshBuffers = &mesh->meshBuffers[0];
    const mat4* transformations = (const mat4*)mesh->transformations;
    const mat4 voxelTransformation = translate(mat4(1), vec3(x, y, z));

    if(transparentNeighbours != 0 &&
       meshBuffers[CENTER])
    {
        const mat4 transformation = voxelTransformation *
                                    transformations[CENTER];
        AppendMeshBuffer(materialMeshBuffer,
                         meshBuffers[CENTER],
                         &transformation);
    }

    for(int i = 0; i < dirCount; i++)
    {
        if(transparentNeighbours & mooreDirs[i] && meshBuffers[dirs[i]])
        {
            const mat4 transformation = voxelTransformation *
                                        transformations[dirs[i]];
            AppendMeshBuffer(materialMeshBuffer,
                             meshBuffers[dirs[i]],
                             &transformation);
        }
    }
}

static void ProcessVoxelMesh( ChunkEnvironment* env,
                              int transparentNeighbours,
                              float x, float y, float z,
                              const VoxelMesh* mesh )
{
    MeshBuffer* materialMeshBuffer =
        GetMeshBufferForMaterial(env, mesh->materialId);
    switch(mesh->type)
    {
        case BLOCK_VOXEL_MESH:
            ProcessBlockVoxelMesh(env,
                                  transparentNeighbours,
                                  materialMeshBuffer,
                                  x, y, z,
                                  (BlockVoxelMesh*)&mesh->data);
            return;
    }
    FatalError("Unknown voxel mesh type.");
}

static void ProcessVoxelMeshes( ChunkEnvironment* env )
{
    const int w = env->w-2;
    const int h = env->h-2;
    const int d = env->d-2;
    const int voxelCount = w*h*d;

    const float hw = ((float)w) / 2.f;
    const float hh = ((float)h) / 2.f;
    const float hd = ((float)d) / 2.f;

    for(int z = 0; z < d; z++)
    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
    {
        const int envIndex = Get3DArrayIndex(x+1,
                                             y+1,
                                             z+1,
                                             env->w,
                                             env->h,
                                             env->d);
        const int transparentNeighbours = env->transparentNeighbours[envIndex];
        List* meshList = env->meshLists[envIndex];

        const int meshCount = GetListLength(meshList);
        for(int i = 0; i < meshCount; i++)
        {
            const VoxelMesh* mesh = *(VoxelMesh**)GetListEntry(meshList, i);
            const float xTranslation = ((float)x) - hw + 0.5f;
            const float yTranslation = ((float)y) - hh + 0.5f;
            const float zTranslation = ((float)z) - hd + 0.5f;
            ProcessVoxelMesh(env,
                             transparentNeighbours,
                             xTranslation,
                             yTranslation,
                             zTranslation,
                             mesh);
        }
    }
}

static ChunkEnvironment* CreateChunkEnvironment( MeshChunkGenerator* generator,
                                                 VoxelVolume* volume,
                                                 int sx, int sy, int sz,
                                                 int w, int h, int d )
{
    ChunkEnvironment* env = new ChunkEnvironment;
    memset(env, 0, sizeof(ChunkEnvironment));
    env->w = w;
    env->h = h;
    env->d = d;
    env->voxels = ReadVoxelChunk(volume,
                                 sx, sy, sz,
                                  w,  h,  d);
    env->meshLists = GatherVoxelMeshListsForChunk(generator,
                                                  env->voxels,
                                                  w, h, d);
    env->transparentVoxels = GatherTransparentVoxelsForChunk(env->meshLists,
                                                             w, h, d);
    env->transparentNeighbours =
        GatherTransparentNeighboursForChunk(env->transparentVoxels,
                                            w, h, d);
    ProcessVoxelMeshes(env);
    return env;
}

static void FreeChunkEnvironment( ChunkEnvironment* env )
{
    const int voxelCount = env->w *
                           env->h *
                           env->d;
    for(int i = 0; i < voxelCount; i++)
        FreeList(env->meshLists[i]);

    delete[] env->voxels;
    delete[] env->meshLists;
    delete[] env->transparentVoxels;
    delete[] env->transparentNeighbours;

    for(int i = 0; i < env->materialCount; i++)
        ReleaseMeshBuffer(env->materialMeshBuffers[i]);
    free(env->materialMeshBuffers);
    free(env->materialIds);

    delete env;
}

// ----------------------------------------------------------------------

static MeshChunk* GenerateMeshChunkWithEnv( ChunkEnvironment* env )
{
    MeshChunk* chunk = new MeshChunk;
    memset(chunk, 0, sizeof(MeshChunk));

    const int materialCount = env->materialCount;
    chunk->materialCount    = env->materialCount;
    chunk->materialIds      = new int[materialCount];
    chunk->materialMeshes   = new Mesh*[materialCount];

    memcpy(chunk->materialIds, env->materialIds, sizeof(int)*materialCount);
    for(int i = 0; i < materialCount; i++)
    {
        Mesh* mesh = CreateMesh(env->materialMeshBuffers[i]);
        ReferenceMesh(mesh);
        chunk->materialMeshes[i] = mesh;
    }

    return chunk;
}

MeshChunk* GenerateMeshChunk( MeshChunkGenerator* generator,
                              VoxelVolume* volume,
                              int sx, int sy, int sz,
                              int w, int h, int d )
{
    ChunkEnvironment* env = CreateChunkEnvironment(generator,
                                                   volume,
                                                   sx-1, sy-1, sz-1,
                                                    w+2,  h+2,  d+2);
    // ^- Enlarge chunk environment by one.

    MeshChunk* chunk = GenerateMeshChunkWithEnv(env);

    FreeChunkEnvironment(env);
    return chunk;
}

void FreeMeshChunk( MeshChunk* chunk )
{
    for(int i = 0; i < chunk->materialCount; i++)
        ReleaseMesh(chunk->materialMeshes[i]);
    delete[] chunk->materialMeshes;
    delete[] chunk->materialIds;
    delete chunk;
}
