local conn
function module_init()
    ubus = require("ubus")

    conn = ubus.connect()
    if not conn then
        return "failed init"
    end

    return tuya_config()
end

function module_deinit()
    if conn then
        conn:close()
    end
end

function lua_main()
    local free_mem = -1

    local status = conn:call("system", "info", {})
    if status and status.memory then
        free_mem = status.memory.free
    end

    return create_json(free_mem)
end

function create_json(free_mem)
    local jsonc = require("luci.jsonc")

    local mem_in_mb = free_mem / (1024 * 1024)
    local data = {
        ram_free = {
            value = string.format("%dMB", math.floor(mem_in_mb))
        }
    }

    return jsonc.stringify(data)
end

function tuya_config()
    local config = {
        type = "auto",
        interval = 5,
    }
    
    return config
end
