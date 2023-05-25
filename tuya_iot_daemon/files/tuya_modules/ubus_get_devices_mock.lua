function lua_main()
    return create_json(free_mem)
end

function create_json()
    local jsonc = require("luci.jsonc")

    local device1 = "\"port_name\":\"/dev/ttyUSB1\",\"vid\":\"10C4\",\"pid\":\"EA60\""
    local device2 = "\"port_name\":\"/dev/ttyUSB2\",\"vid\":\"12D1\",\"pid\":\"AE06\""
    local data = {
        usb_devices = {
            device1,
            device2
        }
    }

    return jsonc.stringify(data)
end

function tuya_config()
    local config = {
        type = "action",
        interval = 15,
        action_name = "get_devices"
    }
    return config
end
