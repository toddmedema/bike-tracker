Code built off: https://github.com/dmiddlecamp/fancy-asset-tracker

Uses the [Asset Tracker Shield from Particle](https://docs.particle.io/datasheets/kits/#electron-asset-tracker)!

Motion tracking outline:

- Set your device name, allowing for multiple trackers on a single account (default: `toddbike`)
- Accelerometer wakes up the electron if the device moves in the 1-2G range (the onboard blue LED will also flash when it detects motion)
- Upon wake (from motion or the long-sleep checking), it connects to the cloud and reports GPS every minute (ie `toddbike_g=40.53211,-40.23455`)
- Once the device stops sensing motion, it continues to report its location for 2 minutes (2 more reports), then goes to sleep for 8 hours
- Publishes the battery status (ie `toddbike_b=3.94v80.1%`) right before going to sleep, serving both as the "going to sleep" indicator, and allowing you to log battery state
- Solar Cell optimization: If it wakes at <20% battery (not enough to start a cell connection, which draws a lot of current), it immediately sleeps for an hour; it'll keep trying this until it's above 20% (even at 20% battery, it should be able to keep doing this for 5+ days)


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

Related libraries and tools:

- https://github.com/benagricola/tracker
- https://community.particle.io/t/asset-tracker-gps-real-time-google-map/20380
- https://docs.particle.io/datasheets/electron-datasheet/
- https://docs.particle.io/reference/firmware/electron
