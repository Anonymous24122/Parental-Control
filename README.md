# 🛡️ C++ DNS Parental Control Filter - Module (IPv4)

Oto lekki, asynchroniczny mini-serwer DNS napisany w C++ z wykorzystaniem interfejsu Winsock2, który służy jako podstawowy moduł kontroli rodzicielskiej do filtrowania niechcianych domen sieciowych w protokole **IPv4**.

---

### ⚠️ WAŻNY KOMUNIKAT (STATUS PROJEKTU)

* **Wersja:** `alpha v0.0.1`
* **Stan:** Projekt znajduje się w fazie bardzo wczesnego rozwoju. Służy głównie jako eksperymentalny Proof of Concept (PoC) implementacji asynchronicznych obietnic (Promises) oraz gniazd sieciowych w systemie Windows.
* **Gwarancja działania:** Twórca **nie daje absolutnie żadnej gwarancji**, że ten kod zadziała stabilnie na każdej maszynie. W szczególności projekt **może nie uruchomić się lub działać wadliwie na komputerach wyposażonych w procesor Intel Celeron pod kontrolą systemu Windows 3.01** (oraz większości innych systemów, jeśli Winsock odmówi współpracy). 

---

### 📄 Licencja i Warunki Użytkowania

Korzystając z tego kodu, kompilując go lub uruchamiając, **automatycznie akceptujesz warunki licencji MIT**. 

> *Oprogramowanie jest dostarczane „takie, jakie jest” (As Is), bez jakiejkolwiek gwarancji – wyraźnej lub dorozumianej. Używasz go w sieci domowej wyłącznie na własną odpowiedzialność.*

---

### 🚀 Jak to działa?

Aplikacja uruchamia lokalny serwer DNS obsługujący wyłącznie adresację **IPv4** na porcie `53` (pod adresem pętli zwrotnej `127.0.0.1`). 
1. Przechwytuje zapytania DNS wysyłane przez system.
2. Dekoduje nazwę szukanej domeny z pakietu UDP.
3. Sprawdza domenę na wbudowanej liście filtrów (w strukturze `std::unordered_map`).
4. **Filtrowanie:** Jeśli domena znajduje się na białej liście, zapytanie jest przekazywane dalej do zewnętrznego serwera DNS IPv4 (np. bramy sieciowej routera pod adresem `192.168.0.1`), aby poprawnie rozwiązać adres. Jeśli domeny nie ma na liście – serwer zwraca pustą/zablokowaną odpowiedź do klienta, skutecznie odcinając dostęp do danej strony.

---

### Konfiguracja
1. Naciśnij `Win + R`, wpisz `pwsh.exe` lub `powershell` i zatwierdź kombinacją `Ctrl + Shift + Enter`.
2. Wklej tę linijkę do powershella (wyłączy obsługę Ipv6 na karcie Wi-Fi) i zatwierdź `Enterem`:
``` pwsh
Disable-NetAdapterBinding -Name "Wi-Fi" -ComponentID ms_tcpip6
```
3. Ustaw adres serwera dns na 127.0.0.1:
``` pwsh
Set-DnsClientServerAddress -InterfaceIndex 8 -ServerAddresses ("127.0.0.1")
```
4. Uruchom `DNS.exe`.
5. Zamknij `DNS.exe` i wklej to do powershell (to zresetuje adresy dns do stanu sprzed zmiany):
``` pwsh
Get-NetAdapter | ForEach-Object {
    $name = $_.Name
    # Przywraca automatyczny DNS dla IPv4 na tej karcie
    netsh interface ipv4 set dns name="$name" source=dhcp
    # Przywraca automatyczny DNS dla IPv6 na tej karcie
    netsh interface ipv6 set dns name="$name" source=dhcp
}
```
6. Wklej do powershell to:
``` pwsh
Enable-NetAdapterBinding -Name "Wi-Fi" -ComponentID ms_tcpip6
```
---

#### Wbudowana lista domen (Hardcoded):
Aktualnie moduł testowo monitoruje ruch dla:
* `https://youtube.com`
* `https://google.com`

---

### 🛠️ Cechy Techniczne

* **Obsługa Sieci:** Niskopoziomowa architektura oparta o standard **IPv4** i struktury `sockaddr_in`.
* **Custom JavaScript-like Promises:** Implementacja własnej klasy `Promise` w C++ (obsługującej `.p_then()`, `.p_catch()` oraz `.p_finally()`) opartej na wątkach (`std::thread`) i zmiennych warunkowych (`std::condition_variable`) do asynchronicznej inicjalizacji sieci.
* **Czysty Winsock2:** Bezpośrednia komunikacja na niskim poziomie za pomocą gniazd UDP (`SOCK_DGRAM`).
* **Wsparcie dla UTF-16:** Konsola została przestawiona w tryb szerokich znaków (`_O_U16TEXT`), aby poprawnie renderować logi systemowe i błędy Windowsa w formacie `wchar_t`.

---

### 🏗️ Kompilacja i Uruchomienie

Projekt wymaga środowiska programistycznego systemu Windows oraz konsolidacji z biblioteką sieciową `ws2_32.lib`.

```bash
# Przykładowa kompilacja za pomocą GCC/MinGW (pamiętaj o podlinkowaniu ws2_32)
g++ -O3 main.cpp -o dns_filter.exe -lws2_32 -std=c++20
```

*Uwaga: Do uruchomienia serwera na porcie 53 (standardowy port dla usług DNS) wymagane mogą być uprawnienia administratora systemu Windows.*
