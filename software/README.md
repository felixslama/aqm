# What Works:

- general BLE communication with sensor board (standalone)
- reporting of sensor vals to user
- Web Interface
- OTA Code
- MQTT Code

# What Doesnt Work:

- BLE Code

# Probable Cause of Isses

New NimBLE Lib doesnt have as large of a memory footprint as the normal one.
We still have some sort of conflict when creating a new BLEC02 class so it doesnt build yet.
