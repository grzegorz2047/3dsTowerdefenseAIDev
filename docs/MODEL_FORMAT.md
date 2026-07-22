# Model asset contract v1

## Cel

Kontrakt opisuje format pośredni między Blenderem a danymi używanymi przez grę. Pliki `.blend` są źródłem roboczym, ale nie są ładowane w runtime. Eksporter tworzy deterministyczny manifest JSON oraz dane mesha generowane do RomFS lub kodu C++.

## Układ współrzędnych

- prawoskrętny układ współrzędnych;
- `+Y` oznacza górę;
- `+X` oznacza prawą stronę świata;
- `+Z` oznacza przód modelu i kierunek południowy siatki mapy;
- jedna jednostka świata odpowiada szerokości jednego kafelka;
- pivot obiektu stojącego na mapie znajduje się na środku podstawy: `(0, 0, 0)`.

## Winding i culling

Trójkąty muszą mieć kolejność **counter-clockwise oglądaną od strony frontu**. Eksporter i walidator muszą odrzucać zdegenerowane trójkąty. Asset nie może wymagać wyłączenia back-face cullingu dla całej sceny.

## Format pośredni manifestu

Każdy asset ma plik `assets/manifests/<id>.asset.json`:

```json
{
  "schema": 1,
  "id": "watchtower_basic",
  "kind": "tower",
  "display_name": "Wieża strażnicza",
  "author": "grzegorz2047",
  "license": "Project",
  "source": "assets/source/watchtower_basic.blend",
  "generated": "romfs/models/watchtower_basic.mesh",
  "forward_axis": "+Z",
  "up_axis": "+Y",
  "pivot": "base_center",
  "triangle_count": 420,
  "material_count": 1,
  "texture": {
    "path": "romfs/textures/world_atlas.t3x",
    "width": 128,
    "height": 128
  },
  "bounds": {
    "min": [-0.45, 0.0, -0.45],
    "max": [0.45, 1.6, 0.45]
  }
}
```

## Wymagane pola

- `schema`: obecnie `1`;
- `id`: małe litery, cyfry i podkreślenia, unikalne w repo;
- `kind`: `tower`, `enemy`, `elite_enemy`, `boss`, `projectile`, `base`, `spawn`, `tile`, `decoration`;
- `display_name`;
- `author`;
- `license`;
- `source`;
- `generated`;
- `forward_axis`: `+Z`;
- `up_axis`: `+Y`;
- `pivot`: `base_center`, wyjątek tylko dla pocisków i efektów;
- `triangle_count`;
- `material_count`;
- `bounds.min` i `bounds.max`.

## Dane wierzchołków

Minimalny format wierzchołka:

```text
position: float32 x, y, z
color:    uint8 r, g, b, a
```

Wariant teksturowany dodaje:

```text
uv: float32 u, v
```

Normalne nie są wymagane dla pierwszego vertical slice. Jeżeli zostaną dodane, powinny być pakowane, a koszt pamięci musi zostać zmierzony.

## Indeksy

- preferowane indeksy 16-bitowe;
- mesh nie może przekraczać 65 535 unikalnych wierzchołków;
- części modelu mogą być osobnymi zakresami indeksów, ale powinny współdzielić jeden bufor i materiał;
- model wymagający więcej niż dwóch draw calli wymaga uzasadnienia.

## Budżety walidowane automatycznie

| kind | maks. tris | maks. materiałów |
| --- | ---: | ---: |
| `enemy` | 350 | 1 |
| `elite_enemy` | 600 | 1 |
| `boss` | 1200 | 2 |
| `tower` | 600 | 2 |
| `projectile` | 80 | 1 |
| `base` | 500 | 2 |
| `spawn` | 500 | 2 |
| `tile` | 120 | 1 |
| `decoration` | 150 | 1 |

## Tekstury

- wymiary muszą być potęgą dwójki;
- maksymalny rozmiar pojedynczej tekstury vertical slice: 256×256;
- preferowany wspólny atlas 128×128 lub 256×256;
- manifest bez `texture` oznacza vertex colors;
- przezroczystość musi być jawnie oznaczona w przyszłej wersji schematu.

## Bounds i skala

- `bounds.min.y` dla obiektu stojącego na ziemi powinno wynosić `0` z tolerancją eksportera;
- zakres nie może zawierać `NaN` ani nieskończoności;
- podstawowa wieża powinna mieścić się na jednym kafelku;
- przeciwnik podstawowy powinien mieć szerokość poniżej `0.9` jednostki;
- pocisk powinien mieć pivot w środku geometrycznym i może używać wyjątku `pivot: center`.

## Pipeline

1. Modelowanie w Blenderze w metrach/jednostkach odpowiadających skali gry.
2. Apply transform: scale `1,1,1`, rotation zgodna z osiami kontraktu.
3. Triangulacja i sprawdzenie normals/winding.
4. Eksport manifestu i surowych danych.
5. `python3 scripts/validate_assets.py`.
6. Generacja docelowego mesha do RomFS lub kodu.
7. Build gry i test Azahar.
8. Pomiar pamięci, draw calli i FPS.

## Stabilność schematu

Zmiana znaczenia istniejącego pola wymaga podniesienia `schema`. Dodawanie opcjonalnych pól zachowuje wersję, o ile starszy importer może je bezpiecznie zignorować.