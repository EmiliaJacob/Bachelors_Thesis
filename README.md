# ydbay

Im Rahmen dieser Bachelorarbeit wurden drei Implementationen erarbeitet, welche die Änderung eines Datensatzes in YottaDB anderswo spiegeln können.

Die Implementationen wurden im Kontext eines vereinfachten Online Auktionssystems umgesetzt.
Bei der Änderung des Titels eines Angebots in der Datenbank werden die Daten automatisch auch im Frontend aktualisiert.

Alle drei Implementationen verwenden YottaDBs Trigger Mechanik, um die Synchronisation der Daten auszulösen.

Bei der ersten Implementation werden die neuen Daten im Trigger in eine POSIX Message Queue gesendet.
Das geschieht mithilfe einer externen C Routine. 
In einem Mosquitto Plugin wurde eine Callback Funktion für das Tick Event des Brokers hinzugefügt.
Bei jedem Aufruf können im Plugin Daten aus der Message Queue empfangen werden.
Diese Daten werden per MQTT Nachrichten an das Frontend gepublisht um die Synchronisation abzuschließen.

Bei der nächsten Implementation werden die neuen Daten einer globalen YottaDB Variable angehängt. 
Auch hier werden die Daten über die Tick Callback Funktion des Mosquitto Plugins ausgelesen und in MQTT Nachrichten gepublisht.

In der letzten Implementationsform wurde, mit Hilfe der Mosquitto Library, ein MQTT Client direkt im Trigger erstellt.
Hierfür wurde ebenfalls eine externe C Routine verwendet.
Durch diesen Client werden die neuen Daten direkt als MQTT Nachrichten gepublisht.

Zum Vergleich der Implementationsarten wurde die Dauer der Synchronisation gemessen.
Es befindet sich im Projekt eine Implementation zur Zeitmessung in Nanosekunden.
