module("luci.controller.controller_tuya", package.seeall)

function index()
    entry({"admin", "services", "example"}, cbi("model_tuya"), "Tuya IoT", 100)
end