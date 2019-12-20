
################################################################
# This is a generated script based on design: design_mips
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2018.3
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_msg_id "BD_TCL-109" "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source design_mips_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# eth_conf, mycpu_top

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7a100tfgg676-2L
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name design_mips

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_msg_id "BD_TCL-005" "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
   return $nRet
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set base_ram [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:emc_rtl:1.0 base_ram ]
  set eth_rgmii [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:rgmii_rtl:1.0 eth_rgmii ]
  set ext_ram [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:emc_rtl:1.0 ext_ram ]
  set flash [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:emc_rtl:1.0 flash ]
  set uart [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:uart_rtl:1.0 uart ]

  # Create ports
  set clk [ create_bd_port -dir I -type clk clk ]
  set_property -dict [ list \
   CONFIG.FREQ_HZ {50000000} \
 ] $clk
  set eth_rst_n [ create_bd_port -dir O -from 0 -to 0 -type rst eth_rst_n ]
  set_property -dict [ list \
   CONFIG.POLARITY {ACTIVE_LOW} \
 ] $eth_rst_n
  set eth_spi_miso [ create_bd_port -dir I eth_spi_miso ]
  set eth_spi_mosi [ create_bd_port -dir O eth_spi_mosi ]
  set eth_spi_sck [ create_bd_port -dir O eth_spi_sck ]
  set eth_spi_ss_n [ create_bd_port -dir O eth_spi_ss_n ]
  set reset [ create_bd_port -dir I -type rst reset ]
  set_property -dict [ list \
   CONFIG.POLARITY {ACTIVE_HIGH} \
 ] $reset

  # Create instance: axi_bram_ctrl_0, and set properties
  set axi_bram_ctrl_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:4.1 axi_bram_ctrl_0 ]
  set_property -dict [ list \
   CONFIG.SINGLE_PORT_BRAM {1} \
 ] $axi_bram_ctrl_0

  # Create instance: axi_bram_ctrl_0_bram, and set properties
  set axi_bram_ctrl_0_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.4 axi_bram_ctrl_0_bram ]
  set_property -dict [ list \
   CONFIG.Coe_File {../../../../../../GuGuMIPS/boot/bootloader.coe} \
   CONFIG.Load_Init_File {true} \
   CONFIG.Memory_Type {Single_Port_ROM} \
   CONFIG.Port_A_Write_Rate {0} \
   CONFIG.Use_Byte_Write_Enable {false} \
 ] $axi_bram_ctrl_0_bram

  # Create instance: axi_bram_ctrl_1, and set properties
  set axi_bram_ctrl_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:4.1 axi_bram_ctrl_1 ]
  set_property -dict [ list \
   CONFIG.SINGLE_PORT_BRAM {1} \
 ] $axi_bram_ctrl_1

  # Create instance: axi_bram_ctrl_1_bram, and set properties
  set axi_bram_ctrl_1_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.4 axi_bram_ctrl_1_bram ]
  set_property -dict [ list \
   CONFIG.Coe_File {no_coe_file_loaded} \
   CONFIG.Load_Init_File {false} \
   CONFIG.Memory_Type {Single_Port_RAM} \
   CONFIG.Port_A_Write_Rate {50} \
   CONFIG.Use_Byte_Write_Enable {true} \
 ] $axi_bram_ctrl_1_bram

  # Create instance: axi_bram_ctrl_2, and set properties
  set axi_bram_ctrl_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:4.1 axi_bram_ctrl_2 ]
  set_property -dict [ list \
   CONFIG.DATA_WIDTH {128} \
   CONFIG.ECC_TYPE {0} \
   CONFIG.SINGLE_PORT_BRAM {1} \
 ] $axi_bram_ctrl_2

  # Create instance: axi_dma_0, and set properties
  set axi_dma_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_0 ]
  set_property -dict [ list \
   CONFIG.c_include_sg {1} \
   CONFIG.c_m_axis_mm2s_tdata_width {8} \
   CONFIG.c_sg_include_stscntrl_strm {0} \
 ] $axi_dma_0

  # Create instance: axi_emc_base, and set properties
  set axi_emc_base [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_emc:3.0 axi_emc_base ]
  set_property -dict [ list \
   CONFIG.C_INCLUDE_DATAWIDTH_MATCHING_0 {0} \
   CONFIG.C_MAX_MEM_WIDTH {32} \
   CONFIG.C_MEM0_TYPE {1} \
   CONFIG.C_MEM0_WIDTH {32} \
   CONFIG.C_NUM_BANKS_MEM {1} \
   CONFIG.C_S_AXI_EN_REG {0} \
   CONFIG.C_TLZWE_PS_MEM_0 {0} \
   CONFIG.C_TWC_PS_MEM_0 {15000} \
   CONFIG.C_TWP_PS_MEM_0 {12000} \
 ] $axi_emc_base

  # Create instance: axi_emc_ext, and set properties
  set axi_emc_ext [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_emc:3.0 axi_emc_ext ]
  set_property -dict [ list \
   CONFIG.C_INCLUDE_DATAWIDTH_MATCHING_0 {0} \
   CONFIG.C_MAX_MEM_WIDTH {32} \
   CONFIG.C_MEM0_TYPE {1} \
   CONFIG.C_MEM0_WIDTH {32} \
 ] $axi_emc_ext

  # Create instance: axi_emc_flash, and set properties
  set axi_emc_flash [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_emc:3.0 axi_emc_flash ]
  set_property -dict [ list \
   CONFIG.C_INCLUDE_DATAWIDTH_MATCHING_0 {1} \
   CONFIG.C_MAX_MEM_WIDTH {16} \
   CONFIG.C_MEM0_TYPE {5} \
   CONFIG.C_MEM0_WIDTH {16} \
 ] $axi_emc_flash

  # Create instance: axi_gpio_0, and set properties
  set axi_gpio_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0 ]
  set_property -dict [ list \
   CONFIG.C_ALL_OUTPUTS {1} \
   CONFIG.C_GPIO_WIDTH {1} \
 ] $axi_gpio_0

  # Create instance: axi_mem_intercon, and set properties
  set axi_mem_intercon [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon ]
  set_property -dict [ list \
   CONFIG.NUM_MI {10} \
   CONFIG.NUM_SI {5} \
   CONFIG.STRATEGY {1} \
 ] $axi_mem_intercon

  # Create instance: axi_timer_0, and set properties
  set axi_timer_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0 ]
  set_property -dict [ list \
   CONFIG.mode_64bit {1} \
 ] $axi_timer_0

  # Create instance: axi_uartlite_0, and set properties
  set axi_uartlite_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_uartlite:2.0 axi_uartlite_0 ]
  set_property -dict [ list \
   CONFIG.C_BAUDRATE {115200} \
 ] $axi_uartlite_0

  # Create instance: clk_wiz, and set properties
  set clk_wiz [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz ]
  set_property -dict [ list \
   CONFIG.CLKOUT1_JITTER {204.383} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {40.000} \
   CONFIG.CLKOUT2_JITTER {285.743} \
   CONFIG.CLKOUT2_PHASE_ERROR {164.985} \
   CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {10.000} \
   CONFIG.CLKOUT2_USED {true} \
   CONFIG.CLKOUT3_REQUESTED_OUT_FREQ {200} \
   CONFIG.CLKOUT3_USED {true} \
   CONFIG.CLKOUT4_REQUESTED_OUT_FREQ {125} \
   CONFIG.CLKOUT4_USED {true} \
   CONFIG.CLKOUT5_JITTER {172.249} \
   CONFIG.CLKOUT5_PHASE_ERROR {164.985} \
   CONFIG.CLKOUT5_REQUESTED_OUT_FREQ {80.000} \
   CONFIG.CLKOUT5_USED {true} \
   CONFIG.CLK_OUT1_PORT {clk_cpu} \
   CONFIG.CLK_OUT2_PORT {clk_emc} \
   CONFIG.CLK_OUT3_PORT {clk_200M} \
   CONFIG.CLK_OUT4_PORT {clk_125M} \
   CONFIG.CLK_OUT5_PORT {clk_router} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {25.000} \
   CONFIG.MMCM_CLKOUT1_DIVIDE {100} \
   CONFIG.MMCM_CLKOUT4_DIVIDE {13} \
   CONFIG.MMCM_DIVCLK_DIVIDE {1} \
   CONFIG.NUM_OUT_CLKS {5} \
 ] $clk_wiz

  # Create instance: eth_conf_0, and set properties
  set block_name eth_conf
  set block_cell_name eth_conf_0
  if { [catch {set eth_conf_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $eth_conf_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: jtag_axi_0, and set properties
  set jtag_axi_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:jtag_axi:1.2 jtag_axi_0 ]

  # Create instance: mycpu_top_0, and set properties
  set block_name mycpu_top
  set block_cell_name mycpu_top_0
  if { [catch {set mycpu_top_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $mycpu_top_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: router_0, and set properties
  set router_0 [ create_bd_cell -type ip -vlnv me.jiegec:ip:router:1.6 router_0 ]

  # Create instance: rst_clk_50M, and set properties
  set rst_clk_50M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_clk_50M ]

  # Create instance: rst_clk_wiz_50M, and set properties
  set rst_clk_wiz_50M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_clk_wiz_50M ]

  # Create instance: system_ila_0, and set properties
  set system_ila_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:system_ila:1.1 system_ila_0 ]
  set_property -dict [ list \
   CONFIG.C_MON_TYPE {MIX} \
   CONFIG.C_NUM_MONITOR_SLOTS {1} \
   CONFIG.C_NUM_OF_PROBES {7} \
   CONFIG.C_PROBE0_TYPE {0} \
   CONFIG.C_PROBE1_TYPE {0} \
   CONFIG.C_PROBE2_TYPE {0} \
   CONFIG.C_PROBE3_TYPE {0} \
   CONFIG.C_PROBE4_TYPE {0} \
   CONFIG.C_PROBE5_TYPE {0} \
   CONFIG.C_PROBE6_TYPE {0} \
   CONFIG.C_SLOT_0_APC_EN {0} \
   CONFIG.C_SLOT_0_AXI_AR_SEL_DATA {1} \
   CONFIG.C_SLOT_0_AXI_AR_SEL_TRIG {1} \
   CONFIG.C_SLOT_0_AXI_AW_SEL_DATA {1} \
   CONFIG.C_SLOT_0_AXI_AW_SEL_TRIG {1} \
   CONFIG.C_SLOT_0_AXI_B_SEL_DATA {1} \
   CONFIG.C_SLOT_0_AXI_B_SEL_TRIG {1} \
   CONFIG.C_SLOT_0_AXI_R_SEL_DATA {1} \
   CONFIG.C_SLOT_0_AXI_R_SEL_TRIG {1} \
   CONFIG.C_SLOT_0_AXI_W_SEL_DATA {1} \
   CONFIG.C_SLOT_0_AXI_W_SEL_TRIG {1} \
   CONFIG.C_SLOT_0_INTF_TYPE {xilinx.com:interface:aximm_rtl:1.0} \
   CONFIG.C_SLOT_1_APC_EN {0} \
   CONFIG.C_SLOT_1_AXI_DATA_SEL {1} \
   CONFIG.C_SLOT_1_AXI_TRIG_SEL {1} \
   CONFIG.C_SLOT_1_INTF_TYPE {xilinx.com:interface:axis_rtl:1.0} \
   CONFIG.C_SLOT_1_TYPE {0} \
   CONFIG.C_SLOT_2_APC_EN {0} \
   CONFIG.C_SLOT_2_AXI_DATA_SEL {1} \
   CONFIG.C_SLOT_2_AXI_TRIG_SEL {1} \
   CONFIG.C_SLOT_2_INTF_TYPE {xilinx.com:interface:axis_rtl:1.0} \
 ] $system_ila_0

  # Create instance: util_vector_logic_0, and set properties
  set util_vector_logic_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:util_vector_logic:2.0 util_vector_logic_0 ]
  set_property -dict [ list \
   CONFIG.C_OPERATION {not} \
   CONFIG.LOGO_FILE {data/sym_notgate.png} \
 ] $util_vector_logic_0

  # Create instance: vio_0, and set properties
  set vio_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:vio:3.0 vio_0 ]
  set_property -dict [ list \
   CONFIG.C_EN_PROBE_IN_ACTIVITY {0} \
   CONFIG.C_NUM_PROBE_IN {0} \
   CONFIG.C_PROBE_OUT0_INIT_VAL {0x1} \
 ] $vio_0

  # Create instance: xlconstant_0, and set properties
  set xlconstant_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0 ]
  set_property -dict [ list \
   CONFIG.CONST_VAL {0} \
 ] $xlconstant_0

  # Create interface connections
  connect_bd_intf_net -intf_net axi_bram_ctrl_0_BRAM_PORTA [get_bd_intf_pins axi_bram_ctrl_0/BRAM_PORTA] [get_bd_intf_pins axi_bram_ctrl_0_bram/BRAM_PORTA]
  connect_bd_intf_net -intf_net axi_bram_ctrl_1_BRAM_PORTA [get_bd_intf_pins axi_bram_ctrl_1/BRAM_PORTA] [get_bd_intf_pins axi_bram_ctrl_1_bram/BRAM_PORTA]
  connect_bd_intf_net -intf_net axi_bram_ctrl_2_BRAM_PORTA [get_bd_intf_pins axi_bram_ctrl_2/BRAM_PORTA] [get_bd_intf_pins router_0/routing_table]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXIS_MM2S [get_bd_intf_pins axi_dma_0/M_AXIS_MM2S] [get_bd_intf_pins router_0/axis_rxd]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_MM2S [get_bd_intf_pins axi_dma_0/M_AXI_MM2S] [get_bd_intf_pins axi_mem_intercon/S02_AXI]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_S2MM [get_bd_intf_pins axi_dma_0/M_AXI_S2MM] [get_bd_intf_pins axi_mem_intercon/S03_AXI]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_SG [get_bd_intf_pins axi_dma_0/M_AXI_SG] [get_bd_intf_pins axi_mem_intercon/S04_AXI]
  connect_bd_intf_net -intf_net axi_emc_0_EMC_INTF [get_bd_intf_ports flash] [get_bd_intf_pins axi_emc_flash/EMC_INTF]
  connect_bd_intf_net -intf_net axi_emc_base_EMC_INTF [get_bd_intf_ports base_ram] [get_bd_intf_pins axi_emc_base/EMC_INTF]
  connect_bd_intf_net -intf_net axi_emc_ext_EMC_INTF [get_bd_intf_ports ext_ram] [get_bd_intf_pins axi_emc_ext/EMC_INTF]
  connect_bd_intf_net -intf_net axi_mem_intercon_M00_AXI [get_bd_intf_pins axi_bram_ctrl_0/S_AXI] [get_bd_intf_pins axi_mem_intercon/M00_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M01_AXI [get_bd_intf_pins axi_mem_intercon/M01_AXI] [get_bd_intf_pins axi_uartlite_0/S_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M02_AXI [get_bd_intf_pins axi_bram_ctrl_1/S_AXI] [get_bd_intf_pins axi_mem_intercon/M02_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M03_AXI [get_bd_intf_pins axi_emc_base/S_AXI_MEM] [get_bd_intf_pins axi_mem_intercon/M03_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M04_AXI [get_bd_intf_pins axi_emc_ext/S_AXI_MEM] [get_bd_intf_pins axi_mem_intercon/M04_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M05_AXI [get_bd_intf_pins axi_emc_flash/S_AXI_MEM] [get_bd_intf_pins axi_mem_intercon/M05_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M06_AXI [get_bd_intf_pins axi_dma_0/S_AXI_LITE] [get_bd_intf_pins axi_mem_intercon/M06_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M07_AXI [get_bd_intf_pins axi_bram_ctrl_2/S_AXI] [get_bd_intf_pins axi_mem_intercon/M07_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M08_AXI [get_bd_intf_pins axi_gpio_0/S_AXI] [get_bd_intf_pins axi_mem_intercon/M08_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M09_AXI [get_bd_intf_pins axi_mem_intercon/M09_AXI] [get_bd_intf_pins axi_timer_0/S_AXI]
  connect_bd_intf_net -intf_net axi_uartlite_0_UART [get_bd_intf_ports uart] [get_bd_intf_pins axi_uartlite_0/UART]
  connect_bd_intf_net -intf_net jtag_axi_0_M_AXI [get_bd_intf_pins axi_mem_intercon/S01_AXI] [get_bd_intf_pins jtag_axi_0/M_AXI]
  connect_bd_intf_net -intf_net mycpu_top_0_interface_aximm [get_bd_intf_pins axi_mem_intercon/S00_AXI] [get_bd_intf_pins mycpu_top_0/interface_aximm]
connect_bd_intf_net -intf_net [get_bd_intf_nets mycpu_top_0_interface_aximm] [get_bd_intf_pins axi_mem_intercon/S00_AXI] [get_bd_intf_pins system_ila_0/SLOT_0_AXI]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_intf_nets mycpu_top_0_interface_aximm]
  connect_bd_intf_net -intf_net router_0_axis_txd [get_bd_intf_pins axi_dma_0/S_AXIS_S2MM] [get_bd_intf_pins router_0/axis_txd]
  connect_bd_intf_net -intf_net router_0_rgmii [get_bd_intf_ports eth_rgmii] [get_bd_intf_pins router_0/rgmii]

  # Create port connections
  connect_bd_net -net axi_gpio_0_gpio_io_o [get_bd_pins axi_gpio_0/gpio_io_o] [get_bd_pins router_0/reset_n]
  connect_bd_net -net clk_2 [get_bd_ports clk] [get_bd_pins clk_wiz/clk_in1]
  connect_bd_net -net clk_wiz_clk_125M [get_bd_pins clk_wiz/clk_125M] [get_bd_pins router_0/clk_125M]
  connect_bd_net -net clk_wiz_clk_200M [get_bd_pins clk_wiz/clk_200M] [get_bd_pins router_0/clk_200M]
  connect_bd_net -net clk_wiz_clk_cpu [get_bd_pins axi_bram_ctrl_0/s_axi_aclk] [get_bd_pins axi_bram_ctrl_1/s_axi_aclk] [get_bd_pins axi_bram_ctrl_2/s_axi_aclk] [get_bd_pins axi_dma_0/m_axi_mm2s_aclk] [get_bd_pins axi_dma_0/m_axi_s2mm_aclk] [get_bd_pins axi_dma_0/m_axi_sg_aclk] [get_bd_pins axi_dma_0/s_axi_lite_aclk] [get_bd_pins axi_gpio_0/s_axi_aclk] [get_bd_pins axi_mem_intercon/ACLK] [get_bd_pins axi_mem_intercon/M00_ACLK] [get_bd_pins axi_mem_intercon/M01_ACLK] [get_bd_pins axi_mem_intercon/M02_ACLK] [get_bd_pins axi_mem_intercon/M06_ACLK] [get_bd_pins axi_mem_intercon/M07_ACLK] [get_bd_pins axi_mem_intercon/M08_ACLK] [get_bd_pins axi_mem_intercon/M09_ACLK] [get_bd_pins axi_mem_intercon/S00_ACLK] [get_bd_pins axi_mem_intercon/S01_ACLK] [get_bd_pins axi_mem_intercon/S02_ACLK] [get_bd_pins axi_mem_intercon/S03_ACLK] [get_bd_pins axi_mem_intercon/S04_ACLK] [get_bd_pins axi_timer_0/s_axi_aclk] [get_bd_pins axi_uartlite_0/s_axi_aclk] [get_bd_pins clk_wiz/clk_cpu] [get_bd_pins jtag_axi_0/aclk] [get_bd_pins mycpu_top_0/aclk] [get_bd_pins router_0/axis_clk] [get_bd_pins rst_clk_wiz_50M/slowest_sync_clk] [get_bd_pins system_ila_0/clk]
  connect_bd_net -net clk_wiz_clk_out2 [get_bd_pins axi_emc_base/rdclk] [get_bd_pins axi_emc_base/s_axi_aclk] [get_bd_pins axi_emc_ext/rdclk] [get_bd_pins axi_emc_ext/s_axi_aclk] [get_bd_pins axi_emc_flash/rdclk] [get_bd_pins axi_emc_flash/s_axi_aclk] [get_bd_pins axi_mem_intercon/M03_ACLK] [get_bd_pins axi_mem_intercon/M04_ACLK] [get_bd_pins axi_mem_intercon/M05_ACLK] [get_bd_pins clk_wiz/clk_emc] [get_bd_pins eth_conf_0/clk] [get_bd_pins rst_clk_50M/slowest_sync_clk] [get_bd_pins vio_0/clk]
  connect_bd_net -net clk_wiz_clk_router [get_bd_pins clk_wiz/clk_router] [get_bd_pins router_0/clk]
  connect_bd_net -net clk_wiz_locked [get_bd_pins clk_wiz/locked] [get_bd_pins rst_clk_50M/dcm_locked] [get_bd_pins rst_clk_wiz_50M/dcm_locked]
  connect_bd_net -net cp0_cause_o [get_bd_pins mycpu_top_0/cp0_cause_o] [get_bd_pins system_ila_0/probe4]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets cp0_cause_o]
  connect_bd_net -net cp0_epc_o [get_bd_pins mycpu_top_0/cp0_epc_o] [get_bd_pins system_ila_0/probe6]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets cp0_epc_o]
  connect_bd_net -net cp0_status_o [get_bd_pins mycpu_top_0/cp0_status_o] [get_bd_pins system_ila_0/probe5]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets cp0_status_o]
  connect_bd_net -net debug_wb_pc [get_bd_pins mycpu_top_0/debug_wb_pc] [get_bd_pins system_ila_0/probe0]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets debug_wb_pc]
  connect_bd_net -net debug_wb_rf_data [get_bd_pins mycpu_top_0/debug_wb_rf_data] [get_bd_pins system_ila_0/probe3]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets debug_wb_rf_data]
  connect_bd_net -net debug_wb_rf_wen [get_bd_pins mycpu_top_0/debug_wb_rf_wen] [get_bd_pins system_ila_0/probe2]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets debug_wb_rf_wen]
  connect_bd_net -net debug_wb_rf_wnum [get_bd_pins mycpu_top_0/debug_wb_rf_wnum] [get_bd_pins system_ila_0/probe1]
set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_nets debug_wb_rf_wnum]
  connect_bd_net -net eth_conf_0_eth_spi_mosi [get_bd_ports eth_spi_mosi] [get_bd_pins eth_conf_0/eth_spi_mosi]
  connect_bd_net -net eth_conf_0_eth_spi_sck [get_bd_ports eth_spi_sck] [get_bd_pins eth_conf_0/eth_spi_sck]
  connect_bd_net -net eth_conf_0_eth_spi_ss_n [get_bd_ports eth_spi_ss_n] [get_bd_pins eth_conf_0/eth_spi_ss_n]
  connect_bd_net -net eth_spi_miso_0_1 [get_bd_ports eth_spi_miso] [get_bd_pins eth_conf_0/eth_spi_miso]
  connect_bd_net -net reset_1 [get_bd_ports reset] [get_bd_pins rst_clk_50M/ext_reset_in]
  connect_bd_net -net rst_clk_50M_interconnect_aresetn [get_bd_pins axi_mem_intercon/ARESETN] [get_bd_pins rst_clk_50M/interconnect_aresetn]
  connect_bd_net -net rst_clk_50M_mb_reset [get_bd_pins rst_clk_50M/mb_reset] [get_bd_pins util_vector_logic_0/Op1]
  connect_bd_net -net rst_clk_50M_peripheral_aresetn [get_bd_ports eth_rst_n] [get_bd_pins axi_bram_ctrl_0/s_axi_aresetn] [get_bd_pins axi_bram_ctrl_1/s_axi_aresetn] [get_bd_pins axi_bram_ctrl_2/s_axi_aresetn] [get_bd_pins axi_dma_0/axi_resetn] [get_bd_pins axi_emc_base/s_axi_aresetn] [get_bd_pins axi_emc_ext/s_axi_aresetn] [get_bd_pins axi_emc_flash/s_axi_aresetn] [get_bd_pins axi_gpio_0/s_axi_aresetn] [get_bd_pins axi_mem_intercon/M00_ARESETN] [get_bd_pins axi_mem_intercon/M01_ARESETN] [get_bd_pins axi_mem_intercon/M02_ARESETN] [get_bd_pins axi_mem_intercon/M03_ARESETN] [get_bd_pins axi_mem_intercon/M04_ARESETN] [get_bd_pins axi_mem_intercon/M05_ARESETN] [get_bd_pins axi_mem_intercon/M06_ARESETN] [get_bd_pins axi_mem_intercon/M07_ARESETN] [get_bd_pins axi_mem_intercon/M08_ARESETN] [get_bd_pins axi_mem_intercon/S00_ARESETN] [get_bd_pins axi_mem_intercon/S01_ARESETN] [get_bd_pins axi_mem_intercon/S02_ARESETN] [get_bd_pins axi_mem_intercon/S03_ARESETN] [get_bd_pins axi_mem_intercon/S04_ARESETN] [get_bd_pins axi_timer_0/s_axi_aresetn] [get_bd_pins axi_uartlite_0/s_axi_aresetn] [get_bd_pins eth_conf_0/rst_in_n] [get_bd_pins jtag_axi_0/aresetn] [get_bd_pins rst_clk_50M/peripheral_aresetn] [get_bd_pins system_ila_0/resetn]
  connect_bd_net -net rst_clk_wiz_50M_peripheral_aresetn [get_bd_pins axi_mem_intercon/M09_ARESETN] [get_bd_pins rst_clk_wiz_50M/peripheral_aresetn]
  connect_bd_net -net util_vector_logic_0_Res [get_bd_pins mycpu_top_0/aresetn] [get_bd_pins util_vector_logic_0/Res]
  connect_bd_net -net vio_0_probe_out0 [get_bd_pins rst_clk_50M/aux_reset_in] [get_bd_pins vio_0/probe_out0]
  connect_bd_net -net xlconstant_0_dout [get_bd_pins clk_wiz/reset] [get_bd_pins rst_clk_wiz_50M/ext_reset_in] [get_bd_pins xlconstant_0/dout]

  # Create address segments
  create_bd_addr_seg -range 0x00002000 -offset 0x1FC00000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FC00000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FC00000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x07FFE000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_bram_ctrl_1/S_AXI/Mem0] SEG_axi_bram_ctrl_1_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x07FFE000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_bram_ctrl_1/S_AXI/Mem0] SEG_axi_bram_ctrl_1_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x07FFE000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_bram_ctrl_1/S_AXI/Mem0] SEG_axi_bram_ctrl_1_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FB00000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_bram_ctrl_2/S_AXI/Mem0] SEG_axi_bram_ctrl_2_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FB00000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_bram_ctrl_2/S_AXI/Mem0] SEG_axi_bram_ctrl_2_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FB00000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_bram_ctrl_2/S_AXI/Mem0] SEG_axi_bram_ctrl_2_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00000000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_emc_base/S_AXI_MEM/Mem0] SEG_axi_emc_base_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00000000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_emc_base/S_AXI_MEM/Mem0] SEG_axi_emc_base_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00000000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_emc_base/S_AXI_MEM/Mem0] SEG_axi_emc_base_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00400000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_emc_ext/S_AXI_MEM/Mem0] SEG_axi_emc_ext_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00400000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_emc_ext/S_AXI_MEM/Mem0] SEG_axi_emc_ext_Mem0
  create_bd_addr_seg -range 0x00400000 -offset 0x00400000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_emc_ext/S_AXI_MEM/Mem0] SEG_axi_emc_ext_Mem0
  create_bd_addr_seg -range 0x00800000 -offset 0x00800000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_emc_flash/S_AXI_MEM/Mem0] SEG_axi_emc_flash_Mem0
  create_bd_addr_seg -range 0x00800000 -offset 0x00800000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_emc_flash/S_AXI_MEM/Mem0] SEG_axi_emc_flash_Mem0
  create_bd_addr_seg -range 0x00800000 -offset 0x00800000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_emc_flash/S_AXI_MEM/Mem0] SEG_axi_emc_flash_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FC00000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x07FFE000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_bram_ctrl_1/S_AXI/Mem0] SEG_axi_bram_ctrl_1_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FB00000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_bram_ctrl_2/S_AXI/Mem0] SEG_axi_bram_ctrl_2_Mem0
  create_bd_addr_seg -range 0x00010000 -offset 0x1FF00000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_dma_0/S_AXI_LITE/Reg] SEG_axi_dma_0_Reg
  create_bd_addr_seg -range 0x00400000 -offset 0x00000000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_emc_base/S_AXI_MEM/Mem0] SEG_axi_emc_0_Mem0
  create_bd_addr_seg -range 0x00800000 -offset 0x00800000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_emc_flash/S_AXI_MEM/Mem0] SEG_axi_emc_0_Mem01
  create_bd_addr_seg -range 0x00400000 -offset 0x00400000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_emc_ext/S_AXI_MEM/Mem0] SEG_axi_emc_ext_Mem0
  create_bd_addr_seg -range 0x00010000 -offset 0x1FA00000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x1F900000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x1FD00000 [get_bd_addr_spaces jtag_axi_0/Data] [get_bd_addr_segs axi_uartlite_0/S_AXI/Reg] SEG_axi_uartlite_0_Reg
  create_bd_addr_seg -range 0x00002000 -offset 0x1FC00000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x07FFE000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_bram_ctrl_1/S_AXI/Mem0] SEG_axi_bram_ctrl_1_Mem0
  create_bd_addr_seg -range 0x00002000 -offset 0x1FB00000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_bram_ctrl_2/S_AXI/Mem0] SEG_axi_bram_ctrl_2_Mem0
  create_bd_addr_seg -range 0x00010000 -offset 0x1FF00000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_dma_0/S_AXI_LITE/Reg] SEG_axi_dma_0_Reg
  create_bd_addr_seg -range 0x00400000 -offset 0x00000000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_emc_base/S_AXI_MEM/Mem0] SEG_axi_emc_0_Mem0
  create_bd_addr_seg -range 0x00800000 -offset 0x00800000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_emc_flash/S_AXI_MEM/Mem0] SEG_axi_emc_0_Mem03
  create_bd_addr_seg -range 0x00400000 -offset 0x00400000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_emc_ext/S_AXI_MEM/Mem0] SEG_axi_emc_ext_Mem0
  create_bd_addr_seg -range 0x00010000 -offset 0x1FA00000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x1F900000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x1FD00000 [get_bd_addr_spaces mycpu_top_0/interface_aximm] [get_bd_addr_segs axi_uartlite_0/S_AXI/Reg] SEG_axi_uartlite_0_Reg

  # Exclude Address Segments
  create_bd_addr_seg -range 0x00010000 -offset 0x1FA00000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_MM2S/SEG_axi_gpio_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1F900000 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_MM2S/SEG_axi_timer_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1FA00000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_S2MM/SEG_axi_gpio_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1F900000 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_S2MM/SEG_axi_timer_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1FF00000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_dma_0/S_AXI_LITE/Reg] SEG_axi_dma_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_SG/SEG_axi_dma_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1FA00000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_SG/SEG_axi_gpio_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1F900000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_SG/SEG_axi_timer_0_Reg]

  create_bd_addr_seg -range 0x00010000 -offset 0x1FD00000 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs axi_uartlite_0/S_AXI/Reg] SEG_axi_uartlite_0_Reg
  exclude_bd_addr_seg [get_bd_addr_segs axi_dma_0/Data_SG/SEG_axi_uartlite_0_Reg]



  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


