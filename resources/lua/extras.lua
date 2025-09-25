-- Extras module 

local Extras = {}

function Extras:grab_game_title()
    local titles = {}
    for line in io.lines("resources/text/game_titles.txt") do
        table.insert(titles, line)
    end
    if #titles == 0 then
        return "No Titles Found"
    end

    return titles[math.random(#titles)]
end

return Extras