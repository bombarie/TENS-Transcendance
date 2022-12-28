# TENS-Transcendance

Hardware design and code related to the development of a remotely-triggerable system of TENS muscle stimulation as part of the experimental dance / VR performance [Transcendance](https://www.theaterutrecht.nl/voorstellingen/transcendance).

The system extends an existing TENS muscle stimulator, the [Promed TENS 1000S](https://www.promed.de/soft-therapy/electrical-pain-therapy/144/promed-pain-therapy-device-tens-1000-s) and makes it controllable with OpenSoundControl (OSC) via WiFi through the use of an ESP32 microcontroller.

### Included in this repository 

**Software**
  - The [Arduino](https://www.arduino.cc/) code for the ESP32 microcontroller.
  - The code for the [Processing](https://processing.org/) sketch which served as a proxy between the ESP32 and a [TouchDesigner](https://derivative.ca/) patch (not by me) that was heavily integrating most of the performance.

**Hardware**
- The hardware designs to split a single TENS output channel to up to 16 different target locations. The PCB was designed in [KiCad](https://www.kicad.org/) 6.0.
- The additional libraries used in KiCad while designing the PCB are included in the repository.


### Dependencies

_coming..._

