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

local meta = {
    __signatures = {
        changeName = { "Entity" },
        changePos = { "Entity" }
    }
}

setmetatable(table, meta)

return table