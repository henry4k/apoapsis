local class = require 'core/middleclass'
local Model = require 'core/Model'
local Mesh  = require 'core/Mesh'
local Texture = require 'core/Texture'


local ReferenceCube = class('example/ReferenceCube')

function ReferenceCube:initialize( stage, shaderProgram )
    local mesh = Mesh:load('example/ReferenceCube/Scene.json', 'Cube')
    local diffuseTexture = Texture:load('2d', 'example/ReferenceCube/Diffuse.png')
    self.model = Model:new(stage, shaderProgram)
    self.model:setMesh(mesh)
    self.model:setTexture(0, diffuseTexture)
    self.model:setUniform('DiffuseSampler', 0)
end

return ReferenceCube
