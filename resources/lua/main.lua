local extras = require("resources.lua.extras")

local save_files = {}

function game_init()
    engine.mem_set(0, 0)    -- default the game_state to 0

    extras:setup_saves()
    extras:test_write()

    print(engine.int_to_string(280991461480)) -- should print "aleph"
end

function game_update()
    local dt = engine.get_frame_time()

    extras:mem_checks()
    handle_input()
end

function handle_input()
    if not engine.mem_get(1) == 0 then
    end
end

function game_draw()

end