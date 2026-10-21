local table = {}

function table.init()
end


function table.update(dt)
end

local meta = {
    __signatures = { 
        init = {},
        update = { "number" }
    },
    __phase = Phase.Input
}

setmetatable(table, meta)

return table