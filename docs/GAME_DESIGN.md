# Citadel Defense 3D — plan projektu

## 1. Cel projektu

`Citadel Defense 3D` to pełna trójwymiarowa gra tower defense dla Nintendo 3DS, uruchamiana jako homebrew. Gra wykorzystuje górny ekran do renderowania mapy 3D i opcjonalnej stereoskopii, a dolny ekran dotykowy do budowania, ulepszania, sprzedaży i zarządzania przebiegiem bitwy.

Najważniejszym trybem będzie kampania z poziomami do przejścia. Każdy poziom ma osobną mapę, układ tras, zestaw fal, ograniczenia, cele dodatkowe i ocenę końcową. Kolejne poziomy odblokowują nowe wieże, mechaniki i typy przeciwników.

## 2. Główna pętla rozgrywki

1. Gracz wybiera odblokowany poziom na mapie kampanii.
2. Przed rozpoczęciem widzi cele, nagrody, dostępne wieże i przewidywane typy przeciwników.
3. Rozpoczyna poziom z ustaloną liczbą monet i punktów życia bazy.
4. Buduje wieże na wyznaczonych polach.
5. Przeciwnicy pojawiają się falami i podążają trasami do bazy.
6. Wieże automatycznie wykrywają i atakują cele.
7. Pokonani przeciwnicy zapewniają monety.
8. Gracz ulepsza lub sprzedaje wieże i reaguje na kolejne fale.
9. Po zakończeniu ostatniej fali gra oblicza wynik i ocenę poziomu.
10. Ukończenie poziomu odblokowuje kolejny etap kampanii oraz ewentualne nowe elementy wyposażenia.

Jedna standardowa misja powinna trwać około 10–20 minut. Poziomy specjalne i finałowe mogą trwać do około 25 minut.

## 3. Kampania i poziomy do przejścia

### 3.1. Struktura kampanii

Kampania jest podzielona na regiony. Każdy region ma własny wygląd, typowe zagrożenia i jedną mechaniczną cechę przewodnią.

Plan MVP:

- Prolog: 1 samouczek;
- Region 1: 5 właściwych poziomów;
- łącznie: 6 grywalnych poziomów.

Plan wersji pełnej:

- 4 regiony po 5 poziomów;
- 4 poziomy finałowe regionów;
- 1 finał kampanii;
- łącznie około 25 poziomów.

### 3.2. Odblokowywanie

Domyślna zasada:

- pierwszy poziom jest dostępny od początku;
- ukończenie poziomu z co najmniej 1 gwiazdką odblokowuje następny;
- wybrane poziomy opcjonalne wymagają określonej liczby gwiazdek;
- nowe wieże są odblokowywane po konkretnych misjach;
- trudniejsze warianty poziomu są odblokowywane po ukończeniu regionu.

Gra nie powinna wymuszać idealnego wyniku, aby przejść dalej. Gracz musi mieć możliwość ukończenia kampanii przy przeciętnym wyniku, natomiast dodatkowe gwiazdki nagradzają lepszą grę i odblokowują zawartość opcjonalną.

### 3.3. Ocena poziomu

Każdy poziom przyznaje od 1 do 3 gwiazdek:

- 1 gwiazdka — ukończenie poziomu;
- 2 gwiazdki — ukończenie z co najmniej 50% początkowego życia bazy;
- 3 gwiazdki — ukończenie bez utraty życia albo spełnienie celu mistrzowskiego poziomu.

Poziom może mieć również cele dodatkowe, na przykład:

- nie używaj wieży ciężkiej;
- nie sprzedawaj wież;
- ukończ poziom z co najmniej 300 monetami;
- pokonaj bossa przed upływem określonego czasu;
- zbuduj maksymalnie sześć wież;
- nie dopuść żadnego szybkiego przeciwnika do ostatniego odcinka trasy.

Cele dodatkowe nie blokują postępu głównej kampanii. Mogą zapewniać odznaki, dodatkowe gwiazdki albo elementy kosmetyczne.

### 3.4. Typy poziomów

Poziomy powinny mieć różne zasady, aby kampania nie była zbiorem podobnych map.

