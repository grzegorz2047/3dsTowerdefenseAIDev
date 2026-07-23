# Pipeline scen środowiskowych

Warstwa rozgrywki i warstwa wizualna są rozdzielone:

- `romfs/levels/<id>.lvl` definiuje siatkę, trasę i fale;
- `assets/scenes/<id>.scene.json` jest źródłem sceny w układzie Blendera;
- `romfs/scenes/<id>.art` jest deterministycznym plikiem wynikowym ładowanym przez grę.

Brak pliku `.art` nie blokuje poziomu. Renderer używa wtedy podstawowego motywu.

## Przepływ Blender → gra

1. W Blenderze użyj standardowego układu: `Z` do góry, przód obiektu w kierunku `-Y`.
2. Każda dekoracja musi stać na płaszczyźnie `Z=0`.
3. Zastosuj jednakową skalę na wszystkich osiach. Pochylenie `X/Y` nie jest obsługiwane; dozwolony jest tylko obrót wokół `Z`.
4. Wyeksportuj dane obiektów do JSON zgodnego z `assets/scenes/tutorial.scene.json`.
5. Wygeneruj plik gry:

```sh
python3 scripts/export_scene_art.py \
  assets/scenes/tutorial.scene.json \
  --output romfs/scenes/tutorial.art
```

`make assets` wykonuje ten krok automatycznie. `make test` uruchamia eksporter w trybie `--check` i odrzuca PR, gdy wynik w RomFS jest nieaktualny lub został ręcznie zmieniony.

## Układ współrzędnych

Źródło deklaruje `coordinate_system=blender_z_up_minus_y_forward`.

Konwersja jest jednoznaczna:

- Blender `X` → gra `X`;
- Blender `-Y` → gra `Z`;
- Blender `Z` musi wynosić `0` dla statycznej dekoracji;
- obrót Blender `Z` jest negowany do obrotu sceny gry;
- skala musi być jednorodna.

Eksporter odrzuca wartości NaN/Infinity, nieznane typy, duplikaty nazw i kolejności, pozycje poza `-4..20`, skalę poza `0.25..3.0`, pochylenie oraz sceny powyżej 48 dekoracji.

## Format źródłowy

```json
{
  "schema": 1,
  "level": "tutorial",
  "theme": "VhalPass",
  "coordinate_system": "blender_z_up_minus_y_forward",
  "objects": [
    {
      "name": "village_ruin",
      "type": "Ruin",
      "group": "ruined village landmark",
      "order": 100,
      "location": [4.1, -1.3, 0.0],
      "scale": [1.1, 1.1, 1.1],
      "rotation_euler_deg": [0.0, 0.0, 20.0]
    }
  ]
}
```

`name` i `order` muszą być unikalne. `order` zapewnia stabilny wynik niezależny od kolejności obiektów zwróconej przez Blender. `group` generuje komentarz porządkujący sekcję w `.art`.

## Format runtime

```text
level=tutorial
theme=VhalPass
prop=Ruin,4.1,1.3,1.1,-20
```

Pola `prop` oznaczają kolejno:

1. typ modularnej dekoracji;
2. pozycję X w lokalnej siatce poziomu;
3. pozycję Z;
4. skalę od `0.25` do `3.0`;
5. obrót w stopniach.

Dostępne moduły: `Pine`, `Rock`, `Ruin`, `Banner`, `Barricade`, `Wagon`, `Cliff`, `Watchtower`.

## Rendering

Dekoracje są przeliczane do geometrii podczas ładowania poziomu i łączone z terenem w jednym statycznym buforze. Nie zwiększają liczby draw calli w klatce. Skala i obrót są wypiekane w pozycjach wierzchołków.

## Budżety Old Nintendo 3DS XL

- maksymalnie 48 dekoracji na scenę;
- maksymalnie 4096 statycznych wierzchołków poziomu;
- jeden materiał oparty na vertex colors;
- jeden statyczny draw call dla terenu, punktów orientacyjnych i dekoracji;
- dekoracje nie mogą zajmować drogi ani zasłaniać pól budowy z kamery startowej.

Test kampanii parsuje scenę tutoriala, sprawdza różnorodność modułów i konserwatywnie wylicza koszt geometrii przed buildem konsolowym. Test eksportera dodatkowo sprawdza konwersję osi, stabilne sortowanie, ograniczenia transformacji oraz wykrywanie nieaktualnego pliku runtime.

## Kompozycja Przełęczy Vhal

Trzy główne punkty orientacyjne to:

- pęknięta brama najazdu na grani;
- ruiny osady z wozem i barykadami;
- cytadela z dwiema wieżami strażniczymi.

Droga ma cieplejszy bruk, pola budowy wyglądają jak kamienne platformy z błękitnym rdzeniem, a nieregularne zbocza i las rozbijają wygląd prostokątnej planszy testowej.
