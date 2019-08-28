# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "ENABLE_CHECKER" -parent ${Page_0}
  ipgui::add_param $IPINST -name "ENABLE_TLB" -parent ${Page_0}


}

proc update_PARAM_VALUE.ENABLE_CHECKER { PARAM_VALUE.ENABLE_CHECKER } {
	# Procedure called to update ENABLE_CHECKER when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.ENABLE_CHECKER { PARAM_VALUE.ENABLE_CHECKER } {
	# Procedure called to validate ENABLE_CHECKER
	return true
}

proc update_PARAM_VALUE.ENABLE_TLB { PARAM_VALUE.ENABLE_TLB } {
	# Procedure called to update ENABLE_TLB when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.ENABLE_TLB { PARAM_VALUE.ENABLE_TLB } {
	# Procedure called to validate ENABLE_TLB
	return true
}


proc update_MODELPARAM_VALUE.ENABLE_TLB { MODELPARAM_VALUE.ENABLE_TLB PARAM_VALUE.ENABLE_TLB } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.ENABLE_TLB}] ${MODELPARAM_VALUE.ENABLE_TLB}
}

proc update_MODELPARAM_VALUE.ENABLE_CHECKER { MODELPARAM_VALUE.ENABLE_CHECKER PARAM_VALUE.ENABLE_CHECKER } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.ENABLE_CHECKER}] ${MODELPARAM_VALUE.ENABLE_CHECKER}
}

