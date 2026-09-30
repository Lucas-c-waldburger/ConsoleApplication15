
local function defer(fn)
    local proxy = newproxy and newproxy(true) or setmetatable({}, { __gc = true })
    local mt = getmetatable(proxy)
    mt.__gc = function() fn() end
    return proxy
end

local function defer_end_window()
	return defer(function()
        if guard then 
            gui.end_window() 
        end
	end)
end

function draw()
    local guard = true

    gui.begin_window("my window")

    local _ = defer_end_window()

    gui.text_unformatted("hello")
    gui.separator()

	mouse_pos = gui.get_mouse_pos()
	gui.text_unformatted(mouse_pos.x)

	gui.separator()

    gui.end_window()
    guard = false
    
    collectgarbage("step", 1) 
end

return draw