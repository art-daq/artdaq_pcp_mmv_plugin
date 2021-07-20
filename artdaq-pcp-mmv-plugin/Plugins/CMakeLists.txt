
basic_plugin(pcpmmv "metric"
  artdaq_utilities::artdaq-utilities_Plugins
  pcp_mmv
  fhiclcpp
  cetlib
  cetlib_except
  ${MF_MESSAGEFACILITY}
)

install_headers()
install_source()
install_fhicl()
