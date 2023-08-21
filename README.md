## Install
Install packages on RutOS using Opkg for web interface 

## Connecting
In the newly installed tab inside web interface enter product ID, Device ID and Device Secret (aquired from Tuya IoT cloud) and turn it on.

## Usage
Based on the installed modules inside tuya_iot_daemon/files/tuya_modules the program will interact with cloud in that way. You can add additional modules by creating a file in this format: `name.lua` where name is any string of your choice that does not exceed `NAME_MAX`. 
#### Structure of Lua file
A single module needs at least `lua_main()` function which will be called from the program when it initializes. It needs to return data in JSON string format which will be sent to Tuya Cloud. The format should be defined by the specific data you've created there (more info on data parsing: https://developer.tuya.com/en/docs/iot/Data-Parsing?id=Kb4qgsj9g1duj). <br>

#### tuya_config()
If module does not have `tuya_config()` function it will automatically be assigned with default settings:

| Field | Explanation |
| ------------- | ------------- |
| `type: "auto"` | `"auto"` type runs the module by the interval. `"action"` type runs the module when the action is received from Tuya Cloud by defined `action_name` |
| `interval: 60` | in seconds for `"auto"` type |
| `action_name: ""` | empty string by default. Used for actions if defined in Tuya Cloud |

#### module_init()
This allows to initialize any libraries that are required for your module at the start of the program (removes the need to initialize each time the module is run)

#### module_deinit()
This is to deinitialize the libraries once the program finishes

## Notes
#### You can check the error/warning logs to help with creating additional modules. There are example ones inside the folder as well (some are for testing purposes)
