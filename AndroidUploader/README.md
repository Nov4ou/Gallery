# Gallery Uploader

Android app for sending a phone photo to the STM32 Gallery TCP server.

Open this folder in Android Studio:

```text
AndroidUploader
```

Android Studio may ask to download the Android Gradle Plugin or Gradle on first
sync. Accept the prompt and then run the `app` configuration on your phone.
The project is configured for the installed `android-36.1` SDK platform.

## Phone Steps

1. Connect the phone WiFi to `STM32_AP`.
2. Open this app.
3. Keep host as `192.168.4.1` and port as `8080`.
4. Choose a category and place.
5. Pick a JPG image.
6. Tap `Send to STM32`.

The app sends the same protocol as the PC Python tool:

```text
IMGHEX:<original_jpg_size>:<file_name>\n
<hex encoded jpg bytes>
```

The sender does not wait for an ACK. After the TCP connection opens it waits
1200 ms so the STM32 can process the `CONNECT` frame, then sends the header,
waits 500 ms, and sends 1024 hex characters per chunk with a 500 ms delay
between chunks, matching the working PC Python script's pacing.

Default file name format:

```text
SCN_20260419_1805_Phone.jpg
```

Use ASCII file names only. Avoid spaces and Chinese characters in the file name.
