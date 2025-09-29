local extras = {}

function extras:setup_saves()
    if not engine.does_directory_exist("saves") then
        engine.create_directory("saves")
    end
end

function extras:test_write()
    local success = engine.write_file("opt", "Thank you for playing Orion Public Transit!" .. os.date("\n%Y-%m-%d %H:%M:%S"))
    if success then
        print("Wrote to opt file successfully.")
    else
        print("Failed to write to opt file.")
    end
end

function extras:mem_checks()
    if engine.mem_get(2) == 1 then
        engine.mem_set(2, 0)
        engine.request_quit()
    elseif engine.mem_get(3) == 1 then
        engine.mem_set(3, 0)
        extras:debug_save()
    end
end

function extras:debug_save()
    local success = engine.write_file("saves/debug_save.txt", "Debug save at " .. os.date("%Y-%m-%d %H:%M:%S"))
    if success then
        print("Debug save created successfully.")
    else
        print("Failed to create debug save.")
    end
end

return extras