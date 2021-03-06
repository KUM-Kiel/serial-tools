# Programmierung des Trillium Seismometers

## Installation der Serial-Tools

1.  Herunterladen der neusten [Binaries bei Github](https://github.com/KUM-Kiel/serial-tools/releases/latest).

2.  Entpacken und installieren der Tools:

    ```text
    $ tar xf serial-tools-2017-09-28-1.1.0-linux.tar.gz
    $ cd serial-tools-2017-09-28-1.1.0-linux/
    $ sudo make install
    ```

*Oder*

1.  Installation der benötigten Programme.

    Unter Ubuntu, Debian oder LinuxMint können folgende Befehle benutzt werden:

    ```text
    $ sudo apt-get update
    $ sudo apt-get install build-essential git
    ```

2.  Kompilieren der Serial-Tools:

    ```text
    $ git clone https://github.com/KUM-Kiel/serial-tools.git
    $ cd serial-tools/
    $ make
    $ sudo make install
    ```

## Vorbereitung

1.  Den Stecker vom Trillium lösen.

2.  Das Programmierkabel mit dem 12-poligen Subconn-Stecker des Trilliums verbinden.

3.  Einen RS232-USB-Adapter auf den 9-poligen D-Sub-Stecker des Programmierkabels stecken.

4.  Ein Batteriepack oder ein Netzteil mit passendem Stecker mit dem 2-poligen Stecker des Programmierkabels verbinden.
    Die Spannung sollte mindestens 12V betragen.

## Programmierung

1.  Linux starten.

2.  In einem Terminal folgenden Befehl eingeben:

    ```text
    $ sudo tail -f /var/log/syslog
    ```

    Falls dies nicht funktioniert, kann stattdessen dieser Befehl verwendet werden:

    ```text
    $ sudo dmesg -w
    ```

3.  Den RS232-USB-Adapter in einen USB-Port stecken.
    Im Terminal sollten jetzt eine Meldungen auftauchen, die ähnlich wie diese hier ist:

    ```text
    [26990.148454] usb 2-1.7.2.1: pl2303 converter now attached to ttyUSB1
    ```

    Tail bzw. Dmesg kann jetzt mit Ctrl+C beendet werden.

4.  Das Trillium-Programm starten.
    Hierbei benötigt man den Namen des Adapters.
    Hier im Beispiel »`ttyUSB1`«.

    ```text
    $ trillium /dev/ttyUSB1
    ```

5.  Im Terminalfenster sollte jetzt eine Nachricht wie die folgende auftauchen:

    ```text
    This is Serial-Tools Version 1.1.0 (2017-09-28).

    Leave this program running to communicate with the Trillium Seismometer.
    Once you are finished, you can close this program with Ctrl+C.

    Searching for Trillium Seismometer ...
    Trillium Compact OBS http://2.43.12.95/
    ```

    Solange das Programm läuft, ist das Trillium unter der ausgegebenen Adresse ereichbar.

6.  Im Browser kann jetzt die entsprechende URL aufgerufen werden.

7.  Nach dem Programmieren kann das Trillium-Programm ebenfalls mit Ctrl+C beendet werden.
