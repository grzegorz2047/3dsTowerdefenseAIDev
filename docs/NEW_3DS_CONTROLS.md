# Sterowanie New Nintendo 3DS

Rozszerzone sterowanie jest opcjonalne. Gra wykrywa model konsoli przez `APT_CheckNew3DS` i aktywuje C-Stick oraz ZL/ZR tylko po poprawnym potwierdzeniu New Nintendo 3DS.

## Stary Nintendo 3DS

Podstawowe sterowanie pozostaje bez zmian:

- Circle Pad: obrót i zbliżenie kamery;
- D-Pad: wybór miejsca budowy;
- L/R: poprzedni lub następny typ wieży;
- ekran dotykowy i standardowe przyciski: wszystkie akcje rozgrywki.

Brak rozszerzonego sprzętu albo błąd wykrywania daje neutralne wejście dodatkowe i nie wpływa na logikę gry.

## New Nintendo 3DS

`SELECT+Y` przełącza dwa schematy. Kombinacja nie uruchamia sprzedaży wieży.

### KAMERA

- C-Stick: obrót i zbliżenie kamery;
- ZL/ZR: poprzedni lub następny typ wieży.

### KURSOR

- C-Stick: poprzednie lub następne miejsce budowy;
- przytrzymanie ZL/ZR: obrót kamery.

Standardowe sterowanie nadal działa w obu schematach.

## Test fizyczny

1. Uruchomić ten sam build na starym i nowym modelu 3DS.
2. Na starym modelu potwierdzić identyczne działanie Circle Pada, D-Pada, L/R i dotyku.
3. Na New 3DS sprawdzić oba schematy oraz przełączanie `SELECT+Y`.
4. Potwierdzić, że przeciwne kierunki C-Sticka nie powodują ruchu.
5. Potwierdzić, że odłączenie lub błędny odczyt rozszerzonego wejścia nie powoduje crasha ani samoczynnego ruchu.
