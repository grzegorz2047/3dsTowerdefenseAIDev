# Pakiet dowodowy audytu sprzętowego

Ten katalog przechowuje wersjonowany dowód wymagany przez `docs/HARDWARE_AUDIT.md` i issue #192.

## Przygotowanie kandydata release

1. Skopiuj `manifest.template.json` do `hardware-audit/releases/<wersja>/manifest.json`.
2. Zbuduj `.cia` i `.3dsx` z dokładnie tego samego commita, który zawiera manifest.
3. Wpisz SHA-256 obu artefaktów zgodne z `dist/SHA256SUMS.txt`.
4. Dodaj wymagane PNG do katalogu wersji i opisz wynik ich przeglądu.
5. Wykonaj fizyczny test Old 3DS według `docs/PERFORMANCE_TESTING.md`; New 3DS jest raportowany osobno, gdy jest dostępny.
6. Ustaw końcową decyzję `PASS`, `WARN` albo `FAIL`. `WARN` wymaga listy odstępstw. `FAIL` Old 3DS zawsze blokuje publikację.
7. Uruchom lokalnie:

```bash
python3 scripts/validate_hardware_evidence.py \
  --manifest hardware-audit/releases/<wersja>/manifest.json \
  --expected-version <wersja> \
  --expected-commit "$(git rev-parse HEAD)" \
  --artifact-dir dist \
  --repository-root . \
  --release
```

## Egzekwowanie w CI

Push gałęzi `publish-v*`:

- buduje `.cia` i `.3dsx` oraz sprawdza `SHA256SUMS.txt`;
- instaluje CIA i uruchamia 3DSX w Azaharze;
- wymaga manifestu `hardware-audit/releases/<wersja>/manifest.json`;
- porównuje manifest z `GITHUB_SHA`, faktycznymi artefaktami i wymaganymi PNG;
- sprawdza progi Old 3DS: średnia `<= 33.333 ms`, najgorsza klatka `<= 36 ms`, render mono `<= 18 ms`, render stereo `<= 28 ms`, pamięć liniowa `>= 512 KiB`;
- pakuje manifest i PNG do ZIP dołączanego do GitHub pre-release;
- nie uruchamia publikacji, gdy wynik Old 3DS to `FAIL` albo brakuje dowodu fizycznego.

Zielony emulator nie zastępuje fizycznego testu. Brak manifestu jest błędem, a nie wynikiem `N/D`.
