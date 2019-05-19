# NodeMCU IoT Workshop

In this repository, you will find all code discussed and displayed during the NodeMCU IoT workshop held on May 18th, 2019 (Sarajevo).

The `src` folder contains the NodeMCU (C++) code, whereas the `public` folder contains the client-side web page (HTML + JavaScript).

You can test this code out by downloading it, and importing it via PlatformIO. (`PIO Home` => `Import Arduino Project`).

Please note that the actual configuration files (`config.js` and `config.h`) have not been provided directly. Instead, rename the `config.sample` files (remove the `sample` extension), and supply your own data, if you want to test out the project.

```cpp
/*  Rename this file to config.h and supply your values */

const char* wifi_ssid = "";
const char* wifi_password = "";
const char* mqtt_user = "";
const char* mqtt_password = "";
```

```js
/* Rename this file to config.js and supply your values */

const ESP8266_URL = "";
const MQTT_USER = "";
const MQTT_PASSWORD = "";
const MQTT_HOST = "";
const MQTT_PORT = 1883;
```

Feel free to contact me for any issues or further questions.