#### Standardowy

- jedna lub dwie trasy;
- pełen wybór odblokowanych wież;
- klasyczne fale;
- cel: przetrwać wszystkie fale.

#### Oblężenie

- duże grupy opancerzonych przeciwników;
- ograniczona liczba miejsc budowy;
- nacisk na ciężkie i obszarowe obrażenia.

#### Szybki atak

- krótsze odstępy między falami;
- szybcy przeciwnicy;
- mniejsza mapa;
- misja trwa krócej, ale wymaga szybkich decyzji.

#### Obrona wielu bram

- dwie lub trzy trasy prowadzą do wspólnej bazy;
- wieże muszą pokrywać różne kierunki;
- priorytety celowania mają większe znaczenie.

#### Ograniczony arsenał

- dostępne są tylko wybrane rodzaje wież;
- poziom uczy konkretnej mechaniki lub kombinacji.

#### Boss

- mniej zwykłych fal;
- unikalny przeciwnik z fazami lub zdolnościami;
- boss może przywoływać jednostki, czasowo wyłączać wieże lub zmieniać trasę.

#### Przetrwanie

- określony czas obrony zamiast ustalonej liczby fal;
- opcjonalny tryb nieskończony po ukończeniu kampanii.

### 3.5. Projekt poziomów MVP

#### Poziom 0 — Strażnica

Cel: samouczek.

- prosta mapa z jedną trasą;
- trzy wyznaczone miejsca budowy;
- dostępna tylko wieża podstawowa;
- 5 krótkich fal;
- gra uczy wyboru pola, budowy, ulepszenia, sprzedaży i uruchomienia fali;
- brak oceny karnej za błędy podczas pierwszego podejścia.

Nagroda: odblokowanie wieży podstawowej i mapy kampanii.

#### Poziom 1 — Zielony trakt

Cel: pierwsza pełna misja.

- jedna długa trasa;
- 8 fal;
- zwykli i grupowi przeciwnicy;
- 8 miejsc budowy;
- podstawowa ekonomia bez dodatkowych ograniczeń.

Nagroda: odblokowanie wieży ciężkiej.

#### Poziom 2 — Rozwidlenie

Cel: nauka pokrywania dwóch kierunków.

- dwie trasy łączące się przed bazą;
- 10 fal;
- zwykli i szybcy przeciwnicy;
- wieże przy wspólnym odcinku mogą atakować obie trasy.

Cel dodatkowy: zbuduj maksymalnie sześć wież.

Nagroda: odblokowanie wyboru priorytetu celu.

#### Poziom 3 — Kamienny przesmyk

Cel: wprowadzenie pancerza.

- wąski teren i ograniczone miejsca budowy;
- 10 fal;
- pierwsi opancerzeni przeciwnicy;
- duże znaczenie wieży ciężkiej;
- część stanowisk ma lepszą wysokość i większą widoczność.

Nagroda: odblokowanie wieży obszarowej.

#### Poziom 4 — Zalana dolina

Cel: zarządzanie tłumem.

- dwie trasy o różnej długości;
- liczne słabe jednostki;
- 12 fal;
- wybrane miejsca budowy chwilowo blokowane przez przypływ lub efekt środowiskowy;
- pierwsze użycie prostego zdarzenia poziomu.

Cel dodatkowy: nie sprzedawaj żadnej wieży.

Nagroda: odblokowanie wieży spowalniającej.

#### Poziom 5 — Brama Cytadeli

Cel: finał pierwszego regionu.

- szeroka mapa z dwiema trasami;
- 12 fal;
- wszystkie poznane klasy przeciwników;
- ostatnia fala zawiera bossa;
- boss ma wysoką odporność na spowolnienie i okresowo wzmacnia pobliskie jednostki.

Nagroda: odblokowanie kolejnego regionu, trudnego wariantu pierwszych map i trybu powtarzania poziomów dla lepszego wyniku.

### 3.6. Ekran wyboru poziomu

Na dolnym ekranie wyświetlana jest mapa kampanii z węzłami poziomów. Górny ekran pokazuje trójwymiarową miniaturę wybranego obszaru albo statyczny podgląd mapy.

