-- opt player module
-- :3

-- Player Structure
local Player = {}
Player.__index = Player

-- shared constants
Player.width = 32
Player.height = 64
Player.speed = 96

-- player constructor
function Player:new(x, y, name)
    local self = setmetatable({}, Player)
    self.name = name or "Aleph"
    self.x = x or 0
    self.y = y or 0
    return self
end

function Player:move(dx, dy)
    if self.can_move then
        self.x = self.x + dx
        self.y = self.y + dy
    end
end

function Player:set_position(x, y)
    self.x = x
    self.y = y
end

function Player:set_facing(direction)
    local valid_directions = { up=true, down=true, left=true, right=true }
    if valid_directions[direction] then
        self.facing = direction
    end
end

function Player:init()
    self.is_loaded = true
    self.can_move = true
    self.facing = "down"

    print("Player " .. self.name .. " initialized at (" .. self.x .. ", " .. self.y .. ")")
end

return Player