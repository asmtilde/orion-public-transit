local Extras = require("resources/lua/extras")

state = 0
block_input = false

local fade_duration = 3.0
local elapsed = 0.0
local has_intro_played = false

is_music_playing = false

local game = {}

game.sound = {}

game.music = {
    slumfunk = "resources/sound/slumfunk.mp3"
}

function game_init()
    state = 1

    math.randomseed(os.time())
    collectgarbage("collect")

    load_resources()

    local new_title = "Orion Public Transit: " .. Extras:grab_game_title()
    engine.change_screen_title(new_title)

    state = 2
end

function load_resources()

    for name, path in pairs(game.music) do
        if not engine.does_file_exist(path) then
            log.error("Music file not found: " .. path)
        else
            game.music[name] = engine.load_music(path)
        end
    end
end

function game_update()
    local dt = engine.get_frame_time()

    handle_input()

    if game.music.slumfunk then
        engine.update_music(game.music.slumfunk)
    end

    if state == 3 then
        if not is_music_playing and game.music.slumfunk then
            engine.play_music(game.music.slumfunk, -1)
            is_music_playing = true
        end
        if elapsed < fade_duration then
            elapsed = elapsed + dt
        else
            block_input = false
            has_intro_played = true
            state = 4
        end
    end
end

function handle_input()
    if not block_input then
        if state == 2 then
            if engine.is_any_key_pressed() then
                state = 3
                block_input = true
            end
        end
    end
end

function draw_fade(start_r, start_g, start_b, end_r, end_g, end_b, elapsed, duration)
    local alpha = math.min(elapsed / duration, 1.0)
    local r = math.floor(start_r + (end_r - start_r) * alpha)
    local g = math.floor(start_g + (end_g - start_g) * alpha)
    local b = math.floor(start_b + (end_b - start_b) * alpha)
    engine.draw_background(r, g, b, 255)
end

function game_draw()
    if state == 1 then
        engine.draw_background(0, 0, 0, 255)
        engine.draw_text_centered("Loading...", 640, 300, 50, 255, 255, 255, 255)
    elseif state == 2 then
        engine.draw_background(0, 0, 0, 255)
        engine.draw_text_centered("Press any key to start", 640, 300, 50, 255, 255, 255, 255)
    elseif state == 3 then
        draw_fade(0, 0, 0, 255, 255, 255, elapsed, fade_duration)
    elseif state == 4 then
        engine.draw_background(255, 255, 255, 255)
        engine.draw_text_centered("shitcode", 640, 300, 50, 0, 0, 0, 255)
    end
end

function game_quit(crashed)
    log.finalize(crashed)
    engine.quit()
end
