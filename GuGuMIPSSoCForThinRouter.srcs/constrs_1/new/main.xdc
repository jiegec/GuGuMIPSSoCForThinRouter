set_property -dict {PACKAGE_PIN D18 IOSTANDARD LVCMOS33} [get_ports clk]
set_property -dict {PACKAGE_PIN F22 IOSTANDARD LVCMOS33} [get_ports reset]

# UART
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L19} [get_ports uart_txd]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN K21} [get_ports uart_rxd]