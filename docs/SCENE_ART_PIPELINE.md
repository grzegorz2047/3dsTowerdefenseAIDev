# Pipeline scen środowiskowych

Warstwa rozgrywki i warstwa wizualna są rozdzielone:

- `romfs/levels/<id>.lvl` definiuje siatkę, trasę i fale;
- `romfs/scenes/<id>.art` definiuje motyw oraz statyczne dekoracje.

Brak pliku `.art` nie blokuje poziomu. Renderer używa wtedy podstawowego motywu.

## Format sceny

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

Test kampanii parsuje scenę tutoriala, sprawdza różnorodność modułów i konserwatywnie wylicza koszt geometrii przed buildem konsolowym.

## Kompozycja Przełęczy Vhal

Trzy główne punkty orientacyjne to:

- pęknięta brama najazdu na grani;
- ruiny osady z wozem i barykadami;
- cytadela z dwiema wieżami strażniczymi.

Droga ma cieplejszy bruk, pola budowy wyglądają jak kamienne platformy z błękitnym rdzeniem, a nieregularne zbocza i las rozbijają wygląd prostokątnej planszy testowej.
