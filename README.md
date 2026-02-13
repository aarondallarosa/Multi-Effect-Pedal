**This repository is a work in progress.**  
Please be patient as I continue to develop and update this project!
I apologize for this being incomplete, everything will be updated and properly structured as the project evolves.

The `.cpp` files here are currently intended for testing DSP algorithms, and are not updated for my current real-time code.  
There are `.mp4` files located in the "Audio Demos" folder which are audio examples demonstrating the effects.



**Thank you for visiting this repository!**

---

## Table of Contents
- [Introduction](#introduction)
- [Effects](#effects)
- [Audio Demos](#audio-demos)
  - [Waveform Analysis](#waveform-analysis)
- [Sources](#sources)

---

## Introduction

Currently, I am engineering a professional-grade multi-effects pedal utilizing the Raspberry Pi platform and embedded C++. The project features various real-time DSP algorithms, modular effects architecture, and a low-latency audio pipeline within an embedded Linux environment, utilizing JACK and ALSA for advanced audio routing and I/O management. Key technical highlights include ARM-based optimization, high-quality hardware integration (ADC/DAC), efficient C++ DSP implementation, performance optimization, and waveform analysis to ensure signal integrity and reliability. This project is continually expanding my skills in embedded systems, DSP, and electrical/computer engineering; furthering both my professional development and fueling my creative expression as a musician.

---

## Effects

_This list of the effects will grow as the project develops._

Currently, the repository includes 3 digital audio effects based on real analog pedals:

- **Overdrive (Ibanez TubeScreamer 808)**
  -- [Code](Overdrive.cpp)
- **Delay (MXR Carbon Copy)**
  -- [Code](DigitalDelay.cpp)
- **Tremolo (Boss TR2)**
  -- [Code](Tremolo.cpp)

---

## Audio Demos

The `Audio Demos` folder contains example outputs for each effect:

- [Delay Input](Audio_Demos/Delay_Intput.mp3)
- [Delay Output](Audio_Demos/Delay_Output.mp3)
  
-  [Overdrive Output](Audio_Demos/Overdrive_Output.mp3)
-  [Overdrive Intput](Audio_Demos/Overdrive_Input.mp3)

-  [Tremolo Input](Audio_Demos/Overdrive_Input.mp3)
-  [Squared Tremolo Output](Audio_Demos/Overdrive_Square_Ouput.mp3)
-  [Triangular Tremolo Input](Audio_Demos/Overdrive_Triangle_Output.mp3)


---

### Waveform Analysis

## Sources

_Relevant sources, references, and inspirations will be listed here:_

- [Tube Screamer Analysis - Electrosmash](https://www.electrosmash.com/tube-screamer-analysis)
- [The Tremolo Project - Blogspot](https://tremolo-project.blogspot.com/2017/08/boss-tr-2.html)
- (Feel free to recommend any resources you think would be helpful!)

---

> _Stay tuned for more updates, improvements, and cleaner code!_
