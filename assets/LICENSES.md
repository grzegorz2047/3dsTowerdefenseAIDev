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
| `tutorial_scene_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/tutorial.scene.json` | deterministyczna konwersja osi i transformacji | `romfs/scenes/tutorial.art` |
| `flooded_road_layout` | level / scene transforms | grzegorz2047 | Project | projektowane od zera w repo | trasa S, wyspy obronne, mokradła i ruiny | `romfs/levels/flooded_road.lvl`, `romfs/scenes/flooded_road.art` |
| `iron_ravine_layout` | level / scene transforms | grzegorz2047 | Project | projektowane od zera w repo | trzy gardła, kopalniane ruiny i wieże strażnicze | `romfs/levels/iron_ravine.lvl`, `romfs/scenes/iron_ravine.art` |
| `storm_ring_layout` | level / scene transforms | grzegorz2047 | Project | projektowane od zera w repo | trasa pierścieniowa i centralne obserwatorium | `romfs/levels/storm_ring.lvl`, `romfs/scenes/storm_ring.art` |
| `ash_gate_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/ash_gate.scene.json` | osmolona brama, popiół i połamane umocnienia | `romfs/scenes/ash_gate.art` |
| `ruined_village_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/ruined_village.scene.json` | zwarta zniszczona osada, wozy i centralny plac | `romfs/scenes/ruined_village.art` |
| `stone_bridge_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/stone_bridge.scene.json` | urwiska i monumentalny kamienny most | `romfs/scenes/stone_bridge.art` |
| `echo_valley_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/echo_valley.scene.json` | ściany doliny, drzewa i wieże sygnałowe | `romfs/scenes/echo_valley.art` |
| `last_citadel_layout` | scene transforms | grzegorz2047 | Project | `assets/scenes/last_citadel.scene.json` | dwie linie fortyfikacji i finałowa brama | `romfs/scenes/last_citadel.art` |
| `guided_rocket_defense` | procedural gameplay mesh / motion | grzegorz2047 | Project | `source/Tower.cpp`, `source/Projectile.cpp` | wyrzutnia i ograniczone skrętem naprowadzanie proceduralne | runtime tower/projectile VBO |
| `tutorial_phase_music` | procedural audio | grzegorz2047 | Project | generowane w `source/AudioSystem.cpp` | spokojna pętla przygotowania, pętla walki i ambient | runtime PCM16 buffers |
| `release_icon_banner` | image/audio | grzegorz2047 | Project | `scripts/generate_release_assets.py` | generowane deterministycznie | `build/release-assets/*` |

## Szablon nowego wpisu

```text
| `asset_id` | model/texture/audio | Autor | SPDX lub Project | URL albo ścieżka źródłowa | opis zmian | ścieżki wynikowe |
```

Dla zewnętrznego assetu do PR należy dołączyć kopię tekstu licencji, gdy licencja tego wymaga, oraz trwały adres źródła. Sam link do strony pobierania nie jest wystarczającym dowodem warunków licencji.
