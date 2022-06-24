
include(MetricPlugins)

cet_build_plugin(pcpmmv artdaq::metric LIBRARIES REG pcp_mmv TRACE::MF)

install_headers()
install_source()
install_fhicl()
