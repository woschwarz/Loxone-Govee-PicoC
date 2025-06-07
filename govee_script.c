/*
 *********************************************
 Govee PicoC integration for Loxone Miniserver
 *********************************************
 Version 25.0.1
 www.github.com/woschwarz
 *********************************************
 I1 = On/Off
 I2 = RGB
 I3 = Dimmming 0-100%
 I4 = Farbtemperatur 2000-9000 in Kelvin
 *********************************************
*/

#define ENABLE_DEBUGGING (0)  

// Standard-IP und Port des LED-Stripe (oder über T1 setzen)
char *IP_ADDRESS = "192.168.1.146";
char *PORT       = "4003";

// Letzter bekannter Zustand, damit nur bei Änderung gesendet wird
int lastOnOffState = -1;
int lastBrightness = 0;
int lastRGB = -1;
int lastColorTemp = -1;
char lastScene[201];  // Puffer für letzte Szene, max. 200 Zeichen + '\0'

// UDP-Nachricht an Govee senden
void SendCommand(char* json) {
    char streamname[100];
    char txBuffer[1024];

    // UDP-Stream zur Govee-IP erzeugen
    sprintf(streamname, "/dev/udp/%s/%s/", IP_ADDRESS, PORT);
    STREAM *TcpStream = stream_create(streamname, 0, 0);
    if (TcpStream == NULL) {
        if (ENABLE_DEBUGGING) {
            printf("Creating Stream failed");
        }
        return;
    }

    // JSON senden
    sprintf(txBuffer, "%s", json);
    if (ENABLE_DEBUGGING) {
        printf("%s", txBuffer);
    }
    stream_write(TcpStream, txBuffer, strlen(txBuffer));
    stream_flush(TcpStream);
    stream_close(TcpStream);
}

// Gerät ein- oder ausschalten
void SetOnOff(int state) {
    char json[50];
    if (state == 0) {
        sprintf(json, "{\"msg\":{\"cmd\":\"turn\",\"data\":{\"value\":0}}}");
        setoutput(0, 0); // Light is Off
    } else {
        sprintf(json, "{\"msg\":{\"cmd\":\"turn\",\"data\":{\"value\":1}}}");
        setoutput(0, 1); // Light is On
    }
    SendCommand(json);
}

// Helligkeit setzen
void SetBrightness(int brightness) {
    char json[100];

    // Begrenzen auf gültigen Bereich
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;

    sprintf(json, "{\"msg\":{\"cmd\":\"brightness\",\"data\":{\"value\":%d}}}", brightness);
    SendCommand(json);
}

// Farbe und Farbtemperatur setzen
void SetColor(int rgbInt, int colorTemp) {
    if (rgbInt == 0) {
        SetOnOff(0); // Ausschalten wenn Farbwert 0
        return;
    }

    // Extrahiere R, G, B aus dem Integer (jede Komponente belegt 3 Dezimalstellen)
    int blue  = rgbInt / 1000000;
    int green = (rgbInt - (blue * 1000000)) / 1000;
    int red   = rgbInt - (blue * 1000000) - (green * 1000);

    // Skaliere Prozentwerte (0–100) in 8-Bit-Werte (0–255)
    int r = (red * 255) / 100;
    int g = (green * 255) / 100;
    int b = (blue * 255) / 100;

    // Farbtemperatur begrenzen
    if (colorTemp != 0) {
        if (colorTemp < 2000) colorTemp = 2000;
        if (colorTemp > 9000) colorTemp = 9000;
    }

    // JSON für colorwc senden
    char json[200];
    sprintf(json, "{\"msg\":{\"cmd\":\"colorwc\",\"data\":{\"color\":{\"r\":%d,\"g\":%d,\"b\":%d},\"colorTemInKelvin\":%d}}}", r, g, b, colorTemp);
    SendCommand(json);
    setoutput(0, 1); // Light is On
}

// Szene setzen (ptReal-Befehl)
void SetScene(char *scene) {
    char json[255];
    sprintf(json, "{\"msg\":{\"cmd\":\"ptReal\",\"data\":{\"command\":[\"%s\"]}}}", scene);
    SendCommand(json);
}

// Hauptschleife
void main() {
    char sceneBuf[256];  // Zwischenspeicher für Szene (Inputtext)

    while (1) {
        // Falls IP-Adresse über TextInput 1 (T1) gesetzt wird, diese verwenden
        char *input_ip = getinputtext(0);
        if (input_ip != NULL && strlen(input_ip) > 0) {
            IP_ADDRESS = input_ip;
        }
        
        // Eingang 0: Ein/Aus
        int inputState = getinput(0);
        if (inputState != lastOnOffState) {
            SetOnOff(inputState);
            lastOnOffState = inputState;
        }

        // Eingang 1: RGB Farbwerte
        int rgbInt = getinput(1);
        if (rgbInt != lastRGB) {
            SetColor(rgbInt, 0); // Nur Farbwert, ohne Farbtemperatur Anpassung
            lastRGB = rgbInt;
            setoutput(1, rgbInt); // Weitergabe Farbwert an Output 1 (O2)
        }

        // Eingang 2: Helligkeit (0–100)
        int brightness = getinput(2);
        if (brightness != lastBrightness) {
            SetBrightness(brightness);
            lastBrightness = brightness;
            setoutput(2, brightness); // Weitergabe Helligkeit an Output 2 (O3)
        }

        // Eingang 3: Farbtemperatur (2000–9000)
        int colorTemp = getinput(3);
        if (colorTemp != lastColorTemp) {
            SetColor(rgbInt, colorTemp); // Farbwert bleibt gleich, nur Farbtemperatur ändern
            lastColorTemp = colorTemp;
        }

        // Experimentel: ptReal Command (String über Text Input 3)
        // Achtung! Miniserver stürzt ab, wenn String zu lang ist!
        char *input_szene = getinputtext(2);

        // Szene nur übernehmen, wenn sie gültig ist und nicht zu lang
        if (input_szene != NULL && strlen(input_szene) > 0 && strlen(input_szene) <= 200) {
            strncpy(sceneBuf, input_szene, sizeof(sceneBuf) - 1);
            sceneBuf[sizeof(sceneBuf) - 1] = '\0';

            if (strcmp(sceneBuf, lastScene) != 0) {
                SetScene(sceneBuf); // ptReal-Befehl senden
                strncpy(lastScene, sceneBuf, sizeof(lastScene) - 1);
                lastScene[sizeof(lastScene) - 1] = '\0';
                //setoutputtext(2, sceneBuf); // Weitergabe Szene an Text Output 3 (Txt3)
            }
        }
        
        free(input_ip);
        free(input_szene);

        // 100 ms Pause, um Loxone-CPU zu entlasten
        sleep(100);
    }
}
main();
