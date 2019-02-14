//PCPMMV_metric.cc: PCPMMV Metric Plugin
// Author: Eric Flumerfelt
// Last Modified: 11/14/2014
//
// An implementation of the MetricPlugin interface for PCPMMV

#ifndef __PCPMMV_METRIC__
#define __PCPMMV_METRIC__ 1

#include "fhiclcpp/fwd.h"
#include "artdaq-utilities/Plugins/MetricMacros.hh"

/**
 * \brief The artdaq namespace
 */
namespace artdaq
{
	/**
	 * \brief An instance of the MetricPlugin class that sends metric data to PCPMMV
	 */
	class PCPMMVMetric : public MetricPlugin
	{
	private:

	public:
		/**
		 * \brief Construct an instance of the PCPMMV metric
		 * \param pset Parameter set with which to configure the MetricPlugin. Also includes "configFile" (path to gmond.conf), "group" (Group for metrics) and "cluster" (Cluster tag to use (optional)).
		 * \param app_name Name of the application sending metrics
		 */
		explicit PCPMMVMetric(fhicl::ParameterSet const& pset, std::string const& app_name) : MetricPlugin(pset, app_name)
		{
		}

		virtual ~PCPMMVMetric()
		{
			MetricPlugin::stopMetrics();
		}

		/**
		* \brief Gets the unique library name of this plugin
		* \return The library name of this plugin, "PCPMMV".
		*/
		std::string getLibName() const override { return "pcpmmv"; }

		/**
		 * \brief PCPMMV does not need any specific action on stop
		 */
		void stopMetrics_() override { }

		/**
		 * \brief PCPMMV does not need any specific action on start
		 */
		void startMetrics_() override { }

		/**
		 * \brief Send a string metric to PCPMMV
		 * \param name Name of the metric
		 * \param value Value of the metric
		 * \param unit Units of the metric
		 */
		void sendMetric_(const std::string& , const std::string& , const std::string& ) override
		{
		}

		/**
		* \brief Send a integer metric to PCPMMV (truncated to int32)
		* \param name Name of the metric
		* \param value Value of the metric
		* \param unit Units of the metric
		*/
		void sendMetric_(const std::string& , const int& , const std::string& ) override
		{
		}

		/**
		* \brief Send a double metric to PCPMMV
		* \param name Name of the metric
		* \param value Value of the metric
		* \param unit Units of the metric
		*/
		void sendMetric_(const std::string& , const double& , const std::string& ) override
		{
		}

		/**
		* \brief Send a float metric to PCPMMV
		* \param name Name of the metric
		* \param value Value of the metric
		* \param unit Units of the metric
		*/
		void sendMetric_(const std::string& , const float& , const std::string& ) override
		{
		}

		/**
		* \brief Send an unsigned long metric to PCPMMV (truncated to uint32)
		* \param name Name of the metric
		* \param value Value of the metric
		* \param unit Units of the metric
		*/
		void sendMetric_(const std::string& , const unsigned long int& , const std::string& ) override
		{
		}
	};
} //End namespace artdaq

DEFINE_ARTDAQ_METRIC(artdaq::PCPMMVMetric)

#endif //End ifndef __PCPMMV_METRIC__
