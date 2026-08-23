local table = {}

function table.init()
	local es = ecs.getEntitiesWith(ComponentId.Transform)
	for _, e in ipairs(es) do
		local tf = e:getTransform()
		local p = tf.position

		p.x = 67.0
		p.y = 69.0
	end
end


function table.update(dt)
	local es = ecs.getEntitiesWith(ComponentId.Transform)
	for _, e in ipairs(es) do
		local tf = e:getTransform()
		local p = tf.position

		p.x = p.x - 1.0
		p.y = p.y - 1.0
	end
end

local meta = {
    __signatures = { 
        init = {},
        update = {"number"}
    }
}

setmetatable(table, meta)

return table