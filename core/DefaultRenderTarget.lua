local class        = require 'middleclass'
local RenderTarget = require 'core/RenderTarget'
local GetDefaultRenderTarget = ENGINE.GetDefaultRenderTarget


local DefaultRenderTarget = class('core/DefaultRenderTarget', RenderTarget)

function DefaultRenderTarget.static:get()
    if not self.instance then
        self.instance = DefaultRenderTarget()
    end
    return self.instance
end

function DefaultRenderTarget:initialize()
    RenderTarget.initialize(self, GetDefaultRenderTarget())
end

function DefaultRenderTarget:destroy()
    -- Overridden, because default render target can't be destroyed.
    self.handle = nil
end


return DefaultRenderTarget
