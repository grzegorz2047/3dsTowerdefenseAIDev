# Budowanie i wydawanie gry

## Docelowe artefakty

Projekt ma tworzyć dwa formaty:

1. `CitadelDefense3D.cia` — podstawowy artefakt release instalowany na konsoli Nintendo 3DS z odpowiednio skonfigurowanym środowiskiem homebrew/custom firmware.
2. `CitadelDefense3D.3dsx` — pomocniczy artefakt deweloperski do szybkich testów przez Homebrew Launcher.

Format `.cia` jest produktem końcowym. Format `.3dsx` pozostaje ważny, ponieważ skraca pętlę testowania i ułatwia diagnostykę bez każdorazowej instalacji paczki.

## Łańcuch budowania

Planowany pipeline:

1. kompilacja kodu C++ przy użyciu devkitARM i reguł `3ds_rules`;
2. kompilacja shaderów PICA;
3. konwersja tekstur i zasobów;
4. utworzenie pliku ELF;
5. utworzenie wersji `.3dsx` wraz z SMDH i RomFS;
6. wygenerowanie ExeFS, RomFS, ikony, bannera i metadanych wymaganych przez wydanie instalowalne;
7. spakowanie wydania `.cia`;
8. obliczenie sum SHA-256;
9. publikacja obu artefaktów w GitHub Release.

## Metadane aplikacji

Robocze wartości:

- nazwa: `Citadel Defense 3D`;
- autor: `grzegorz2047`;
- opis: `A fully 3D tower defense game for Nintendo 3DS`;
- produkt docelowy: aplikacja homebrew;
- wersjonowanie: semantyczne, np. `0.1.0`;
- identyfikator tytułu: zostanie wybrany przed pierwszym publicznym wydaniem i zapisany w jednym pliku konfiguracyjnym.

Identyfikator tytułu nie może być zmieniany pomiędzy zwykłymi aktualizacjami, ponieważ konsola potraktowałaby nową paczkę jako inną aplikację.

## Zasoby wymagane przez `.cia`

- ikona aplikacji;
- banner;
- plik SMDH lub równoważne metadane;
- RomFS z mapami, falami, teksturami, dźwiękami i lokalizacją;
- ExeFS z kodem gry;
- konfiguracja pakowania przechowywana w repozytorium;
- żadne prywatne klucze ani dane pochodzące z komercyjnego SDK nie mogą trafić do repozytorium.

## Tryby pipeline

### Pull request

- kompilacja kodu;
- walidacja danych poziomów;
- testy hostowe logiki niezależnej od sprzętu;
- utworzenie `.3dsx`;
- opcjonalne utworzenie testowego `.cia`, jeżeli środowisko CI zawiera wyłącznie legalne narzędzia open-source i nie wymaga sekretów.

### Merge do `main`

- pełna kompilacja;
- artefakty diagnostyczne;
- build `.3dsx` i `.cia`;
- brak automatycznej publikacji wersji użytkowej bez taga.

### Tag `v*`

- czysty build od zera;
- utworzenie `.cia` i `.3dsx`;
- sumy SHA-256;
- utworzenie GitHub Release;
- dołączenie instrukcji instalacji i changelogu.

## Instalacja i aktualizacje

Dokumentacja release powinna jasno informować, że:

- `.cia` jest instalowane przez narzędzie dostępne na odpowiednio skonfigurowanej konsoli;
- użytkownik wykonuje instalację na własnym urządzeniu i odpowiada za jego konfigurację;
- aktualizacja powinna zachować ten sam identyfikator tytułu;
- zapis gry musi być zgodny wstecznie albo migrowany przed wydaniem nowej wersji;
- przed wersją `1.0.0` zgodność zapisu może być ograniczona, ale każda taka zmiana musi zostać opisana.

## Kryteria ukończenia pipeline release

- pojedyncze polecenie lokalne buduje `.3dsx`;
- pojedyncze polecenie lokalne buduje `.cia`;
- CI odtwarza build w świeżym środowisku;
- release zawiera oba formaty i sumy kontrolne;
- instalacja `.cia` na rzeczywistym sprzęcie uruchamia grę;
- zamknięcie i ponowne uruchomienie gry zachowuje zapis;
- repozytorium nie zawiera materiałów objętych NDA ani prywatnych kluczy.
