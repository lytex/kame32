#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <web_template.h>
#include <kame.h>
#include <config.h>


Kame robot;
WebServer server(80);
String command = "";

int getBatteryPercent() {
    int raw = analogRead(34);
    int percent = map(raw, 0, 4095, 0, 100);
    return percent;
}

String getBatteryIcon(int percent) {
    if (percent > 80)
        return "[█████]";
    else if (percent > 60)
        return "[████ ]";
    else if (percent > 40)
        return "[███  ]";
    else if (percent > 20)
        return "[██   ]";
    else
        return "[█    ]";
}

String generateHTML(String ip, int batteryPercent) {
    String html = htmlTemplate;
    html.replace("{{IP}}", ip);
    html.replace("{{BATT_PERCENT}}", String(batteryPercent));
    html.replace("{{BATT_ICON}}", getBatteryIcon(batteryPercent));
    return html;
}

void handleRoot() {
    int battery = getBatteryPercent();
    String ip = WiFi.softAPIP().toString();
    server.send(200, "text/html", generateHTML(ip, battery));
}

void handleCmd() {
    if (server.hasArg("val")) {
        command = server.arg("val");
        Serial.println("New command: " + command);
    }
    server.send(200, "text/plain", "OK");
}

void spinner(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {
        server.handleClient();
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);
    //WiFi.mode(WIFI_STA);
    //WiFi.begin(SSID, PASSWORD);
    MDNS.begin(HOSTNAME);

    robot.init();
    // robot.loadCalibration();
    int offsetLegs = 100;
    int offsetLimbs = 10;
    // int servo_calibration[8] = {
    //     -35-offsetLimbs,
    //     10+offsetLegs-10,
    //     -50-offsetLegs,
    //     20+offsetLegs,
    //     -10+offsetLimbs,
    //     10-offsetLegs-0,
    //     15-offsetLimbs,
    //     30+offsetLimbs
    // };
    int servo_calibration[8] = {
        -10-offsetLimbs,
        30+offsetLimbs,
        -30-offsetLegs,
        -70+offsetLegs,
        -35+offsetLimbs,
        20-offsetLimbs,
        30+offsetLegs,
        50-offsetLegs,
    };

    // board_pins[0] = SERVO_4_PIN;
    // board_pins[1] = SERVO_6_PIN;
    // board_pins[2] = SERVO_1_PIN;
    // board_pins[3] = SERVO_2_PIN;
    // board_pins[4] = SERVO_0_PIN;
    // board_pins[5] = SERVO_7_PIN;
    // board_pins[6] = SERVO_5_PIN;
    // board_pins[7] = SERVO_3_PIN;

    robot.setCalibration(servo_calibration);
    robot.home();

    server.on("/", handleRoot);
    server.on("/cmd", handleCmd);
    server.begin();
}

void loop() {
    server.handleClient();
    if (command != "") {
        digitalWrite(LED_PIN, HIGH);
        if (command == "forward") {
            robot.walk(1, 500);
        } else if (command == "left") {
            robot.turnL(1, 500);
        } else if (command == "right") {
            robot.turnR(1, 500);
        } else if (command == "backward") {
            robot.backward(1, 500);
        } else if (command == "hello") {
            robot.hello();
        } else if (command == "jump") {
            robot.jump();
        } else if (command == "pushup") {
            robot.pushUp(2, 2000);
        } else if (command == "dance") {
            robot.dance(1, 500);
        } else if (command == "moonwalk") {
            robot.moonwalkL(1, 500);
        } else if (command == "frontback") {
            robot.frontBack(1, 500);
        } else {
            robot.home();
        }
        digitalWrite(LED_PIN, LOW);
    }
}