# 3dsTowerdefenseAIDev

Pełna trójwymiarowa gra typu tower defense dla Nintendo 3DS, tworzona jako aplikacja homebrew w C++ z użyciem devkitARM, libctru, Citro3D i Citro2D.

## Główne założenia

- trójwymiarowe mapy, wieże, przeciwnicy i pociski;
- opcjonalny stereoskopowy efekt 3D na górnym ekranie;
- sterowanie kamerą i kursorem przyciskami konsoli;
- interfejs budowania i ulepszania na dolnym ekranie dotykowym;
- kampania z poziomami do przejścia, ocenami i zapisem postępu;
- styl low-poly i budżet wydajności dostosowany do starszego Nintendo 3DS;
- docelowy format wydania `.cia` instalowany na konsoli z custom firmware;
- pomocniczy format `.3dsx` do szybkiego uruchamiania wersji deweloperskich przez Homebrew Launcher.

## Dokumentacja

- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — plan kampanii, poziomów, mechanik i architektury gry;
- [`docs/BUILD_AND_RELEASE.md`](docs/BUILD_AND_RELEASE.md) — plan budowania, pakowania i publikowania wersji `.3dsx` oraz `.cia`;
- [`docs/PERFORMANCE_TESTING.md`](docs/PERFORMANCE_TESTING.md) — powtarzalny test FPS, renderowania, pamięci i stabilności na oryginalnym Nintendo 3DS XL.

## Planowany stos technologiczny

- C++17 w zakresie wspieranym przez toolchain devkitARM;
- devkitARM / devkitPro;
- libctru;
- Citro3D do renderowania sceny 3D;
- Citro2D do interfejsu 2D;
- RomFS do danych map, poziomów, fal i zasobów;
- zapis postępu na karcie SD;
- makerom i bannertool do pakowania wydania `.cia`.

## Pierwszy cel

Pierwszym wydaniem będzie grywalne MVP zawierające samouczek i pięć poziomów kampanii, trzy rodzaje wież, trzy podstawowe klasy przeciwników, bossów, zapis postępu oraz obsługę stereoskopowego 3D.

Każdy release powinien udostępniać co najmniej:

- `CitadelDefense3D.cia` — główny artefakt dla gracza;
- `CitadelDefense3D.3dsx` — artefakt deweloperski i awaryjny;
- sumy kontrolne SHA-256;
- krótką instrukcję instalacji i listę zmian.
