Code built off: https://github.com/dmiddlecamp/fancy-asset-tracker

Uses the [Asset Tracker Shield from Particle](https://docs.particle.io/datasheets/kits/#electron-asset-tracker)!

Motion tracking outline:

- Sets the accelerometer to wake up the electron if the device moves in the 1-2G range.
- If it wakes up and then senses motion again from the accelerometer, it connects to the cloud and reports GPS every minute
- Once the device stops sensing motion, it continues to report its location for the next 3 minutes, then goes to sleep for 6 hours
- If it wakes without motion, it sends a heartbeat, then goes back to sleep

Setup, building and flashing
===

Make sure you have the [particle-cli](https://github.com/spark/particle-cli) installed

```
./build.sh
# Then put Electron into DFU firmware flashing mode (yellow blinking)
./flash
```

`/lib` contains a fork of the official Particle libraries

Attributions
===

This project is possible thanks to awesome work from Particle and Adafruit, go buy stuff from them!

Other similar libraries:
https://github.com/benagricola/tracker
