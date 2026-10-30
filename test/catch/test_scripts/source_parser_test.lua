local music_table = {}
local var = false

function music_table.isGuitar(instrument)
    var = (instrument.type == InstrumentType.Guitar)
end

function music_table.addKeyboard(gig)
    local kb = Instrument(InstrumentType.Keyboard, 12, 399.99)
    gig:addInstrument(kb)
end

function music_table.setSizeForSmoking(venue, size)
    if venue.allowsSmoking then
        venue:setSize(size)
    else
        venue:setSize(VenueSize.Medium)
    end               
end

local meta_stuff = {
    __signatures = { 
        isGuitar = { "const Instrument&" },
        addKeyboard = { "Gig&" },
        setSizeForSmoking = { "Venue*", "VenueSize" }
    }
}

setmetatable(music_table, meta_stuff)

return music_table