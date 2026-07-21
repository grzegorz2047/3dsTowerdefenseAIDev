# 3dsTowerdefenseAIDev

Pełna trójwymiarowa gra typu tower defense dla Nintendo 3DS, tworzona jako aplikacja homebrew w C++ z użyciem devkitARM, libctru, Citro3D i Citro2D.

## Główne założenia

- trójwymiarowe mapy, wieże, przeciwnicy i pociski;
- opcjonalny stereoskopowy efekt 3D na górnym ekranie;
- sterowanie kamerą i kursorem przyciskami konsoli;
- interfejs budowania i ulepszania na dolnym ekranie dotykowym;
- kampania z poziomami do przejścia, ocenami i zapisem postępu;
- styl low-poly i budżet wydajności dostosowany do starszego Nintendo 3DS;
- docelowy format uruchomieniowy `.3dsx`.

## Dokumentacja

Pełny plan projektu, kampanii, poziomów, mechanik i architektury znajduje się w pliku [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Planowany stos technologiczny

- C++17 w zakresie wspieranym przez toolchain devkitARM;
- devkitARM / devkitPro;
- libctru;
- Citro3D do renderowania sceny 3D;
- Citro2D do interfejsu 2D;
- RomFS do danych map, poziomów, fal i zasobów;
- zapis postępu na karcie SD.

## Pierwszy cel

Pierwszym wydaniem będzie grywalne MVP zawierające samouczek i pięć poziomów kampanii, trzy rodzaje wież, trzy podstawowe klasy przeciwników, bossów, zapis postępu oraz obsługę stereoskopowego 3D.
