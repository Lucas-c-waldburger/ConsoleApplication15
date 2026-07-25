local table = {}

function table.changeName(e)
    if e:hasName() then
        e:getName().value = "changed"
    end
end

function table.changePos(e)
    if e:hasTransform() then
        pos = e:getTransform().position
        pos.x = 500
        pos.y = 500
    end               
end

return table