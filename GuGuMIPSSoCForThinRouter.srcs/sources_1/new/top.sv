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

    output [19:0] base_ram_addr,
    output [3:0] base_ram_be_n,
    output base_ram_ce_n,
    output base_ram_oe_n,
    output base_ram_we_n,
    inout [31:0]base_ram_data,

    input reset,
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

    design_mips_wrapper inst(
        .clk(clk),

        .base_ram_addr({base_ram_addr, 2'b0}),
        .base_ram_ben(base_ram_be_n),
        .base_ram_ce_n(base_ram_ce_n),
        .base_ram_dq_io(base_ram_data),
        .base_ram_oen(base_ram_oe_n),
        .base_ram_wen(base_ram_we_n),
        .base_ram_wait(0),

        .reset(reset),

        .uart_rxd(uart_rxd),
        .uart_txd(uart_txd)
    );
endmodule