Karta poziomu powinna zawierać:

- nazwę;
- numer i region;
- liczbę zdobytych gwiazdek;
- najlepszy wynik;
- poziom trudności;
- przewidywane typy przeciwników;
- dostępne wieże;
- nagrodę za pierwsze ukończenie;
- cele dodatkowe;
- przycisk rozpoczęcia.

### 3.7. Powtarzanie poziomów

Każdy ukończony poziom można rozegrać ponownie, aby:

- zdobyć brakujące gwiazdki;
- poprawić wynik;
- ukończyć cele dodatkowe;
- przetestować inne ustawienie wież;
- zagrać na wyższym poziomie trudności.

Poziomy nie zużywają energii ani biletów. Gra pozostaje całkowicie offline i nie stosuje mechanik czasowych.

### 3.8. Poziomy trudności

MVP może mieć jeden podstawowy poziom trudności. Po ukończeniu regionu odblokowywany jest wariant trudny:

- przeciwnicy mają więcej zdrowia;
- fale pojawiają się szybciej;
- gracz rozpoczyna z mniejszą liczbą monet;
- część przeciwników otrzymuje dodatkową odporność;
- poziom przyznaje osobną ocenę i najlepszy wynik.

Balans poziomu trudnego powinien być przechowywany jako modyfikatory danych, a nie jako osobna kopia każdej mapy.

## 4. Sterowanie

### Górny ekran

Wyświetla:

- mapę 3D;
- trasy;
- wieże;
- przeciwników;
- pociski i efekty;
- zasięg zaznaczonej wieży;
- numer i postęp fali;
- monety;
- życie bazy.

### Dolny ekran

Wyświetla:

- panel dostępnych wież;
- parametry zaznaczonej wieży;
- ulepszenie;
- sprzedaż;
- zmianę priorytetu celu;
- uruchomienie następnej fali;
- szybkość gry;
- pauzę;
- cele bieżącego poziomu.

### Przyciski

- Circle Pad — przesuwanie kamery;
- D-pad — przesuwanie kursora po polach budowy;
- A — wybór lub potwierdzenie;
- B — anulowanie;
- X — rozpoczęcie następnej fali;
- Y — zmiana prędkości 1×/2×;
- L/R — obrót kamery między czterema ustalonymi kątami;
- Start — pauza;
- ekran dotykowy — obsługa interfejsu i wybór elementów.

## 5. Kamera

- perspektywiczna kamera nad planszą;
- nachylenie około 45°;
- cztery kierunki obrotu;
- ograniczony zoom;
- automatyczne utrzymywanie kamery w granicach mapy;
- brak możliwości wejścia pod geometrię;
- w samouczku możliwość chwilowego przejęcia kamery przez grę w celu pokazania ważnego miejsca.

## 6. Mapa

Mapa jest oparta na siatce, na przykład 16×16 lub 20×20 pól. Pole ma typ logiczny niezależny od wyglądu modelu:

- Ground;
- Road;
- BuildSpot;
- Blocked;
- Spawn;
- Base;
- Decoration;
- TemporaryBlocked;
- TriggerZone.

Na początku trasy są z góry zdefiniowane. Wieże nie mogą całkowicie blokować drogi. Pozwala to utrzymać prosty, przewidywalny i wydajny system ruchu.

Każdy poziom przechowuje:

- rozmiar mapy;
- model lub zestaw kafelków terenu;
- punkty tras;
- punkty pojawiania się jednostek;
- bazę lub bazy;
- pola budowy;
- dekoracje;
- oświetlenie i tło;
- zdarzenia poziomu;
- dozwolone wieże;
- startową ekonomię;
- fale;
- cele i progi ocen.

## 7. Wieże

### Wieża podstawowa

- niski koszt;
- szybki atak;
- średni zasięg;
- niewielkie obrażenia;
- dobra przeciwko zwykłym i szybkim jednostkom.

### Wieża ciężka

- wolny atak;
- duże obrażenia pojedynczego trafienia;
- częściowe przebijanie pancerza;
- wysoki koszt.

