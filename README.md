# Loxone-Govee-PicoC
Dieses Script wurde erstellt um Govee Leuchten mit einem Loxone Miniserver zu steuern.

## Voraussetzungen
- Loxone Miniserver mit Admin Zugriff
- Govee Leuchte die mit lokalen API Zugriff funktioniert (kompatible Geräte unter https://app-h5.govee.com/user-manual/wlan-guide)

## Kurzanleitung
- In der Govee Home App die lokale API aktivieren (siehe ttps://app-h5.govee.com/user-manual/wlan-guide)   
- Danach im Router oder einen LAN-Scanner die IP-Adresse der Govee Leuchte herausfinden.  
- In der Loxone Config einen Programm Baustein einfügen und das Script hinein kopieren.
- Im Script die IP-Adresse anpassen auf die der eigenen Govee Leuchte `char *IP_ADDRESS = "192.168.1.146";`
- Der Port 4003 muss normalerweise nicht geändert werden.
- Baustein Lichtsteuerung von LC1 auf I2 verbinden.
- Im Lichtbaustein unter Lichtkreise den LC1 auf Typ RGB setzen.
- Danach nach belieben Stimmungen erstellen (oder später in der App)
- In den Loxone Miniserver speichern und testen.

## Beschreibung
### Eingänge
I1 = On/Off  
I2 = RGB Wert (über Lichtsteuerung)  
I3 = Dimming 0-100%  
I4 = Farbtemperatur 2000-9000 in Kelvin  

### Ausgänge
O1 = On = 1, Off = 0  
O2 = RGB Wert  
O3 = Dimmwert  
Etxt = Error Text  

## Alternative Möglichkeit über Virtuellen Ausgang (ohne Script)
Möchtest du lediglich die Govee LED ein- und ausschalten können. Dann läßt sich das auch ganz einfach über einen Virtuellen Ausgang einrichten.

- API wie oben beschrieben in der Govee APP aktivieren
- In der Loxone Config einen Virtuellen Ausgang erstellen.
    - Als Adresse eintragen: `/dev/udp/IP-der-Govee-Leuchte/4003/`
- Danach einen Virtueller Ausgang Befehl erstellen
    - Befehl bei EIN: `{ "msg": { "cmd": "turn", "data":{"value": 1 }}}`
    - Befehl bei AUS: `{ "msg": { "cmd": "turn", "data":{"value": 0 }}}`