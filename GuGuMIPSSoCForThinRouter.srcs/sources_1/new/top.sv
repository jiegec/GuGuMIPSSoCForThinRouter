`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 08/28/2019 07:15:23 PM
// Design Name: 
// Module Name: top
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module top(
    input clk,
    input reset,

    output [19:0] base_ram_addr,
    output [3:0] base_ram_be_n,
    output base_ram_ce_n,
    output base_ram_oe_n,
    output base_ram_we_n,
    inout [31:0] base_ram_data,

    output [19:0] ext_ram_addr,
    output [3:0] ext_ram_be_n,
    output ext_ram_ce_n,
    output ext_ram_oe_n,
    output ext_ram_we_n,
    inout [31:0] ext_ram_data,

    output [22:0] flash_a,
    output flash_byte_n,
    output flash_ce_n,
    output flash_oe_n,
    output flash_rp_n,
    output flash_vpen,
    output flash_we_n,
    inout [15:0] flash_d,

    input [3:0] eth_rgmii_rd,
    input eth_rgmii_rx_ctl,
    input eth_rgmii_rxc,
    output [3:0] eth_rgmii_td,
    output eth_rgmii_tx_ctl,
    output eth_rgmii_txc,
    output eth_rst_n,

    output eth_spi_mosi, // MOSI
    input eth_spi_miso, // MISO
    output eth_spi_sck,
    output eth_spi_ss_n,

    input uart_rxd,
    output uart_txd,

    output uart_rdn,
    output uart_wrn,
    input uart_dataready,
    input uart_tbre,
    input uart_tsre
    );

    assign uart_rdn = 1;
    assign uart_wrn = 1;

    assign flash_byte_n = 1;
    assign flash_vpen = 1;

    design_mips_wrapper inst(
        .clk(clk),
        .reset(reset),

        .base_ram_addr({base_ram_addr, 2'b0}),
        .base_ram_ben(base_ram_be_n),
        .base_ram_ce_n(base_ram_ce_n),
        .base_ram_dq_io(base_ram_data),
        .base_ram_oen(base_ram_oe_n),
        .base_ram_wen(base_ram_we_n),
        .base_ram_wait(0),

        .ext_ram_addr({ext_ram_addr, 2'b0}),
        .ext_ram_ben(ext_ram_be_n),
        .ext_ram_ce_n(ext_ram_ce_n),
        .ext_ram_dq_io(ext_ram_data),
        .ext_ram_oen(ext_ram_oe_n),
        .ext_ram_wen(ext_ram_we_n),
        .ext_ram_wait(0),

        .flash_addr(flash_a),
        .flash_ce_n(flash_ce_n),
        .flash_dq_io(flash_d),
        .flash_oen(flash_oe_n),
        .flash_rpn(flash_rp_n),
        .flash_wen(flash_we_n),
        .flash_wait(0),

        .eth_rgmii_rd(eth_rgmii_rd),
        .eth_rgmii_rx_ctl(eth_rgmii_rx_ctl),
        .eth_rgmii_rxc(eth_rgmii_rxc),
        .eth_rgmii_td(eth_rgmii_td),
        .eth_rgmii_tx_ctl(eth_rgmii_tx_ctl),
        .eth_rgmii_txc(eth_rgmii_txc),
        .eth_rst_n(eth_rst_n),

        .eth_spi_mosi(eth_spi_mosi),
        .eth_spi_miso(eth_spi_miso),
        .eth_spi_sck(eth_spi_sck),
        .eth_spi_ss_n(eth_spi_ss_n),

        .uart_rxd(uart_rxd),
        .uart_txd(uart_txd)
    );
endmodule
