local ValueMatcher = require 'test.mock.ValueMatcher'


--- Creates an easily programmable function for testing purposes.
-- Multiple behaviours can be defined.
-- A behaviour consists of a set of arguments and a set of return values.
-- If the function is called with these arguments it will return the programmed
-- return values.
local ProgrammableFn =
{
    mt = {},
    prototype = {}
}
ProgrammableFn.mt.__index = ProgrammableFn.prototype
setmetatable(ProgrammableFn.prototype, ProgrammableFn.prototype)
setmetatable(ProgrammableFn, ProgrammableFn)


function ProgrammableFn:__call()
    return self:new()
end

function ProgrammableFn:new()
    local instance = {
        behaviours = {}
    }
    return setmetatable(instance, self.mt)
end

local function behaviourReturnValues( behaviour )
    local next = behaviour.nextReturnSet

    local returnSet = behaviour.returnSets[next]

    if next < #behaviour.returnSets then
        next = next + 1
    else
        next = 1
    end
    behaviour.nextReturnSet = next

    return table.unpack(returnSet)
end

function ProgrammableFn.mt:__call( ... )
    local behaviour = self:findMatchingBehaviour_({...})
    if not behaviour then
        error('No matching behaviour for call.', 2)
    end
    return behaviourReturnValues(behaviour)
end

function ProgrammableFn.prototype:findMatchingBehaviour_( arguments )
    for _,behaviour in ipairs(self.behaviours) do
        if ValueMatcher.matches(arguments, behaviour.arguments) then
            return behaviour
        end
    end
    return nil
end

--- Creates a new behaviour entry or extends to one.
--
-- @param specification
-- The specification is a table, that contains the arguments that must match to
-- trigger this behaviour and the values that will be returned then.
-- Both are optional and can be passed like this:
-- `canBeCalled{with={1,2}, thenReturn={3}}`
function ProgrammableFn.prototype:canBeCalled( specification )
    local arguments = specification.with or {}
    local returnSet = specification.thenReturn or {}

    local behaviour = self:findMatchingBehaviour_(arguments)
    if behaviour then
        table.insert(behaviour.returnSets, returnSet)
    else
        behaviour = {
            arguments = arguments,
            returnSets = { returnSet },
            nextReturnSet = 1
        }
        table.insert(self.behaviours, behaviour)
    end

    return self
end

function ProgrammableFn.prototype:reset()
    self.behaviours = {}
    return self
end


return ProgrammableFn
