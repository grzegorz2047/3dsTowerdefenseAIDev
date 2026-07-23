# Fabuła kampanii

Kampania składa się z prologu tutorialowego oraz pięciu rozdziałów regionu Asterii. Teksty są przechowywane poza kodem w `romfs/narrative/pl/<mission-id>.txt`.

Każdy plik zawiera:

- dwie krótkie karty odprawy;
- kartę wprowadzającą mechanikę misji;
- zakończenie zwycięstwa;
- zakończenie porażki.

Identyfikatory misji i kart używają wyłącznie małych liter ASCII, cyfr, kropki i podkreślenia. Test kampanii sprawdza kompletność plików, zgodność `mission.id` z katalogiem oraz globalną unikalność identyfikatorów kart.

## Sterowanie odprawą

- `A` lub `PRAWO`: następna karta;
- `B` lub `LEWO`: poprzednia karta, a na pierwszej karcie powrót;
- `X`: pominięcie pozostałych kart;
- `Y` na ekranie kampanii: ponowne otwarcie odprawy wybranej misji.

Odprawa przed misją jest pokazywana tylko przed pierwszym uruchomieniem w danej sesji wyboru. Szybki restart po porażce lub zwycięstwie nie pokazuje jej ponownie. Karta wyniku pojawia się dokładnie raz po zakończeniu każdego podejścia.

Brak lub uszkodzenie pliku narracji nie zatrzymuje rozgrywki. Kampania wyświetla błąd odprawy, ale pozwala uruchomić poziom bez tekstów fabularnych.
