## Zakres

- Co zmienia ten PR?
- Jakie issue zamyka lub rozwija?

## Walidacja

- [ ] testy hostowe
- [ ] build `.3dsx`
- [ ] build `.cia`
- [ ] instalacja CIA w Azaharze
- [ ] bezpośredni smoke test 3DSX

## Audyt sprzętowy Nintendo 3DS

Pełny kontrakt: [`docs/HARDWARE_AUDIT.md`](../docs/HARDWARE_AUDIT.md). Powiązany tracker: #192.

### Wpływ

- Profil Old 3DS: `BRAK / NISKI / ŚREDNI / WYSOKI`
- Profil New 3DS: `BRAK / NISKI / ŚREDNI / WYSOKI`
- Zmienione obszary: `CPU / PICA200 / pamięć liniowa / geometria / tekstury / górny ekran / dolny ekran / dotyk / stereo / release / N/D`
- Uzasadnienie `N/D`, jeśli pełny audyt nie jest potrzebny:

### Obowiązkowa checklista dla zmian wizualnych lub sprzętowych

- [ ] podstawowa rozgrywka nie zależy od funkcji New 3DS
- [ ] zachowane są budżety wierzchołków, obiektów i pamięci liniowej
- [ ] brak nowych alokacji liniowych lub transferów tekstur wykonywanych stale co klatkę
- [ ] oba ekrany zostały sprawdzone
- [ ] każdy widoczny przycisk dolnego ekranu ma zgodny obszar dotykowy
- [ ] mono i stereo używają fizycznego suwaka 3D zgodnie z kontraktem
- [ ] dołączono lub wskazano pakiet PNG wymagany przez audyt
- [ ] zapisano wynik `PASS/WARN/FAIL` dla Old 3DS albo jawnie wskazano, że fizyczny retest pozostaje wymagany
- [ ] wynik New 3DS, jeśli dostępny, jest raportowany osobno

## Dowody

- Commit artefaktów:
- SHA-256 CIA:
- SHA-256 3DSX:
- Link do artefaktów/logów:
- Link do PNG:
- Fizyczny model konsoli i wynik:

## Self-review

- [ ] przejrzałem cały diff
- [ ] nie ma zmian spoza zakresu
- [ ] dokumentacja i testy odpowiadają aktualnemu zachowaniu
- [ ] znane ograniczenia są zapisane w PR lub issue