### Wieża obszarowa

- obrażenia dla grupy przeciwników;
- krótki lub średni zasięg;
- wolniejszy atak;
- dobra przeciwko licznym słabym jednostkom.

### Wieża spowalniająca

- małe obrażenia;
- nakłada czasowe spowolnienie;
- efekt nie sumuje się bez ograniczeń;
- bossowie mają częściową odporność.

Każda wieża posiada:

- koszt;
- poziom;
- zasięg;
- obrażenia;
- szybkość ataku;
- typ obrażeń;
- priorytet celu;
- koszt ulepszenia;
- wartość sprzedaży;
- ograniczenia poziomu;
- identyfikator modelu, dźwięku i efektu.

W MVP wieże mają trzy poziomy rozwoju bez rozgałęzionego drzewa.

## 8. Przeciwnicy

Podstawowe klasy:

- zwykły;
- szybki;
- grupowy;
- opancerzony;
- boss.

Dane przeciwnika:

- pozycja;
- bieżące i maksymalne zdrowie;
- prędkość;
- pancerz;
- odporności;
- nagroda;
- postęp na trasie;
- aktywne efekty statusu;
- identyfikator trasy;
- identyfikator modelu i animacji.

## 9. Celowanie

Priorytety wież:

- pierwszy na trasie;
- ostatni na trasie;
- najsilniejszy;
- najsłabszy;
- najbliższy.

Domyślnie wieża atakuje jednostkę, która przeszła najdłuższą część trasy. Pełne wyszukiwanie celów nie powinno odbywać się w każdej klatce. Wieże mogą aktualizować cel kilka razy na sekundę, a pomiędzy aktualizacjami śledzić już wybraną jednostkę.

## 10. Pociski i obrażenia

- pociski fizyczne lecące do celu;
- ataki natychmiastowe dla wybranych efektów;
- obrażenia obszarowe;
- status spowolnienia;
- pula obiektów dla pocisków i efektów;
- brak dynamicznej alokacji pamięci w intensywnej części rozgrywki.

## 11. Fale

Fala definiuje:

- grupy przeciwników;
- liczbę jednostek;
- odstęp między pojawieniami;
- wejście i trasę;
- opóźnienie grupy;
- modyfikator zdrowia, prędkości i nagrody;
- warunek uruchomienia;
- opcjonalne zdarzenie fabularne lub środowiskowe.

Fale są zapisane w danych poziomu i nie są zakodowane na stałe w C++.

## 12. Styl graficzny

- low-poly;
- proste i czytelne sylwetki;
- ograniczona liczba materiałów;
- tekstury najczęściej 128×128 lub 256×256;
- wyraźne oznaczenie klas wież i przeciwników;
- modele projektowane tak, aby dobrze wyglądały także na małej rozdzielczości;
- proste animacje obrotu, odrzutu, ruchu i śmierci;
- efekty cząsteczkowe zastępowane tam, gdzie to możliwe, billboardami lub animowanymi sprite'ami.

## 13. Stereoskopowe 3D

Scena na górnym ekranie jest renderowana dla lewego i prawego oka. Rozstaw kamer zależy od ustawienia suwaka 3D.

Zasady komfortu:

- główna płaszczyzna mapy pozostaje blisko powierzchni ekranu;
- interfejs nie powinien mieć agresywnej głębi;
- efekty nie powinny znacząco wychodzić przed ekran;
- tryb bez stereoskopii musi działać bez różnic w rozgrywce;
- wydajność i liczba obiektów muszą uwzględniać dwukrotne renderowanie górnej sceny.

## 14. Dźwięk

Minimalny zestaw:

- kliknięcia interfejsu;
- budowa, ulepszenie i sprzedaż;
- odrębny dźwięk każdego typu wieży;
- trafienie i eksplozja;
- śmierć przeciwnika;
- utrata życia bazy;
- rozpoczęcie i ukończenie fali;
- zwycięstwo i porażka;
- muzyka mapy kampanii;
- muzyka bitwy;
- osobny motyw bossa, jeżeli pozwoli na to budżet pamięci.

## 15. Zapis postępu

