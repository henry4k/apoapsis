--- Changes the velocity of a @{Solid} over time.
-- Here a force is defined by its vector and position. The vector defines the
-- Forces that are at the center will only change the solids linear velocity,
-- while forces that are applied off-center will also change the angular
-- velocity. They are applied in each simulation step until you 'destroy' them.
--
-- @module core.physics.Force


local assert = assert
local class  = require 'middleclass'
local Object = class.Object
local Vec    = require 'core/Vector'
local DestroyForce = ENGINE.DestroyForce
local SetForce     = ENGINE.SetForce


local Force = class('core/physics/Force')

--- Initially all properties are zero, so that the force has no effect.
-- @warning DON'T CALL THIS DIRECTLY!  Use @{Solid:createForce} instead.
function Force:initialize( handle )
    self.handle = handle
end

function Force:destroy()
    DestroyForce(self.handle)
    self.handle = nil
end

--- Changes the properties of the force.
--
-- @param value
-- Describes the magnitude and direction that is applied in one second.
--
-- @param relativePosition
-- Point where the force is applied to the solid.
--
-- @param useLocalCoordinates
-- If set direction and position will be relative to the solids orientation.
function Force:set( value, relativePosition, useLocalCoordinates )
    assert(Vec:isInstance(value), 'Value must be vector.')
    assert(Vec:isInstance(relativePosition) or nil, 'Relative position must be vector.')
    relativePosition    = relativePosition or Vec(0,0,0)
    useLocalCoordinates = useLocalCoordinates or false
    SetForce(self.handle,
             value[1],
             value[2],
             value[3],
             relativePosition[1],
             relativePosition[2],
             relativePosition[3],
             useLocalCoordinates)
end


return Force
