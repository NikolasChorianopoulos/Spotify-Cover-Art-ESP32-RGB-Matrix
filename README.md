# Spotify-Cover-Art-ESP32-RGB-Matrix

This project demonstrates how to display the cover art of the currently playing song on a 64x64 RGB LED matrix using an ESP32 microcontroller and the Spotify Web API.

Originally developed as my undergraduate thesis, the project aims to integrate IoT and multimedia by leveraging the ESP32 Devkit v1, the Waveshare 64x64 RGB LED Matrix P2, and a 5V/5A power supply to power both the ESP32 and the LED matrix.

For programming, I used Visual Studio Code with the PlatformIO extension, which provided faster upload times and real-time error handling compared to the Arduino IDE. This choice significantly enhanced the development experience and efficiency.

# Current Status
As of now, the code performs its intended functionality, but there are a few issues to address:

Display Speed: The transition speed of updates to the display needs improvement for smoother performance.

Color Correction: Some colors, particularly blacks, appear distorted with a greenish hue, requiring adjustments to ensure accurate color representation.
