# Opcjonalna kamera ruchowa

Kamera ruchowa jest domyślnie wyłączona i nie zastępuje sterowania Circle Padem ani C-Stickiem.

## Sterowanie w misji

- `SELECT + GÓRA`: włącz lub wyłącz kamerę ruchową;
- `SELECT + B`: ustaw bieżące położenie konsoli jako neutralne;
- Circle Pad i pozostałe sterowanie kamerą działają równolegle.

## Zachowanie bezpieczeństwa

- sensor jest uruchamiany dopiero po świadomym włączeniu;
- pierwsza poprawna próbka służy jako automatyczna kalibracja i nie porusza kamerą;
- martwa strefa usuwa drobny dryf po odłożeniu konsoli;
- filtr wygładza nagłe wartości i ogranicza wyjście do zakresu `[-1, 1]`;
- błąd inicjalizacji lub utrata danych daje neutralne wejście i komunikat `RUCH: BRAK SENSORA`;
- sensor jest zawsze wyłączany przy opuszczeniu sesji lub niszczeniu `InputSystem`.

## Test fizyczny

1. Uruchomić misję i potwierdzić brak wpływu sensora przed włączeniem.
2. Trzymając konsolę nieruchomo, nacisnąć `SELECT + GÓRA`.
3. Delikatnie obrócić i pochylić konsolę; kamera powinna reagować płynnie.
4. Odłożyć konsolę i potwierdzić brak ciągłego dryfu.
5. Nacisnąć `SELECT + B` w nowej pozycji neutralnej i sprawdzić natychmiastowe wycentrowanie.
6. Ponownie nacisnąć `SELECT + GÓRA` i potwierdzić, że Circle Pad nadal działa, a sensor nie wpływa na kamerę.
