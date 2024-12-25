# Spotify-Cover-Art-ESP32-RGB-Matrix

This project demonstrates how to display the cover art of the currently playing song on a 64x64 RGB LED matrix using an ESP32 microcontroller and the Spotify Web API.

Originally developed as my undergraduate thesis, the project aims to integrate IoT and multimedia by leveraging the ESP32 Devkit v1, the Waveshare 64x64 RGB LED Matrix P2, and a 5V/5A power supply to power both the ESP32 and the LED matrix.

For programming, I used Visual Studio Code with the PlatformIO extension, which provided faster upload times and real-time error handling compared to the Arduino IDE. This choice significantly enhanced the development experience and efficiency. The code is written in C++.

# How it works
The core functionality of the code revolves around leveraging the Spotify API to fetch JSON data for the currently playing song. From this data, we extract only the URL of the cover art. Specifically, we utilize the third URL provided, as it is already formatted to 64x64 pixels, eliminating the need for further resizing.

Using the [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder) library, the cover art image is decoded into an RGB format that the matrix can interpret. The [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) library is then employed to render the converted image onto the RGB LED matrix.

The code operates within a loop that continuously checks whether the currently playing song has changed. When a change is detected, it updates the URL and refreshes the image displayed on the matrix accordingly. The code also incorporates a mechanism to refresh the authorization token required for accessing the Spotify API. This ensures uninterrupted functionality by automatically renewing the token when it expires, maintaining a seamless connection to retrieve the currently playing song's data.

Included in the project files is a Python script designed to generate both an authorization token and a refresh token. The authorization token has a validity of approximately one hour, after which it will need to be refreshed to maintain access to the Spotify API. The refresh token simplifies this process by enabling automated token renewal.








# Current Status
As of now, the code performs its intended functionality, but there are a few issues to address:

Display Speed: The transition speed of updates to the display needs improvement for smoother performance.

Color Correction: Some colors, particularly blacks, appear distorted with a greenish hue, requiring adjustments to ensure accurate color representation.


**It is important to note that this project has only been tested with the ESP32 Devkit v1 and the Waveshare 64x64 RGB LED Matrix P2. Compatibility with other microcontrollers or LED matrices has not been verified and may require additional testing or modifications.**


