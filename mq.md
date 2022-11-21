# mq

- uebergabeparameter und check auf count
  - Der Funktionskopf der externene Routine sieht wie folgt aus
    - void addMqttMessage(int count, ydb_char_t *topic, ydb_char_t *payload)
  - Neben topic und payload im YottaDB konformen Datentyp ydb_char_t bekommt die Funktion noch einen Integer namens \textit{count} uebergeben.
  - Er wird von YottaDB vorgeschrieben
  - Der erste Uebergabeparameter jeder Externen Routine muss ein Integer sein.
  - In ihm ist gepeichert mit wie vielen Argumenten die Routine von M aus aufgerufen wurde.
  - Der Integer wird nicht in der External Call Table gelistet und kann nicht bewusst beim Aufruf der Routine in M gesetzt werden
  - Er ist vorhanden, da es moeglich ist die Routine in M mit weniger Parametern aufzurufen, als deklariert wurden.


- die attribute, wie sie gesetzt sind und wo ihre default einstellungen zu finden sind
- open aufruf und errno
  - messasge queue descriptor entspricht in linux einem fd
- Warum statische Variabel?
- Erstellen der Nachricht
- senden, msg prio egal
- Entweder VIEW NOISOLATION oder send TIMEOUT
- Unterbrechung durch Signal
  - was ist das signal?
  - warum ist ein signal handler nicht implementierbar?
    - Fuer das Signal SIGALRM darf kein handler hinzugefuegt werden
    - was fuer ein Fehler wird angezeigt

# View NOISOLATION
- muss auf jedenfall gemacht werden
- Es gibt keinen Sinn die Isolation hier zu erzwingen. In dem Trigger werden keine Veraenderungen an anderen Nodes gemacht.
- Es koennen nachrichten verloren gehen oder durch 4x Restart haengt sich ydb auf, falls der ausleseprozess ausgefallen sein sollte.