Zapisywane są:

- najwyższy odblokowany poziom;
- gwiazdki dla każdego poziomu i trudności;
- najlepszy wynik;
- najlepsza liczba zachowanego życia;
- ukończone cele dodatkowe;
- odblokowane wieże i mechaniki;
- ukończony samouczek;
- ustawienia dźwięku, sterowania i stereoskopii;
- wersja formatu zapisu i suma kontrolna.

Zapis powinien być odporny na przerwanie procesu. Preferowany jest zapis do pliku tymczasowego i atomowa podmiana po poprawnym zakończeniu serializacji.

## 16. Format danych poziomu

Docelowo poziom powinien być opisany danymi, a nie osobną klasą C++.

Przykładowy logiczny model:

```json
{
  "id": "region1_level2",
  "name": "Rozwidlenie",
  "map": "maps/region1_level2.map",
  "startingCoins": 300,
  "baseHealth": 20,
  "allowedTowers": ["basic", "heavy"],
  "paths": ["left", "right"],
  "waves": "waves/region1_level2.json",
  "unlock": {
    "requiredLevel": "region1_level1",
    "requiredStars": 1
  },
  "rewards": {
    "firstCompletionUnlock": "target_priorities"
  },
  "stars": {
    "one": "complete",
    "two": { "minimumHealth": 10 },
    "three": { "minimumHealth": 20 }
  },
  "bonusObjectives": [
    { "type": "maximumBuiltTowers", "value": 6 }
  ]
}
```

Na 3DS można zastosować prostszy format binarny generowany podczas budowania albo niewielki parser JSON używany tylko przy ładowaniu poziomu. Decyzję należy podjąć po pomiarze czasu ładowania i rozmiaru kodu.

## 17. Architektura projektu

```text
source/
  main.cpp
  Game.cpp
  GameState.cpp
  Renderer.cpp
  Camera.cpp
  Input.cpp
  Campaign.cpp
  Level.cpp
  LevelLoader.cpp
  LevelResult.cpp
  Map.cpp
  Path.cpp
  Tower.cpp
  TowerManager.cpp
  Enemy.cpp
  EnemyManager.cpp
  Projectile.cpp
  ProjectileManager.cpp
  WaveManager.cpp
  ObjectiveManager.cpp
  Collision.cpp
  SaveSystem.cpp
  AudioSystem.cpp
  UiSystem.cpp

include/
  Game.hpp
  GameState.hpp
  Renderer.hpp
  Camera.hpp
  Campaign.hpp
  Level.hpp
  LevelLoader.hpp
  Map.hpp
  Tower.hpp
  Enemy.hpp
  Projectile.hpp
  WaveManager.hpp
  ObjectiveManager.hpp
  SaveSystem.hpp

assets/
  models/
  textures/
  audio/
  shaders/
  maps/
  levels/
  waves/
  campaign/
```

Najważniejsze stany gry:

- Boot;
- MainMenu;
- CampaignMap;
- LevelBriefing;
- Loading;
- Playing;
- Paused;
- Victory;
- Defeat;
- Settings.

## 18. Budżet wydajności

Docelowo na starszym Nintendo 3DS:

- stabilne 30 FPS;
- około 30–40 aktywnych przeciwników;
- około 15–20 wież;
- ograniczona, kontrolowana liczba pocisków;
- jedna główna scena 3D i prosty interfejs 2D;
- ograniczenie przezroczystości i efektów nakładających się;
- statyczna geometria mapy łączona w większe bufory;
- frustum culling dla dekoracji i obiektów poza widokiem;
- aktualizacja AI z niższą częstotliwością niż renderowanie;
- pule obiektów;
- testy obowiązkowo na rzeczywistym urządzeniu, nie wyłącznie w emulatorze.

## 19. Zakres MVP

MVP obejmuje:

