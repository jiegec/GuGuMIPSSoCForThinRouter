# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "N_CONFIG_ENTRY" -parent ${Page_0}


}

proc update_PARAM_VALUE.N_CONFIG_ENTRY { PARAM_VALUE.N_CONFIG_ENTRY } {
	# Procedure called to update N_CONFIG_ENTRY when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.N_CONFIG_ENTRY { PARAM_VALUE.N_CONFIG_ENTRY } {
	# Procedure called to validate N_CONFIG_ENTRY
	return true
}


proc update_MODELPARAM_VALUE.N_CONFIG_ENTRY { MODELPARAM_VALUE.N_CONFIG_ENTRY PARAM_VALUE.N_CONFIG_ENTRY } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.N_CONFIG_ENTRY}] ${MODELPARAM_VALUE.N_CONFIG_ENTRY}
}

