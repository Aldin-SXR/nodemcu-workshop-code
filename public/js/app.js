let app = angular.module("node-app", [ "colorpicker.module" ]);

let client = mqtt.connect({ host: MQTT_HOST, port: MQTT_PORT, username: MQTT_USER, password: MQTT_PASSWORD });
client.on("connect", () => {
    console.log("Successful MQTT connection.");
    client.subscribe("/ldr");
    client.subscribe("/led");
});

app.controller("ledController", ($scope, $http) => {
    $scope.isOn = false;
    client.on("message", (topic, message) => {
        if (topic === "/led") {
            if (String(message) === "on") {
                $scope.isOn = true;
            }
        }
    });

    // /* Check current LED state */
    // $http.get(ESP8266_URL + "/led").then(response => {
    //     if (response.data === "on") {
    //         $scope.isOn = true;
    //     }
    // }, error => {
    //     console.log(error);
    // });

    $scope.flipLED = () => {
        $scope.isOn = !$scope.isOn;
        let power = {
            power: $scope.isOn
        }
        client.publish("/flip", JSON.stringify(power));

        // $http.post(ESP8266_URL + "/flip", power).then(response => {
        //     console.log(response);
        // }, error => {
        //     console.log(error);
        // });
    }
});

app.controller("rgbController", ($scope, $http) => {
    $scope.selectColor = (e) => {
        let color = ($scope.color).replace('(', '').replace(')', '').replace('rgb', '');
        color = {
            r: color.split(",")[0],
            g: color.split(",")[1],
            b: color.split(",")[2]
        };
        
        client.publish("/rgb", JSON.stringify(color));

        // $http.post(ESP8266_URL + "/rgb", color, {
        //     headers: { "Content-Type": "application/json" }
        // }).then(response => {
        //     console.log(response);
        // }, error => {
        //     console.log(error);
        // });
    }
});

app.controller("ldrController", ($scope,$interval, $http) => {
    $scope.voltage = "0";

    /* MQTT */
    client.on("message", (topic, message) => {
        if (topic === "/ldr") {
            $scope.voltage = String(message);
            $scope.$apply();
        }
    });
    /* HTTP */

    // $interval(() => {
    //     $http.get(ESP8266_URL + "/ldr").then(response => {
    //         $scope.voltage = response.data;
    //     }, error => {
    //         console.log(error);
    //     });
    // }, 1000);
});