# Stały audyt sprzętowy Nintendo 3DS

Ten dokument jest obowiązkowym kontraktem przeglądu zmian wpływających na renderer, UI, zasoby, sceny, modele, tekstury, dotyk, stereoskopię lub wydajność. Zielony build i uruchomienie w emulatorze nie zastępują testu na fizycznej konsoli.

Powiązany tracker: #192.

## Profile urządzeń

### Old Nintendo 3DS / 3DS XL — profil bazowy

Profil Old 3DS jest kryterium dopuszczenia gry. Cała podstawowa kampania, oba ekrany, dotyk, dźwięk i stereoskopia muszą działać bez funkcji dostępnych wyłącznie na New 3DS.

Build otrzymuje `FAIL`, jeżeli na Old 3DS występuje crash, trwały wyciek pamięci, nieczytelny lub niedostępny dotykowo interfejs albo przekroczenie progów z `PERFORMANCE_TESTING.md`.

### New Nintendo 3DS / New 3DS XL — profil rozszerzony

Profil New 3DS jest raportowany osobno. C-Stick, ZL/ZR i dodatkowy zapas CPU mogą poprawiać komfort, ale nie mogą odblokowywać mechaniki wymaganej do ukończenia kampanii ani maskować regresji profilu Old 3DS.

## Trzy niezależne rodzaje dowodu

1. **Walidacja automatyczna** — testy hostowe, statyczne budżety, build `.3dsx` i `.cia` oraz smoke test Azahar.
2. **Przegląd obrazu** — PNG z wymaganych ekranów i stanów. Zrzut emulatora ocenia układ oraz czytelność, ale nie potwierdza komfortu fizycznej stereoskopii.
3. **Test fizyczny** — pomiar na konsoli według `PERFORMANCE_TESTING.md`. Tylko ten etap potwierdza realny budżet sprzętu, dotyk, suwak 3D i komfort obrazu.

Żaden z tych dowodów nie zastępuje pozostałych.

## Kiedy audyt jest obowiązkowy

Pełna checklista jest wymagana, gdy PR zmienia co najmniej jeden z obszarów:

- `source/Renderer.cpp`, shadery lub konfigurację render targetów;
- `UiRenderer`, układ dolnego albo górnego ekranu lub obszary dotykowe;
- modele, dekoracje, tekstury, atlasy, PNG, RomFS lub sceny map;
- liczbę przeciwników, wież, pocisków albo efektów widocznych w jednej klatce;
- stereoskopię, kamerę, fizyczny suwak 3D lub obsługę Old/New 3DS;
- alokacje liniowe, transfery tekstur, cykl życia zasobów albo benchmark;
- pipeline release i sposób generowania `.cia` lub `.3dsx`.

Dla zmian czysto logicznych autor może oznaczyć sekcję jako `N/D`, ale musi krótko uzasadnić brak wpływu sprzętowego.

## Checklista kodu i architektury

### CPU i pętla klatki

- [ ] brak nieograniczonej pracy zależnej od liczby obiektów;
- [ ] brak nowych alokacji sterty lub pamięci liniowej wykonywanych stale w pętli klatki;
- [ ] koszt aktualizacji przeciwników, wież, pocisków i UI pozostaje ograniczony przez jawne pojemności;
- [ ] logika podstawowej rozgrywki nie zależy od speedupu New 3DS;
- [ ] długie operacje ładowania są wykonywane poza ustabilizowanym oknem pomiarowym.

### PICA200 i geometria

- [ ] liczba wierzchołków mapy mieści się w `PerformanceBudget::kMaximumLevelVertices`;
- [ ] nowe modele są low-poly i nie powielają niepotrzebnie tej samej geometrii;
- [ ] liczba draw calli rośnie świadomie i jest opisana w PR;
- [ ] stereo nie tworzy dodatkowych zasobów co klatkę;
- [ ] stan GPU jest współdzielony tam, gdzie nie zmienia wyniku renderowania.

### Pamięć liniowa i zasoby

- [ ] każda udana `linearAlloc` ma jednoznacznie sparowane `linearFree`;
- [ ] częściowo nieudana inicjalizacja zwalnia wcześniej utworzone zasoby;
- [ ] wejście i wyjście z misji co najmniej 10 razy nie powoduje narastającego spadku wolnej pamięci;
- [ ] zachowany jest minimalny zapas pamięci liniowej z `PERFORMANCE_TESTING.md`;
- [ ] rozmiar nowych zasobów i koszt ich jednoczesnego przebywania w pamięci są znane.

### Tekstury i transfery

