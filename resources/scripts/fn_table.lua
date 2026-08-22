local table = {}

function table.changeName(e)
    if e:hasName() then
        e:getName().value = "changed"
    end
end

function table.changePos(e)
    if e:hasTransform() then
        local t = e:getTransform()
        local p = t.position

        p.x = 500.0
        p.y = 500.0
    end               
end

function table.setPos(e, newPos)
    if e:hasTransform() then
        local t = e:getTransform()
        local p = t.position

        p = newPos
    end               
end

function table.gameControllerCallback(e, ev)
    if e:hasGameControllerState() then
        local gc = e:getGameControllerState()
        local id = gc.joystickID

        id = 69
    end
end

function table.timerCallback(ev, e)
    if e:hasSpriteAnimationComponent() then
        local anim = e:getSpriteAnimationComponent()
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