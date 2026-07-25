# Pakiet dowodowy audytu sprzętowego

Ten katalog przechowuje wersjonowany dowód wymagany przez `docs/HARDWARE_AUDIT.md` i issue #192.

## Przygotowanie kandydata release

1. Zbuduj kandydata `.cia` i `.3dsx` z niezmiennego commita i zachowaj SHA tego commita.
2. Pobierz dokładnie te artefakty, zapisz ich SHA-256 i wykonaj na nich fizyczny test Old 3DS; New 3DS raportuj osobno, gdy jest dostępny.
3. Na osobnej gałęzi `publish-v*` skopiuj `manifest.template.json` do `hardware-audit/releases/<wersja>/manifest.json`.
4. W polu `commit` wpisz SHA wcześniej przetestowanego commita kandydata, a nie SHA commita dodającego manifest.
5. Wpisz SHA-256 fizycznie przetestowanych `.cia` i `.3dsx`, dodaj wymagane PNG oraz wyniki ich przeglądu.
6. Ustaw końcową decyzję `PASS`, `WARN` albo `FAIL`. `WARN` wymaga listy odstępstw. `FAIL` Old 3DS zawsze blokuje publikację.
7. Lokalnie zbuduj wskazany commit i uruchom walidator:

```bash
candidate=$(python3 -c 'import json; print(json.load(open("hardware-audit/releases/<wersja>/manifest.json"))["commit"])')
git worktree add /tmp/citadel-release "$candidate"
make -C /tmp/citadel-release release
python3 scripts/validate_hardware_evidence.py \
  --manifest hardware-audit/releases/<wersja>/manifest.json \
  --expected-version <wersja> \
  --expected-commit "$candidate" \
  --artifact-dir /tmp/citadel-release/dist \
  --repository-root . \
  --release
```

## Egzekwowanie w CI

Push gałęzi `publish-v*`:

- odczytuje commit kandydata z manifestu i buduje właśnie ten commit, nie commit gałęzi publikacyjnej;
- sprawdza, czy odbudowane `.cia` i `.3dsx` mają SHA-256 zgodne z fizycznie przetestowanymi plikami;
- instaluje CIA i uruchamia 3DSX w Azaharze;
- wymaga manifestu `hardware-audit/releases/<wersja>/manifest.json` i kompletu PNG;
- sprawdza progi Old 3DS: średnia `<= 33.333 ms`, najgorsza klatka `<= 36 ms`, render mono `<= 18 ms`, render stereo `<= 28 ms`, pamięć liniowa `>= 512 KiB`;
- pakuje manifest i PNG do ZIP dołączanego do GitHub pre-release;
- publikuje release wskazujący na commit kandydata;
- nie uruchamia publikacji, gdy wynik Old 3DS to `FAIL`, brakuje dowodu albo build nie jest reprodukowalny bajt w bajt.

Zielony emulator nie zastępuje fizycznego testu. Brak manifestu jest błędem, a nie wynikiem `N/D`.
