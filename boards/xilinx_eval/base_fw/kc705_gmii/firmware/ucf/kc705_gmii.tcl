# Ethernet RefClk (125MHz)
create_clock -period 5.000 -name sysclk [get_ports sysclk_p]

# Incoming GMII rx clock
create_clock -period 8.000 -name gmii_rx_clk [get_ports gmii_rx_clk]
set_input_delay -clock [get_clocks gmii_rx_clk] -min 5.5 -max 0.5 [get_ports {gmii_rxd[*] gmii_rx_dv gmii_rx_er}]

set_clock_groups -asynchronous -group [get_clocks -include_generated_clocks gmii_rx_clk]

set_false_path -through [get_pins infra/clocks/rst_reg/Q]
set_false_path -through [get_nets infra/clocks/nuke_i]

set_property IOB TRUE [get_cells {infra/eth/gmii_txd_reg* infra/eth/gmii_tx_*_reg}]

set_property IOSTANDARD LVDS [get_ports {sysclk_*}]
set_property PACKAGE_PIN AD12 [get_ports sysclk_p]
set_property PACKAGE_PIN AD11 [get_ports sysclk_n]

set_property IOSTANDARD LVCMOS15 [get_ports {leds[*]}]
set_property SLEW SLOW [get_ports {leds[*]}]
set_property PACKAGE_PIN AB8 [get_ports {leds[0]}]
set_property PACKAGE_PIN AA8 [get_ports {leds[1]}]
set_property PACKAGE_PIN AC9 [get_ports {leds[2]}]
set_property PACKAGE_PIN AB9 [get_ports {leds[3]}]

set_property IOSTANDARD LVCMOS25 [get_ports {gmii* phy_rst}]
set_property PACKAGE_PIN K30 [get_ports {gmii_gtx_clk}]
set_property PACKAGE_PIN M27 [get_ports {gmii_tx_en}]
set_property PACKAGE_PIN N29 [get_ports {gmii_tx_er}]
set_property PACKAGE_PIN N27 [get_ports {gmii_txd[0]}]
set_property PACKAGE_PIN N25 [get_ports {gmii_txd[1]}]
set_property PACKAGE_PIN M29 [get_ports {gmii_txd[2]}]
set_property PACKAGE_PIN L28 [get_ports {gmii_txd[3]}]
set_property PACKAGE_PIN J26 [get_ports {gmii_txd[4]}]
set_property PACKAGE_PIN K26 [get_ports {gmii_txd[5]}]
set_property PACKAGE_PIN L30 [get_ports {gmii_txd[6]}]
set_property PACKAGE_PIN J28 [get_ports {gmii_txd[7]}]
set_property PACKAGE_PIN U27 [get_ports {gmii_rx_clk}]
set_property PACKAGE_PIN R28 [get_ports {gmii_rx_dv}]
set_property PACKAGE_PIN V26 [get_ports {gmii_rx_er}]
set_property PACKAGE_PIN U30 [get_ports {gmii_rxd[0]}]
set_property PACKAGE_PIN U25 [get_ports {gmii_rxd[1]}]
set_property PACKAGE_PIN T25 [get_ports {gmii_rxd[2]}]
set_property PACKAGE_PIN U28 [get_ports {gmii_rxd[3]}]
set_property PACKAGE_PIN R19 [get_ports {gmii_rxd[4]}]
set_property PACKAGE_PIN T27 [get_ports {gmii_rxd[5]}]
set_property PACKAGE_PIN T26 [get_ports {gmii_rxd[6]}]
set_property PACKAGE_PIN T28 [get_ports {gmii_rxd[7]}]
set_property PACKAGE_PIN L20 [get_ports {phy_rst}]
