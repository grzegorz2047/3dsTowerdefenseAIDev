# Asset licenses

Każdy asset wizualny, dźwiękowy lub font używany przez grę musi mieć wpis w tym pliku przed merge.

## Dozwolone źródła

- `Project` — wykonane od zera dla tego repozytorium przez właściciela projektu lub współtwórcę, który przekazał prawa do dystrybucji;
- `CC0-1.0` — domena publiczna / Creative Commons Zero;
- inne licencje tylko po osobnym przeglądzie warunków modyfikacji, redystrybucji, atrybucji i dystrybucji binarnej.

## Niedozwolone

- ripy z innych gier;
- pliki o nieznanym autorze lub pochodzeniu;
- assety oznaczone wyłącznie jako „free” bez tekstu licencji;
- assety, których licencja zabrania modyfikacji lub redystrybucji w paczce gry;
- modele lub tekstury wygenerowane na podstawie chronionych postaci i marek bez zgody.

## Rejestr

| ID | Typ | Autor | Licencja | Źródło | Modyfikacje | Pliki wynikowe |
| --- | --- | --- | --- | --- | --- | --- |
| `tutorial_procedural_tiles` | mesh / vertex colors | grzegorz2047 | Project | proceduralnie generowane w `source/Renderer.cpp` | brak zewnętrznego źródła | runtime VBO |
| `tutorial_watchtower` | mesh / vertex colors | grzegorz2047 | Project | zaprojektowane od zera w `source/Renderer.cpp` | podstawa, trzon, blanki i kusza | runtime VBO |
| `tutorial_raider` | mesh / vertex colors | grzegorz2047 | Project | zaprojektowane od zera w `source/Renderer.cpp` | humanoidalna sylwetka z tarczą | runtime VBO |
| `tutorial_bolt` | mesh / vertex colors | grzegorz2047 | Project | zaprojektowane od zera w `source/Renderer.cpp` | grot i drewniany trzon | runtime VBO |
| `tutorial_citadel` | mesh / vertex colors | grzegorz2047 | Project | zaprojektowane od zera w `source/Renderer.cpp` | warownia, blanki, wieżyczki i sztandar | runtime static level VBO |
| `tutorial_invasion_gate` | mesh / vertex colors | grzegorz2047 | Project | zaprojektowane od zera w `source/Renderer.cpp` | kamienna brama i świecący portal | runtime static level VBO |
| `release_icon_banner` | image/audio | grzegorz2047 | Project | `scripts/generate_release_assets.py` | generowane deterministycznie | `build/release-assets/*` |

## Szablon nowego wpisu

```text
| `asset_id` | model/texture/audio | Autor | SPDX lub Project | URL albo ścieżka źródłowa | opis zmian | ścieżki wynikowe |
```

Dla zewnętrznego assetu do PR należy dołączyć kopię tekstu licencji, gdy licencja tego wymaga, oraz trwały adres źródła. Sam link do strony pobierania nie jest wystarczającym dowodem warunków licencji.
