-- logging.lua
local logging = {}

-- Configuration
logging.debug_enabled = false
logging.buffer = {}
logging.max_entries = 1024
logging.log_file = "game.log"

-- Add a log entry to the buffer
function logging.log(msg)
    if #logging.buffer < logging.max_entries then
        table.insert(logging.buffer, msg)
    else
        -- optional: overwrite oldest entry (ring buffer)
        table.remove(logging.buffer, 1)
        table.insert(logging.buffer, msg)
    end
end

-- Save logs to file
function logging.save()
    local f, err = io.open(logging.log_file, "w")
    if not f then
        print("Failed to write log:", err)
        return
    end

    f:write("Log started at ", os.date(), "\n\n")
    for _, entry in ipairs(logging.buffer) do
        f:write(entry, "\n")
    end
    f:close()
end

-- Call this when the game ends
function logging.finalize(crashed)
    if logging.debug_enabled or crashed then
        logging.save()
    end
end

-- Enable/disable debug mode
function logging.set_debug(enabled)
    logging.debug_enabled = enabled
end

return logging
