local Shutdown = {
    handlers = {}
}

--- Registers a handler that is called when the game is being stopped.
function Shutdown.registerHandler( handler )
    table.insert(Shutdown.handlers, handler)
end

function Shutdown.callHandlers_()
    print('Calling shutdown handlers ..')
    for _,handler in ipairs(Shutdown.handlers) do
        handler()
    end
end
NATIVE.SetEventCallback('Shutdown', Shutdown.callHandlers_)

return Shutdown