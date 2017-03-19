# Projekt Bmb (rok 2015)
## Wyścig szczurów

Serwer gry prowadzi rekrutacje graczy, każdy gracz musi mieć unikalny nick. Serwer zestawia graczy w pary tak długo aż każdy rozegra wyścig z każdym z puli aktualnie zalogowanych na serwer. Jeśli po rozegraniu wszystkich gier dołączy nowy gracz, będzie on grał ze wszystkimi wcześniej zalogowanymi. Za każdy wygrany wyścig przyznawany jest punkt, za przegraną zero punktów. Gdy rozegrane zostaną wszystkie możliwe w danej chwili gry, serwer roześle klientom aktualny ranking wg. zdobytych punktów. Każdy z graczy może w dowolnym momencie odłączyć się od serwera, przez co znika z rankingu, jeśli nie dokończył wyścigu to jego przeciwnik dostaje punkt. Zakończenie gier nie oznacza automatycznego rozłączenia, klienci/gracze mogą czekać na nowego gracza.

Gra (wyścig) polega na sprawdzeniu który z graczy szybciej pisze na klawiaturze. Serwer przesyła każdemu z graczy losowe słowa z bazy słów (plik w specjalnym katalogu), użytkownik musi jak najszybciej i bezbłędnie przepisać to słowo. Kolejne słowa są przesyłane dopiero po poprawnym przepisaniu poprzedniego. Wygrywa ten kto szybciej przepisze cały tekst. Obaj uczestnicy wyścigu przepisują ten sam tekst. Gdy jeden z graczy zakończy przepisywanie, ten drugi musi zostać o tym niezwłocznie powiadomiony, nie kończy on już przepisywania. W pojedynczym wyścigu słowa z bazy nie mogą się powtarzać.

Każde połączenie do gracza musi być obsługiwane przez oddzielny wątek, każda gra w parze ma mieć oddzielny wątek. Jako klienta można użyć telnetu.

Oceniający: Marcin Borkowski

## Wytyczne dot. projektu

- Każdy student jest przydzielony do projektu przez prowadzących. Tematy projektów znajdują się poniżej, Przydział można odczytać w tabeli ocen.
- Każdy z projektów jest oceniany przez jedną z osób prowadzących. Gotowy kod należy przedstawić osobie wymienionej w projekcie jako oceniający. Projekty nie pokazane nie będą oceniane! Projekt należy zaprezentować podczas check-point'ów na laboratorium.
- Każdy z prowadzących ma wyznaczone 2 check-point'y (patrz kalendarium poniżej) na których należy pokazać postęp prac. Finalne oddanie projektów odbędzie się 15 i 16 czerwca 2015 . Końcowe oceny będą znane w ciągu kilku dni. Wpisy można będzie odbierać codziennie 10-14 w 323 g.MiNI.
- Z projektu można zdobyć 44 punkty z czego 10 punktów można stracić za brak postępów w kodowaniu lub nieobecność na check-point'ach . Działanie, ogólny schemat programu i metody synchronizacji są oceniane podczas prezentowania programu (student od razu poznaje tą część oceny), błędy w kodowaniu są sprawdzane po oddaniu kodu.
- Kod jest oceniany tylko raz tzn. nie można go poprawiać, ale zawsze można się skonsultować z oceniającym przed oddaniem kodu.
- Żaden projekt nie będzie oceniany po 16 czerwca 2015, oczywiście można go oddać wcześniej. Wszystkie projekty są policzone na 5 tygodni pracy.
- Dopracowanie projektu jest również przedmiotem oceny.
- Oceny z projektu wraz z końcowymi ocenami z laboratorium dla wszystkich studentów będą zamieszczone na stronie ocen
- Po okazaniu działającego programu należy wgrać archiwum do katalogu wskazanego przez oceniającego.
- W projekcie muszą wystąpić wątki według standardu POSIX, nie wolno używać procesów potomnych.
- Do synchronizacji używamy tylko mechanizmów POSIX, co wyklucza użycie semaforów SYS V.
- Wszystkie uwagi zamieszczone przy zadaniach cząstkowych obowiązują także w projekcie.
- Projekt musi być napisany w języku C.
- W kodzie należy dopracować wszystkie zasady poprawnego kodowania - długość procedur, zmienne globalne, komentarze przed nietrywialnymi funkcjami.
- Należy kodować z zachowaniem możliwie dużej przenośności kodu, w szczególności obsługa sytuacji błędnych i wartości zwracanych z funkcji systemowych.
- Wszystkie projekty używają jedynie prostego interfejsu tekstowego konsoli
- Gotowy projekt należy spakować do pliku login.tar.gz (bez polskich znaków). Po rozpakowaniu cały projekt ma być w katalogu LOGIN. W katalogu tym ma znajdować sie plik Makefile umożliwiający kompilację projektu za pomocą komendy make (bez nazwy TARGETU). Kompilacja plików .c ma być przeprowadzona z opcją -Wall. W archiwum tar.gz nie wolno umieszczać żadnych plików poza .c, .h, Makefile i ew. plików wynikających z natury zadania.
- Tam gdzie zadanie wymaga danych wejściowych w postaci plików należy przygotować jeden zestaw testowy.
- Wszystkie programy w projekcie wywołane bez parametrów mają pokazywać poprawną składnię wywołania tzw. USAGE.