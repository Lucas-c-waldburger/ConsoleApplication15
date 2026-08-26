local table = {}
local id = 0

function table.init()
	local e = ecs.createEntity()
	if e:isValid() then
		local name = e:addComponent(ComponentId.Name)
		name.value = "greg"
		id = e:getId()
	end
end


function table.update()
	local e = ecs.getEntityById(id)
	if e:isValid() and e:hasComponent(ComponentId.Name) then
		e:getComponent(ComponentId.Name).value = "hank"
	end
end

local meta = {
    __signatures = { 
        init = {},
        update = {}
    }
}

setmetatable(table, meta)

return table