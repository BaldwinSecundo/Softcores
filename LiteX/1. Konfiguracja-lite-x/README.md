# Konfiguracja LiteX (WSL) + Vivado (Windows)

Instrukcja krok po kroku, jak skonfigurować środowisko **LiteX** działające pod **WSL2**, które komunikuje się z **Vivado** zainstalowanym natywnie na **Windows**. Jako przykład wykorzystano zaprogramowanie płytki **PYNQ-Z1** softcorem **RISC-V**.

## Wymagania

- **Windows** z zainstalowanym **Vivado 2025** wraz z dodaną obsługą interesującej nas płytki FPGA (tutaj: PYNQ-Z1)
- **PuTTY** (opcjonalnie, do podglądu portu szeregowego)
- **WSL2** z dystrybucją **Ubuntu**

## Spis treści

1. [Konfiguracja LiteX na WSL](#1-konfiguracja-litex-na-wsl)
2. [Konfigurowanie wrappera Vivado (Windows ↔ WSL)](#2-konfigurowanie-wrappera-vivado-windows--wsl)
3. [Generowanie projektu LiteX](#3-generowanie-projektu-litex)
4. [Kompilacja do bitstreamu](#4-kompilacja-do-bitstreamu)
5. [Wgrywanie na układ](#5-wgrywanie-na-układ)
6. [Połączenie się z układem (UART)](#6-połączenie-się-z-układem-uart)
7. [Debugging](#7-debugging)

---

## 1. Konfiguracja LiteX na WSL

Zainstaluj i uruchom WSL, zaktualizuj system oraz upewnij się, że dostępny jest **Python 3.6+**.

We wspólnej lokalizacji (np. `C:\litex`, czyli pod WSL `/mnt/c/litex`) pobierz i uruchom instalator LiteX:

```bash
wget https://raw.githubusercontent.com/enjoy-digital/litex/master/litex_setup.py
chmod +x litex_setup.py
./litex_setup.py --init --install --user   # --user instaluje do katalogu użytkownika
```

Zainstaluj kompilator pod GCC (RISC-V):

```bash
pip3 install meson
./litex_setup.py --gcc=riscv
```

Źródło: [LiteX – Installation (wiki)](https://github.com/enjoy-digital/litex/wiki/Installation)

## 2. Konfigurowanie wrappera Vivado (Windows ↔ WSL)

Aby móc wywoływać Vivado zainstalowane na Windows bezpośrednio z poziomu WSL, wykorzystujemy gotowe skrypty:

🔗 [wsl-bridge-vivado-skills](https://github.com/Passionate0424/wsl-bridge-vivado-skills)

Skopiuj zawarty w repozytorium skrypt (wrapper) i wskaż w nim lokalizację Vivado na Windows:

```bash
cp scripts/vivado ~/.local/bin/vivado
chmod +x ~/.local/bin/vivado
```

W skrypcie wrappera należy zaktualizować ścieżkę do pliku `vivado.bat` na Windows:

```bash
#!/bin/bash
# Call Windows Vivado and forward all arguments
# Note: Use absolute path for cmd.exe to avoid PATH conflicts

# TODO: Update this path to your actual Windows Vivado batch file location
VIVADO_PATH="C:/AMDDesignTools/2025.2/Vivado/bin/vivado.bat"

if [ ! -f "/mnt/$(echo $VIVADO_PATH | sed 's|:|/|; s|\\|/|' | tr '[:upper:]' '[:lower:]')" ] && [ ! -f "$(wslpath "$VIVADO_PATH")" ]; then
    echo "Warning: Vivado path might be incorrect: $VIVADO_PATH"
fi

/mnt/c/Windows/System32/cmd.exe /c "$VIVADO_PATH" "$@"
```

> Znajdź własną lokalizację instalacji Vivado na Windows (plik `Vivado.bat`) i podmień ją w zmiennej `VIVADO_PATH`.

**Ustawienie ścieżki i uprawnień:**

- Zaktualizuj `PATH`, tak aby katalog z wrapperem był na początku ścieżki:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

## 3. Generowanie projektu LiteX

Przejdź do lokalizacji, gdzie zainstalowany jest LiteX i znajdź przykładowe projekty (targety) – w tym poradniku wykorzystywany jest PYNQ-Z1:

```bash
cd /mnt/c/litex/litex-boards/litex_boards/targets
python digilent_pynq_z.py --build
```

Framework LiteX wygeneruje cały projekt, jednak **nie skompiluje się on jeszcze do bitstreamu** – pojawi się błąd wynikający z niepoprawnej struktury skryptu TCL (wygenerowany jest pod składnię Linuksa, a nie Windows, gdzie zainstalowane jest Vivado).

Powstanie folder `build`, a w nim m.in. folder `digilent_pynq_z1` oraz `gateware`. W folderze `gateware` znajduje się skrypt TCL, który należy jeszcze przetworzyć:

```
build/digilent_pynq_z1/gateware/
```

## 4. Kompilacja do bitstreamu

Wykorzystujemy skrypt z repozytorium [wsl-bridge-vivado-skills](https://github.com/Passionate0424/wsl-bridge-vivado-skills), aby przekonwertować pliki TCL do składni wspieranej przez Windows.

### 4.1 Instalacja bootgen

Do kompilacji do bitstreamu Xilinx potrzebny jest `bootgen`:

```bash
sudo apt install xilinx-bootgen
bootgen   # powinno wyświetlić informację, że narzędzie jest zainstalowane w systemie
```

### 4.2 Naprawa ścieżek TCL – `fix_tcl_paths.sh`

Skrypt konwertuje ścieżki WSL (`/mnt/x/...`) na format Windows (`X:/...`) wewnątrz plików `.tcl`:

```bash
#!/bin/bash
# Helper script to convert WSL paths in TCL files to Windows paths

TARGET_DIR="$1"

if [ -z "$TARGET_DIR" ]; then
    echo "Usage: $0 <directory_containing_tcl_files>"
    exit 1
fi

echo "Fixing TCL paths in $TARGET_DIR..."

# Convert /mnt/d/ to D:/ format (and others)
# Adapting common drive letters c-z
for drive in {c..z}; do
    sed -i "s|/mnt/$drive/|${drive^^}:/|g" "$TARGET_DIR"/*.tcl 2>/dev/null
done

echo "Done."
```

Uruchamiamy skrypt, podając lokalizację folderu `gateware` (tu: `build/digilent_pynq_z1/gateware/`) – konwertuje on ścieżki na składnię wspieraną przez Windows.

### 4.3 Kompilacja bitstreamu

Następnie kompilujemy bitstream (może to chwilę potrwać, ponieważ w tle wywoływane jest Vivado):

> ⚠️ **Uwaga:** w Vivado, w zakładce **Boards**, musisz mieć dodaną swoją płytkę (np. PYNQ-Z1) – w przeciwnym razie skrypt nie rozpozna komponentów układu i kompilacja się nie powiedzie.

W folderze `build/digilent_pynq_z1/gateware/` uruchamiamy skrypt `.sh`:

```bash
./build_digilent_pynq_z1.sh
```

W lokalizacji `build/digilent_pynq_z1/gateware/` powinniśmy otrzymać plik `digilent_pynq_z1.bit` – nasz bitstream.

## 5. Wgrywanie na układ

Za pomocą Vivado – **Hardware Manager** – łączymy się przez USB z układem i programujemy go wygenerowanym bitstreamem (`digilent_pynq_z1.bit`).

Po poprawnym zaprogramowaniu diody na płytce (od **LD0** do **LD5**) powinny się kolorowo zaświecić i zamigać.

## 6. Połączenie się z układem (UART)

Dla PYNQ-Z1 procesor PS (hardware) nie jest skonfigurowany, więc terminal do softcore'u RISC-V nie jest dostępny przez niego bezpośrednio. Należy podłączyć konwerter **USB-TTL (USB na UART)** pod złącze **PMOD_A** (⚠️ nie pod PMOD B! – złącza są opisane na płytce):

- **pmoda:0** – pierwszy element listy, czyli **Y18** (to Twój **TX**)
- **pmoda:1** – drugi element listy, czyli **Y19** (to Twój **RX**)

Przypisanie pinów (constraints) dla projektu demonstracyjnego dostępne jest w lokalizacji:

```
/litex/litex-boards/litex_boards/platforms/[nazwa_skryptu]
```

np. `digilent_pynq_z1.py`.

Po prawidłowym podłączeniu konwertera USB-TTL, za pomocą konsoli lub PuTTY podłącz się pod odpowiedni port COM (np. `COM8`) i ustaw parametry portu szeregowego:

- **Baudrate:** `115200`
- **Flow control:** `NONE` (jeśli występują problemy z połączeniem)

Jeśli po podłączeniu ekran jest czarny, wciśnij dowolny klawisz (np. Enter) – powinna pojawić się konsola LiteX BIOS:

```
litex> help

LiteX BIOS, available commands:

leds              - Set LEDs value
flush_cpu_dcache  - Flush CPU data cache
crc               - Compute CRC32 of a part of the address space
ident             - Identifier of the system
help              - Print this help

serialboot        - Boot from Serial (SFL)
reboot            - Reboot
boot              - Boot from Memory

mem_cmp           - Compare memory content
mem_speed         - Test memory speed
mem_test          - Test memory access
mem_copy          - Copy address space
mem_write         - Write address space
mem_read          - Read address space
mem_list          - List available memory regions

litex> leds
leds <value>
litex> leds 2
```

## 7. Debugging

### Problem z formatem plików tekstowych utworzonych na Windows

Ponieważ praca odbywa się w WSL na plikach trzymanych na partycji Windows (`/mnt/c/...`), upewnij się, że edytory tekstowe na Windows (np. Notepad++) nie zapisują skryptów w formacie Windows (**CRLF**), tylko w formacie uniksowym (**LF**). W razie potrzeby można je przekonwertować.

Poniżej skrypt konwertujący wrapper Vivado oraz skrypt budowania/kompilacji bitstreamu uzyskany z frameworka LiteX:

```bash
sudo apt update && sudo apt install dos2unix -y

dos2unix ./gateware/build_digilent_pynq_z1.sh
dos2unix /home/ihor/.local/bin/vivado
```

---

## Przydatne linki

- [LiteX – repozytorium i dokumentacja instalacji](https://github.com/enjoy-digital/litex/wiki/Installation)
- [wsl-bridge-vivado-skills – wrapper Vivado dla WSL](https://github.com/Passionate0424/wsl-bridge-vivado-skills)
