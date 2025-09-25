-- room.lua

function check_facing_object(player, object)
    local facing = player.facing
    local px, py = player.x, player.y
    local ox, oy = object.x, object.y
    local threshold = 16

    if facing == "up" and math.abs(px - ox) < threshold and (py - oy) > 0 and (py - oy) < threshold then
        return true
    elseif facing == "down" and math.abs(px - ox) < threshold and (oy - py) > 0 and (oy - py) < threshold then
        return true
    elseif facing == "left" and math.abs(py - oy) < threshold and (px - ox) > 0 and (px - ox) < threshold then
        return true
    elseif facing == "right" and math.abs(py - oy) < threshold and (ox - px) > 0 and (ox - px) < threshold then
        return true
    end

    return false
end

function interact_with_object(player, object)
    if check_facing_object(player, object) then
        if object.on_interact then
            object.on_interact(player, object)
        end
    end
end