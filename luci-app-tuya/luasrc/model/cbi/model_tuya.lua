map = Map("tuya_daemon")

section = map:section(NamedSection, "tuya_connect", "tuya_daemon", "Tuya Daemon")

flag = section:option(Flag, "enable", "Enable", "Enable program")

product_id = section:option(Value, "product_id", "Product ID")
device_id = section:option(Value, "device_id", "Device ID")
device_secret = section:option(Value, "device_secret", "Device Secret")
product_id.maxlength = 35;
device_id.maxlength = 35;
device_secret.maxlength = 35;

return map