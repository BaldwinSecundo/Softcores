## PYNQ-Z1 - MicroBlaze V (RISC-V) softcore
## Piny sprawdź względem swojej płytki i schematu adaptera UART.

# LED (LD0) sterowany z AXI GPIO
set_property PACKAGE_PIN R14 [get_ports {gpio_io_o_0[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {gpio_io_o_0[0]}]

# Reset (SW0) - stan wysoki = procesor w resecie
set_property PACKAGE_PIN M20 [get_ports reset_rtl]
set_property IOSTANDARD LVCMOS33 [get_ports reset_rtl]

# LED sygnalizujący aktywny reset (LD1)
set_property PACKAGE_PIN P14 [get_ports rst_led]
set_property IOSTANDARD LVCMOS33 [get_ports rst_led]

# UART na złączu PMOD JA
set_property PACKAGE_PIN Y18 [get_ports rx_0]
set_property IOSTANDARD LVCMOS33 [get_ports rx_0]
set_property PACKAGE_PIN Y19 [get_ports tx_0]
set_property IOSTANDARD LVCMOS33 [get_ports tx_0]

## Zegar sys_clock (125 MHz) - zwykle przypisywany automatycznie przez board automation.
## Odkomentuj tylko, jeśli Vivado zgłasza brak przypisania:
# set_property PACKAGE_PIN H16 [get_ports sys_clock]
# set_property IOSTANDARD LVCMOS33 [get_ports sys_clock]
# create_clock -period 8.000 -name sys_clk_pin -waveform {0 4} [get_ports sys_clock]
