local class  = require 'middleclass'
local CreateCamera                = ENGINE.CreateCamera
local DestroyCamera               = ENGINE.DestroyCamera
local SetCameraAttachmentTarget   = ENGINE.SetCameraAttachmentTarget
local SetCameraViewTransformation = ENGINE.SetCameraViewTransformation
local SetCameraFieldOfView        = ENGINE.SetCameraFieldOfView
local SetCameraNearAndFarPlanes   = ENGINE.SetCameraNearAndFarPlanes


local Camera = class('core/Camera')

function Camera:initialize( modelWorld )
    self.handle = CreateCamera(modelWorld.handle)
    self.modelWorld = modelWorld
end

function Camera:destroy()
    DestroyCamera(self.handle)
    self.handle = nil
end

function Camera:getModelWorld()
    return self.modelWorld
end

function Camera:setAttachmentTarget( solid, flags )
    flags = flags or 'rt'
    SetCameraAttachmentTarget(self.handle, solid.handle, flags )
    self.attachmentTarget = solid
end

function Camera:getAttachmentTarget()
    return self.attachmentTarget
end

function Camera:setViewTransformation( transformation )
    SetCameraViewTransformation(self.handle, transformation.handle)
end

function Camera:setFieldOfView( fov )
    SetCameraFieldOfView(self.handle, fov)
end

function Camera:setNearAndFarPlanes( zNear, zFar )
    SetCameraNearAndFarPlanes(self.handle, zNear, zFar)
end


return Camera