- uruchomienie jako `.3dsx`;
- menu główne;
- mapę kampanii;
- samouczek i pięć poziomów do przejścia;
- trwałe odblokowywanie poziomów;
- system 1–3 gwiazdek;
- cele dodatkowe;
- jedną rodzinę wizualną map;
- trzy podstawowe wieże, z czwartą odblokowywaną pod koniec regionu;
- trzy podstawowe klasy przeciwników oraz opancerzonego bossa;
- budowanie, ulepszanie i sprzedaż;
- fale, ekonomię, życie bazy, zwycięstwo i porażkę;
- sterowanie przyciskami i dotykiem;
- kamerę 3D;
- stereoskopowe 3D;
- podstawowy dźwięk;
- zapis postępu i najlepszych wyników.

Poza MVP:

- pełna kampania 20–25 poziomów;
- dodatkowe regiony;
- rozgałęzione ulepszenia;
- bohater;
- tryb nieskończony;
- wyzwania dzienne bez zależności od sieci;
- edytor map;
- osiągnięcia;
- elementy kosmetyczne;
- wiele profili zapisu.

## 20. Etapy realizacji

### Etap 1 — fundament techniczny

- toolchain devkitARM;
- bazowy projekt 3DS;
- renderowanie prostej sceny Citro3D;
- oba ekrany;
- wejście;
- kamera;
- stereoskopia;
- prosty pomiar FPS i pamięci.

### Etap 2 — model danych i poziom testowy

- `LevelDefinition`;
- `LevelLoader`;
- testowa mapa 3D;
- trasa;
- pola budowy;
- warunek zwycięstwa i porażki;
- hot-reload nie jest wymagany na urządzeniu, ale walidator danych powinien działać na komputerze.

### Etap 3 — przeciwnicy

- ruch po trasie;
- kilka wejść i tras;
- zdrowie;
- dotarcie do bazy;
- usuwanie i pule obiektów.

### Etap 4 — wieże i walka

- budowanie;
- celowanie;
- obrót głowicy;
- pociski;
- obrażenia;
- zasięg;
- ulepszenia;
- sprzedaż.

### Etap 5 — fale, ekonomia i cele

- `WaveManager`;
- nagrody;
- cele dodatkowe;
- gwiazdki;
- ekran wyniku poziomu.

### Etap 6 — kampania

- mapa poziomów;
- odblokowywanie;
- briefing;
- zapis postępu;
- powtarzanie poziomów;
- najlepsze wyniki.

### Etap 7 — zawartość MVP

- samouczek;
- pięć poziomów regionu;
- boss;
- balans fal;
- modele i tekstury;
- dźwięki i muzyka.

### Etap 8 — optymalizacja i stabilizacja

- profilowanie na Old 3DS;
- zmniejszanie draw calli;
- budżety pamięci;
- testy zapisu;
- testy przejścia całej kampanii;
- testy z włączonym i wyłączonym stereoskopowym 3D;
- pakiet `.3dsx` i RomFS.

## 21. Kryteria ukończenia MVP

MVP jest gotowe, gdy:

- uruchamia się przez Homebrew Launcher;
- użytkownik może rozpocząć nową kampanię;
- działa mapa wyboru poziomu;
- sześć poziomów można przejść od początku do końca;
- ukończenie poziomu zapisuje wynik, gwiazdki i odblokowania;
- można powtórzyć poziom i poprawić wynik;
- mapa, wieże i przeciwnicy są renderowane w pełnym 3D;
- stereoskopia działa i może być wyłączona;
- budowanie, ulepszanie, sprzedaż i priorytety celowania działają;
- wszystkie fale można wygrać zgodnie z balansem;
- boss pierwszego regionu działa;
- zapis jest odporny na typowe przerwanie działania;
- gra utrzymuje docelową płynność na rzeczywistym starszym 3DS;
- nie ma błędów blokujących przejście kampanii.

## 22. Następny krok

Następnym zadaniem powinno być utworzenie technicznego szkieletu projektu z devkitARM, Citro3D i Citro2D oraz wdrożenie pierwszego pionowego wycinka:

- uruchomienie gry;
- górny ekran z prostą mapą 3D;
- dolny ekran z panelem testowym;
- kamera obracana L/R;
- jedno pole budowy;
- jeden przeciwnik poruszający się po trasie;
- jedna wieża strzelająca do celu;
- dane poziomu testowego ładowane z RomFS.