- [ ] format i rozmiar tekstur są dostosowane do 3DS oraz narzędzi `tex3ds`;
- [ ] atlas jest preferowany, gdy zmniejsza liczbę zmian stanu i transferów;
- [ ] tekstury nie są ponownie przesyłane w każdej klatce bez udokumentowanej potrzeby;
- [ ] nowe PNG nie powodują nieuzasadnionego wzrostu RomFS ani pamięci roboczej;
- [ ] filtrowanie, przezroczystość i krawędzie są sprawdzone w natywnej rozdzielczości ekranu.

### Oba ekrany, dotyk i stereo

- [ ] górny ekran pozostaje czytelny w mono i stereo;
- [ ] dolny ekran nie ma tekstu, przycisków ani wskaźników poza bezpiecznym obszarem;
- [ ] każdy widoczny przycisk dolnego ekranu ma zgodny obszar dotykowy i stan aktywny/wciśnięty;
- [ ] fizyczny suwak 3D jest jedynym źródłem decyzji o renderowaniu drugiego oka;
- [ ] przy suwaku 0% renderowane jest jedno oko, a przy wartości większej od zera dwa oka;
- [ ] HUD, tekst i znaczniki nie rozjeżdżają się pomiędzy lewym i prawym okiem;
- [ ] maksymalna głębia nie powoduje podwójnych krawędzi ani dyskomfortu.

## Wymagany pakiet PNG

Dla każdej zmiany wizualnej należy dołączyć lub wskazać artefakt zawierający co najmniej:

1. ekran kampanii;
2. briefing misji;
3. konfigurator benchmarku;
4. obciążoną misję z HUD-em;
5. pauzę;
6. zwycięstwo lub porażkę;
7. ekran wyniku benchmarku;
8. ekran bez stereo oraz porównywalny stan stereo, z adnotacją o ograniczeniach emulatora.

Każdy PNG jest oceniany pod kątem:

- ucięcia i nakładania tekstu;
- kontrastu i czytelności w rozdzielczości 400×240 lub 320×240;
- poprawnego wyróżnienia zaznaczenia, wciśnięcia i niedostępności;
- zgodności wizualnego przycisku z obszarem dotykowym;
- spójności HUD-u pomiędzy ekranami i oczami;
- widoczności ważnych obiektów bez zasłaniania trasy i celów.

## Pomiar fizyczny

Wykonaj pełny scenariusz z `PERFORMANCE_TESTING.md` na Old 3DS. Dla New 3DS powtórz ten sam profil i zapisz wynik osobno.

Minimalny zapis wyniku:

```text
Commit/tag:
SHA-256 CIA:
SHA-256 3DSX:
Model konsoli: Old / New, dokładny wariant:
Wersja systemu/CFW:
Format uruchomienia: CIA / 3DSX:
Profil benchmarku i poziom obciążenia:
Tryb CPU New 3DS: włączony / wyłączony / N/D:

MONO
FPS / AVG / MAX / RENDER / MEM:

STEREO
FPS / AVG / MAX / RENDER / MEM:
Komfort obrazu i widoczne artefakty:

10 wejść bez crasha i trwałego spadku MEM: TAK / NIE
Dotyk całego widocznego UI: PASS / WARN / FAIL
Oba ekrany: PASS / WARN / FAIL
Wynik końcowy urządzenia: PASS / WARN / FAIL
Uwagi:
```

## Klasyfikacja wyniku

- **PASS** — wszystkie wymagane testy przechodzą, a progi są zachowane.
- **WARN** — nie ma blokującej regresji, ale istnieje mierzalny spadek zapasu, problem czytelności albo brak części dowodu wymagający świadomej decyzji.
- **FAIL** — crash, wyciek, niedostępna podstawowa funkcja, przekroczony budżet Old 3DS, błędne stereo, niesprawny dotyk lub brak możliwości ukończenia podstawowego przebiegu.

`FAIL` na Old 3DS blokuje merge/release. Wynik New 3DS nie zmienia tej decyzji.

## Telemetria docelowa

Issue #192 pozostaje otwarte do czasu, gdy benchmark będzie raportował co najmniej:

- czas logiki CPU;
- osobny czas lewego i prawego oka;
- koszt UI górnego i dolnego ekranu;
- oczekiwanie/synchronizację końca klatki;
- draw calle i przesłane wierzchołki;
- bieżący i minimalny zapas pamięci liniowej;
- uploady i transfery tekstur;
- surowy stan suwaka 3D, liczbę oczu i separację;
- profil Old/New 3DS oraz stan rozszerzonego trybu CPU.
