# Art direction — Citadel Defense 3D

## Cel wizualny

Gra ma wyglądać jak czytelna, ręcznie stylizowana makieta fantasy. Priorytetem nie jest realizm, lecz natychmiastowe rozpoznanie funkcji obiektu na ekranie 400×240 oraz stabilna wydajność na Old Nintendo 3DS.

## Zasady

1. **Silna sylwetka** — wieża, przeciwnik i baza muszą być rozpoznawalne bez tekstury i z odległości kamery używanej w grze.
2. **Mało drobnych detali** — detale mniejsze niż kilka pikseli zastępujemy zmianą bryły, koloru lub kontrastu.
3. **Kolor oznacza funkcję** — droga, pole budowy, spawn, baza, przeciwnik i aktywny wybór mają osobne, stabilne rodziny kolorów.
4. **Jeden kierunek światła zapisany w kolorach** — styl używa prostego vertex color / hand-painted shading zamiast kosztownego oświetlenia per-pixel.
5. **Ograniczona paleta** — środowisko jest stonowane, obiekty interaktywne są jaśniejsze i bardziej nasycone.
6. **Brak realizmu materiałowego** — metal, drewno i kamień rozróżnia kształt, paleta oraz kilka prostych plam koloru.

## Paleta funkcjonalna

- teren: zielenie o niskim nasyceniu;
- droga: ciepłe brązy;
- pola budowy i konstrukcje gracza: błękity;
- przeciwnicy: magenta/czerwienie;
- spawn: jasna zieleń;
- baza: czerwony akcent;
- zaznaczenie poprawne: turkus/zielony;
- zaznaczenie błędne: pomarańcz/czerwień.

Kolory nie mogą być jedynym nośnikiem znaczenia. Spawn, baza, wieża i przeciwnik muszą mieć odmienne sylwetki.

## Budżety geometrii

| Typ | Docelowy budżet |
| --- | ---: |
| Przeciwnik podstawowy | 150–350 tris |
| Elitarny przeciwnik | 350–600 tris |
| Boss | 700–1200 tris |
| Wieża podstawowa | 250–600 tris |
| Pocisk | 12–80 tris |
| Baza / spawn | 150–500 tris |
| Dekoracja | 20–150 tris |

Model przekraczający budżet wymaga uzasadnienia i pomiaru na Old 3DS/Azahar.

## Tekstury i materiały

- preferowany wspólny atlas 128×128 lub 256×256;
- pojedynczy model powinien używać jednego materiału, wyjątkowo dwóch;
- tekstury są opcjonalne — proste obiekty korzystają z vertex colors;
- bez normal map, PBR i dużych tekstur;
- filtrowanie nearest lub delikatne linear zależnie od czytelności;
- przezroczystość tylko dla efektów, nie dla podstawowej geometrii.

## Animacja

Pierwszy pionowy zestaw nie używa pełnego szkieletu. Ruch powstaje przez transformacje części:

- obrót głowicy/lufy wieży;
- unoszenie kryształu lub ramienia;
- kołysanie korpusu przeciwnika;
- skalowanie/obrót pocisku;
- proste klatki vertex animation dopiero po pomiarze kosztu.

Skeletal animation jest zarezerwowane dla modeli, których nie da się czytelnie animować prostszą metodą.

## Pierwszy pionowy zestaw

1. **Wieża strażnicza** — kamienna podstawa, drewniany korpus, niebieski dach/akcent gracza, obrotowa głowica.
2. **Najeźdźca** — niski korpus, szerokie ramiona, magentowy element rozpoznawczy, wyraźny kierunek przodu.
3. **Bełt/pocisk** — wydłużona bryła z jasnym grotem.
4. **Baza** — masywniejsza bryła z czerwonym rdzeniem/celownikiem.
5. **Spawn** — niski portal lub brama z zielonym akcentem.
6. **Kafelki** — ground, road, build spot i blocked jako proste moduły o zgodnych krawędziach.

## Źródła i licencje

- kluczowe modele tworzymy od zera dla projektu;
- zewnętrzne assety mogą być użyte wyłącznie przy jednoznacznej licencji pozwalającej na modyfikację i redystrybucję;
- preferowane CC0;
- każdy zewnętrzny plik musi mieć wpis w `assets/LICENSES.md`;
- zabronione są ripy z gier, modele z niejasnym pochodzeniem i assety wymagające dystrybucji bez zmian, jeżeli pipeline je modyfikuje.

## Kryteria jakości

Asset jest gotowy do gry, gdy:

- mieści się w budżecie tris i materiałów;
- ma poprawny pivot, skalę oraz winding;
- jest czytelny z kamery startowej;
- nie znika przy back-face culling;
- ma udokumentowaną licencję;
- przechodzi walidator assetów;
- nie powoduje istotnego spadku FPS ani nadmiernego wzrostu pamięci.