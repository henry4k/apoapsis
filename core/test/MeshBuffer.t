#!/usr/bin/env lua
-- vim: set filetype=lua:
require 'core/test/common'

local Mock = require 'test/mock/Mock'


describe('A mesh bufffer')
    :setup(function()
        NATIVE = {
            CreateMeshBuffer = Mock(),
            DestroyMeshBuffer = Mock(),
            TransformMeshBuffer = Mock(),
            AppendMeshBuffer = Mock(),
            AppendIndexToMeshBuffer = Mock(),
            AppendVertexToMeshBuffer = Mock()
        }

        ResetMocks = function()
            NATIVE.CreateMeshBuffer:reset()
            NATIVE.DestroyMeshBuffer:reset()
            NATIVE.TransformMeshBuffer:reset()
            NATIVE.AppendMeshBuffer:reset()
            NATIVE.AppendIndexToMeshBuffer:reset()
            NATIVE.AppendVertexToMeshBuffer:reset()
        end

        FakeRequire:whitelist('core/MeshBuffer')
        FakeRequire:whitelist('core/middleclass')
        FakeRequire:install()

        MeshBuffer = require 'core/MeshBuffer'
    end)

    :beforeEach(function()
        ResetMocks()
    end)

    :it('can be created and destroyed.', function()
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'the handle'}}
        NATIVE.DestroyMeshBuffer:canBeCalled{with={'the handle'}}

        local buffer = MeshBuffer()
        assert(buffer.handle == 'the handle')
        NATIVE.CreateMeshBuffer:assertCallCount(1)

        buffer:destroy()
        assert(buffer.handle == nil)
        NATIVE.DestroyMeshBuffer:assertCallCount(1)
    end)

    :it('can transform its vertices.', function()
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'mesh buffer handle'}}
        local buffer = MeshBuffer()

        NATIVE.TransformMeshBuffer:canBeCalled{with={'mesh buffer handle', 'matrix handle'}}

        local transformation = { handle='matrix handle' }
        buffer:transform(transformation)

        NATIVE.TransformMeshBuffer:assertCallCount(1)
    end)

    :it('can append other mesh buffers.', function()
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'mesh buffer handle a'}}
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'mesh buffer handle b'}}
        NATIVE.AppendMeshBuffer:canBeCalled{with={'mesh buffer handle a',
                                                  'mesh buffer handle b'} }
        NATIVE.AppendMeshBuffer:canBeCalled{with={'mesh buffer handle a',
                                                  'mesh buffer handle b',
                                                  'matrix handle'} }

        bufferA = MeshBuffer()
        bufferB = MeshBuffer()

        bufferA:appendMeshBuffer(bufferB)

        local transformation = { handle='matrix handle' }
        bufferA:appendMeshBuffer(bufferB, transformation)

        NATIVE.AppendMeshBuffer:assertCallCount(2)
    end)

    :it('can append indices.', function()
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'mesh buffer handle'}}
        local buffer = MeshBuffer()

        NATIVE.AppendIndexToMeshBuffer:canBeCalled{with={'mesh buffer handle', 42}}

        buffer:appendIndex(42)

        NATIVE.AppendIndexToMeshBuffer:assertCallCount(1)
    end)

    :it('can append vertices.', function()
        NATIVE.CreateMeshBuffer:canBeCalled{thenReturn={'mesh buffer handle'}}
        local buffer = MeshBuffer()

        NATIVE.AppendVertexToMeshBuffer:canBeCalled{with={'mesh buffer handle',
                                                          1, 2, 3,
                                                          4, 5, 6,
                                                          7, 8,
                                                          9, 10, 11,
                                                          12, 13, 14, 15} }

        NATIVE.AppendVertexToMeshBuffer:canBeCalled{with={'mesh buffer handle',
                                                          10, 20, 30,
                                                          1, 1, 1,
                                                          0, 0,
                                                          0, 1, 0,
                                                          1, 0, 0, 1}}

        buffer:appendVertex({x=1, y=2, z=3,
                             r=4, g=5, b=6,
                             tx=7, ty=8,
                             nx=9, ny=10, nz=11,
                             tanx=12, tany=13, tanz=14, tanw=15})

        buffer:appendVertex({x=10, y=20, z=30})

        NATIVE.AppendVertexToMeshBuffer:assertCallCount(2)
    end)


bdd.runTests()
