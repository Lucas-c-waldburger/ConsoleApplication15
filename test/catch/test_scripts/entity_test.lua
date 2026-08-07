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

return table