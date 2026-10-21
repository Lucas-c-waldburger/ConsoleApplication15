local table = {}

function table.changeName(e)
    if e:hasComponent(ComponentId.Name) then
        e:getComponent(ComponentId.Name).value = "changed"
    end
end

function table.changePos(e)
    if e:hasComponent(ComponentId.Transform) then
        local t = e:getComponent(ComponentId.Transform)
        local p = t.position

        p.x = 500.0
        p.y = 500.0
    end               
end

function table.setPos(e, newPos)
    if e:hasComponent(ComponentId.Transform) then
        local t = e:getComponent(ComponentId.Transform)
        local p = t.position

        p = newPos
    end               
end

function table.gameControllerCallback(e, ev)
    if e:hasComponent(ComponentId.GameControllerState) then
        local gc = e:getComponent(ComponentId.GameControllerState)
        local id = gc.joystickID

        id = 69
    end
end

function table.timerCallback(ev, e)
    if e:hasComponent(ComponentId.SpriteAnimationComponent) then
        local anim = e:getComponent(ComponentId.SpriteAnimationComponent)
        local name = anim.spriteSeriesName

        name = "fired"
    end
end

local meta = {
    __signatures = { 
        changeName = { "Entity" },
        changePos = { "Entity" },
        setPos = { "Entity&", "SDL_FPoint" },
        gameControllerCallback = { "Entity", "const GameControllerConnectedEvent &"},
        timerCallback = { "const  TimerFiredEvent&", "Entity" }
    }
}

setmetatable(table, meta)

return table