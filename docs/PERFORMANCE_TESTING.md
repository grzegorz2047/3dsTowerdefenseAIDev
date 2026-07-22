# Test wydajności na oryginalnym Nintendo 3DS XL

Ten dokument opisuje powtarzalny pomiar wydajności gry na docelowym sprzęcie projektu. Wynik z emulatora nie zastępuje tego testu.

## Wymagane środowisko

- oryginalny Nintendo 3DS XL, nie New Nintendo 3DS;
- najnowszy artefakt `CitadelDefense3D.cia` albo `CitadelDefense3D.3dsx` z tego samego commita;
- konsola uruchomiona ponownie przed testem;
- brak aplikacji działających w tle;
- dźwięk włączony;
- prędkość gry `1x`;
- zapisany model konsoli, wersja systemu i sposób uruchomienia CIA/3DSX.

## Uruchomienie scenariusza

1. Uruchom grę i zaczekaj na menu kampanii.
2. Naciśnij jednocześnie `SELECT+X`.
3. Potwierdź, że tytuł misji to `TEST WYDAJNOSCI`.
4. Zbuduj co najmniej jedną wieżę i rozpocznij falę przyciskiem `X`.
5. Przytrzymaj `SELECT`, aby wyświetlić diagnostykę.
6. Nie zmieniaj kamery ani prędkości podczas właściwego okna pomiarowego.

Test nie zapisuje gwiazdek, wyników ani odblokowań kampanii.

## Pomiar mono

1. W menu wyłącz 3D przyciskiem `L` albo ustaw fizyczny suwak 3D na zero.
2. Uruchom scenariusz stresowy.
3. Zaczekaj, aż wszyscy przeciwnicy zostaną utworzeni.
4. Przytrzymaj `SELECT` przez co najmniej 10 sekund.
5. Zapisz wartości:
   - `FPS`;
   - `AVG` — średni czas klatki;
   - `MAX` — najgorszy czas klatki;
   - `RENDER` — czas renderowania ostatniej klatki;
   - `MEM` — wolna pamięć liniowa.

## Pomiar stereo

1. W menu włącz 3D przyciskiem `L`.
2. Ustaw limit głębi na 100% przyciskiem `R`.
3. Ustaw fizyczny suwak 3D na maksimum.
4. Powtórz ten sam przebieg i zapisz te same wartości.
5. Dodatkowo oceń komfort obrazu, podwójne krawędzie i stabilność paralaksy.

## Progi

| Metryka | Cel | Ostrzeżenie |
|---|---:|---:|
| Średnia klatka | `<= 33,33 ms` | `> 33,33 ms` |
| Najgorsza klatka | `<= 36 ms` | `> 36 ms` |
| Render mono | `<= 18 ms` | `> 18 ms` |
| Render stereo | `<= 28 ms` | `> 28 ms` |
| Wolna pamięć liniowa | `>= 512 KiB` | `< 512 KiB` |

Pojedynczy skok po wejściu do misji nie jest podstawą do odrzucenia buildu. Liczy się ustabilizowane okno po utworzeniu przeciwników.

## Test stabilności zasobów

Po pomiarze wydajności wykonaj dodatkowo:

1. Wejdź do dowolnej misji i wróć do kampanii.
2. Powtórz co najmniej 10 razy, mieszając zwykłe misje, replay i test wydajności.
3. Potwierdź brak crasha oraz brak trwałego spadku `MEM` pomiędzy kolejnymi sesjami.

Spadek pamięci po każdym wejściu oznacza potencjalny wyciek i blokuje uznanie testu za zaliczony.

## Formularz wyniku

```text
Commit/tag:
Model konsoli:
Wersja systemu/CFW:
Format: CIA / 3DSX

MONO
FPS:
AVG:
MAX:
RENDER:
MEM:

STEREO 100%
FPS:
AVG:
MAX:
RENDER:
MEM:
Komfort obrazu:

10 kolejnych wejść bez crasha: TAK / NIE
Uwagi:
```

## Kryterium zaliczenia

Build spełnia podstawowy budżet Old 3DS XL, gdy:

- średnia klatka w mono i stereo nie przekracza 33,33 ms;
- najgorsza wartość po ustabilizowaniu pozostaje na poziomie najwyżej 36 ms;
- pamięć liniowa pozostaje powyżej 512 KiB;
- nie występuje narastający ubytek pamięci ani crash w teście 10 sesji;
- stereoskopia pozostaje komfortowa i nie powoduje widocznego podwójnego obrazu w punkcie zbieżności.
