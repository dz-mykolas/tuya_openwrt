
function ubus_get_memory()
    ubus = require("ubus")

    local conn = ubus.connect()
    if not conn then
        error("Failed to connect to ubus")
    end
    
    local status = conn:call("system", "info", {})
    local free_mem = -1
    if status and status.memory then
        free_mem = status.memory.free
    end
    conn:close()
    return free_mem
end

function tuya_config()
    local config = {
        type = "haha",
        interval = -2,
    }
    return config
end
