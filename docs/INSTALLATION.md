# Instalacja Citadel Defense 3D

## Dostępne pliki

- `CitadelDefense3D.cia` — główny format instalacyjny dla konsoli Nintendo 3DS z odpowiednio skonfigurowanym custom firmware.
- `CitadelDefense3D.3dsx` — format uruchamiany przez Homebrew Launcher oraz bezpośrednio przez emulatory obsługujące homebrew.
- `SHA256SUMS.txt` — sumy kontrolne obu plików.

## Weryfikacja pobranych plików

W katalogu z artefaktami uruchom:

```bash
sha256sum -c SHA256SUMS.txt
```

Na Windows można porównać wynik polecenia `Get-FileHash -Algorithm SHA256 <plik>` z wartością zapisaną w `SHA256SUMS.txt`.

## Azahar

### Najprostszy test: `.3dsx`

1. Uruchom aktualną wersję Azahar.
2. Wybierz `File -> Load File` lub przeciągnij `CitadelDefense3D.3dsx` do okna emulatora.
3. Nie instaluj `.3dsx` jako CIA — ten format jest uruchamiany bezpośrednio.

### Instalacja `.cia`

Plik `.cia` nie jest uruchamiany bezpośrednio jak ROM:

1. wybierz `File -> Install CIA`;
2. wskaż `CitadelDefense3D.cia`;
3. po zakończeniu instalacji uruchom `Citadel Defense 3D` z listy zainstalowanych tytułów.

Azahar obsługuje też instalację z wiersza poleceń:

```bash
azahar --install CitadelDefense3D.cia
```

### Gdy pojawia się czarny ekran lub aplikacja się zamyka

- zaktualizuj Azahar do najnowszej wersji stabilnej albo aktualnego wydania testowego;
- zacznij od bezpośredniego uruchomienia `.3dsx`, aby oddzielić problem instalacji CIA od problemu aplikacji;
- sprawdź sumy SHA-256;
- w poprawionej wersji gry błąd startu pozostaje widoczny na ekranie do naciśnięcia START;
- do zgłoszenia dołącz treść ekranu diagnostycznego oraz plik `azahar_log.txt`.

Typowe lokalizacje danych Azahar:

- Windows: katalog danych aplikacji `Azahar`, podkatalog `log`;
- Linux: katalog danych `azahar-emu/log`;
- nazwa pliku logu: `azahar_log.txt`.

## Instalacja `.cia` na konsoli

1. Upewnij się, że konsola ma legalnie skonfigurowane środowisko homebrew/custom firmware i narzędzie potrafiące instalować paczki `.cia`.
2. Skopiuj `CitadelDefense3D.cia` na kartę SD.
3. Zainstaluj paczkę przy użyciu narzędzia dostępnego w Twoim środowisku.
4. Uruchom `Citadel Defense 3D` z menu HOME.

Projekt nie zawiera prywatnych kluczy, materiałów z komercyjnego SDK ani komponentów służących do omijania zabezpieczeń konsoli.

## Uruchomienie `.3dsx` na konsoli

1. Utwórz katalog `/3ds/CitadelDefense3D/` na karcie SD.
2. Skopiuj do niego `CitadelDefense3D.3dsx`.
3. Uruchom Homebrew Launcher.
4. Wybierz `Citadel Defense 3D`.

RomFS jest osadzony w pliku `.3dsx`, dlatego poziom nie wymaga osobnego kopiowania.

## Sterowanie

- Circle Pad — przesuwanie kamery;
- L/R — obrót kamery;
- D-pad — wybór pola budowy;
- A — budowa wieży;
- START — wyjście.

## Aktualizacje

Wersje `.cia` korzystają ze stałego Unique ID `0xC1D3D`. Aktualizacja powinna być instalowana nad poprzednią wersją, o ile notatki wydania nie wskazują inaczej.

Pierwsze wydania są wersjami alfa. Test instalacji na fizycznym urządzeniu jest osobnym krokiem jakościowym i jego status jest publikowany w issue #11.
