
basic_plugin(pcpmmv "metric"
  artdaq_utilities::artdaq-utilities_Plugins
  pcp_mmv
  ${FHICLCPP}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  ${MF_MESSAGEFACILITY}
)

install_headers()
install_source()
install_fhicl()
