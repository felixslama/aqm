# What Works:

- general BLE communication with sensor board (standalone)
- reporting of sensor vals to user
- Web Interface
- OTA Code

# What Doesnt Work:

- MQTT Code

# Probable Cause of Isses

Likely a memory leak somewhere in the bluetooth scanning procedure.
This leads to MQTT failing because of insufficient heap memory.
