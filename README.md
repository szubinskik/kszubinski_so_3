# Pracownia P3  
## Temat: Zaimplementować prosty system plików  

### Krótki opis  
Celem projektu jest zaimplementowanie systemu plików symulującego skrzynkę pocztową. Gotowy system zamontowany przez użytkownika, ma umożliwić przeglądanie mu struktury skrzynki pocztowej (folderów i maili) tak jak zwykłej struktry katalogów i plików na lokalnym dysku.  

### Dokładniejsza specyfikacja
W celu implementacji logiki systemu posłużę się biblioteką libfuse. Współpracuje ona z modułem FUSE jądra Linuksa, umożliwiając łatwe tworzenie systemów plików w przestrzeniu użytkownika. Rzecz jasna, kernelowy system plików teoretycznie mógłby być wydajniejszy. Przewiduję jednak, że większość czasu działania programu zostanie poświęcona na komunikację z serwerem, więc ewentualna strata wydajności związana z użyciem FUSE jest pomijalna.  
Aby uzyskać strukturę katalogów i dostęp do maili, program będzie komunikował się z serwerem mailowym przy użyciu protokołu IMAP. W swojej pierwotnej wersji protokół ten jest dość okrojony, jednak współczesne serwery implementują kolejne rozszerzenia. Za punkt odniesienia, które komendy protokołu są "rozpowszechnione" uznam serwery usług GMail oraz Office365.  
Do samej komunikacji zamierzam wykorzystać bibliotekę libcurl i napisać moduł parsujący odpowiedzi serwera. Może się zdarzyć, że z jakiś ważnych powodów wykorzystam gotowe rozwiązanie, nie jest to jednak obecnie planowane. Zaletami libcurl są przyjazna licencja i powszechne wykorzystanie, jak również duże możliwości. By komunikować się w sposób bezpieczny z serwerem, program będzie wymagał kilku dodatkowych bibliotek, takich jak np. OpenSSL. Zostaną one wymienione w finalnym opisie projektu.
Wreszcie, lista funkcjonalności systemu plików (w kolejności od najbardziej porządanych):
  1. Wypisywanie zawartości katalogów (z standardową ścieżką);
  2. Odczytywanie podanych maili;
  3. Tworzenie, kopiowanie i usuwanie katalogów;
  4. Usuwanie maili (jest to nieco bardziej złożone ze względu na specyfikację protokołu IMAP, ale można zdefiniować porządne zachowanie);
  5. Przenoszenie i kopiowanie maili. Zwróćmy uwagę, że nie ma operacji tworzenia maili - jej ew. implementacja (i co by to miało znaczyć) jest do rozstrzygnięcia;
  6. Dane o mailu - to co byśmy chcieli znać wywołując `stat`, np. datę modyfikacji, autor (mógłby być adres mailowy).
