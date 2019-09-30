// PCPMMV_metric.cc: PCPMMV Metric Plugin
// Author: Eric Flumerfelt
// Last Modified: 3/22/2019
//
// An implementation of the MetricPlugin interface for PCPMMV

#ifndef __PCPMMV_METRIC__
#define __PCPMMV_METRIC__ 1

#include "TRACE/tracemf.h"  // order matters -- trace.h (no "mf") is nested from MetricMacros.hh
#define TRACE_NAME (app_name_ + "_pcpmmv_metric").c_str()

#include "artdaq-utilities/Plugins/MetricMacros.hh"
#include "fhiclcpp/fwd.h"

#include <pcp/pmapi.h>

#include <pcp/mmv_stats.h>

/**
 * \brief The artdaq namespace
 */
namespace artdaq {
/**
 * \brief An instance of the MetricPlugin class that sends metric data to PCPMMV
 *
 * pmlogger must be configured to log the artdaq metrics so that the web display will retrieve them.
 * Run artdaq, and ensure that the metrics are now available through `pminfo -f mmv`. Then,
 * run (as root) `cd /var/lib/pcp/pmlogger;pmlogconf -r config.default` and restart pmlogger.
 */
class PCPMMVMetric : public MetricPlugin
{
private:
	std::unordered_map<std::string, int> registered_metric_types_;
	std::vector<mmv_metric_t> registered_metrics_;

	void* mmvAddr_;
	int domain_;

	size_t initial_metric_collection_time_;
	std::chrono::steady_clock::time_point metric_start_time_;

	void init_mmv()
	{
		if (registered_metrics_.size() > 0)
		{
			mmv_stats_flags_t flags{};
			METLOG(TLVL_INFO) << "Going to initialize mmv metric with name " << normalize_name_(app_name_) << ", metric count " << registered_metrics_.size();
			METLOG(TLVL_INFO) << "First metric name: " << registered_metrics_[0].name << ", type " << registered_metrics_[0].type << ", item " << registered_metrics_[0].item;
			mmvAddr_ = mmv_stats_init(normalize_name_(app_name_).c_str(), domain_, flags, &registered_metrics_[0], registered_metrics_.size(), 0, 0);
		}
	}

	void stop_mmv()
	{
		if (mmvAddr_)
		{
			mmv_stats_stop(normalize_name_(app_name_).c_str(), mmvAddr_);
			mmvAddr_ = nullptr;
		}
	}

	std::string normalize_name_(std::string name)
	{
		auto nameTemp(name);

		auto pos = nameTemp.find('%');
		while (pos != std::string::npos)
		{
			nameTemp = nameTemp.replace(pos, 1, "Percent");
			pos = nameTemp.find('%');
		}
		std::replace(nameTemp.begin(), nameTemp.end(), ' ', '_');

		if (nameTemp.size() > MMV_NAMEMAX - 1)
		{
			nameTemp = nameTemp.substr(0, MMV_NAMEMAX - 1);
		}
		return nameTemp;
	}

	bool check_time_()
	{
		auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - metric_start_time_).count();
		METLOG(TLVL_INFO) << "Duration since start: " << dur << " seconds. (initial = " << initial_metric_collection_time_ << " seconds)";
		if (dur < 0) return false;
		return static_cast<size_t>(dur) > initial_metric_collection_time_;
	}

	pmUnits infer_units_(std::string unitString)
	{
		pmUnits output;
		output.scaleCount = PM_COUNT_ONE;
		output.scaleSpace = PM_SPACE_BYTE;
		output.scaleTime = PM_TIME_SEC;

		std::transform(unitString.begin(), unitString.end(), unitString.begin(), ::tolower);
		std::string before = unitString;
		std::string after = "";

		if (auto pos = unitString.find('/') != std::string::npos)
		{
			before = unitString.substr(0, pos - 1);
			after = unitString.substr(pos);
		}
		std::istringstream iss(before);
		std::vector<std::string> before_tokens{std::istream_iterator<std::string>{iss},
		                                       std::istream_iterator<std::string>{}};

		iss.str(after);
		std::vector<std::string> after_tokens{std::istream_iterator<std::string>{iss},
		                                      std::istream_iterator<std::string>{}};

		for (auto token : before_tokens)
		{
			if (token == "") continue;
			if (token == "s" || token.find("sec") == 0)
			{
				output.dimTime++;
			}
			else if (token == "b" || token.find("byte") == 0)
			{
				output.dimSpace++;
			}
			else
			{
				output.dimCount++;
			}
		}

		for (auto token : after_tokens)
		{
			if (token == "") continue;
			if (token == "s" || token.find("sec") == 0)
			{
				output.dimTime--;
			}
			else if (token == "b" || token.find("byte") == 0)
			{
				output.dimSpace--;
			}
			else
			{
				output.dimCount--;
			}
		}

		return output;
	}

public:
	/**
   * \brief Construct an instance of the PCPMMV metric
   * \param pset Parameter set with which to configure the MetricPlugin. 
   * \param app_name Name of the application sending metrics
   *
   * pcp_domain_number can be used to change the domain parameter
   * seconds_before_init determines how long the metric will wait, collecting metric names before starting to log metrics (to reduce the number of stop/init cycles)
   */
	explicit PCPMMVMetric(fhicl::ParameterSet const& pset, std::string const& app_name, std::string const& metric_name)
	    : MetricPlugin(pset, app_name, metric_name)
	    , registered_metric_types_()
	    , registered_metrics_()
	    , mmvAddr_(nullptr)
	    , domain_(pset.get<int>("pcp_domain_number", 0))
	    , initial_metric_collection_time_(pset.get<size_t>("seconds_before_init", 30))
	{}

	virtual ~PCPMMVMetric() { MetricPlugin::stopMetrics(); }

	/**
   * \brief Gets the unique library name of this plugin
   * \return The library name of this plugin, "PCPMMV".
   */
	std::string getLibName() const override { return "pcpmmv"; }

	/**
   * \brief PCPMMV does not need any specific action on stop
   */
	void stopMetrics_() override {}

	/**
   * \brief PCPMMV does not need any specific action on start
   */
	void startMetrics_() override { metric_start_time_ = std::chrono::steady_clock::now(); }

	/**
   * \brief Send a string metric to PCPMMV
   * \param name Name of the metric
   * \param value Value of the metric
   * \param unit Units of the metric
   */
	void sendMetric_(const std::string& name, const std::string& value, const std::string& unit) override
	{
		auto nname = normalize_name_(name);
		if (!registered_metric_types_.count(nname))
		{
			METLOG(TLVL_INFO) << "Adding string metric named " << nname;
			mmv_metric_t newMetric;
			strcpy(newMetric.name, nname.c_str());
			newMetric.item = registered_metrics_.size();
			newMetric.type = MMV_TYPE_STRING;
			newMetric.semantics = MMV_SEM_INSTANT;
			newMetric.dimension = infer_units_(unit);
			newMetric.indom = 0;
			newMetric.helptext = 0;
			newMetric.shorttext = 0;

			registered_metrics_.push_back(newMetric);
			registered_metric_types_[nname] = MMV_TYPE_STRING;
			stop_mmv();
		}

		if (registered_metric_types_[nname] != MMV_TYPE_STRING)
		{
			METLOG(TLVL_ERROR) << "PCP-MMV Metric: Metric instance has wrong type! Expected " << registered_metric_types_[nname]
			                   << ", got std::string";
			return;
		}

		if (!mmvAddr_ && check_time_())
		{
			init_mmv();
		}

		if (mmvAddr_)
		{
			auto base = mmv_lookup_value_desc(mmvAddr_, nname.c_str(), 0);
			auto val = value;
			if (val.size() > MMV_STRINGMAX - 1)
			{
				val = val.substr(0, MMV_STRINGMAX - 1);
			}

			mmv_set_string(mmvAddr_, base, value.c_str(), value.size());
		}
	}

	/**
   * \brief Send a integer metric to PCPMMV
   * \param name Name of the metric
   * \param value Value of the metric
   * \param unit Units of the metric
   */
	void sendMetric_(const std::string& name, const int& value, const std::string& unit) override
	{
		auto nname = normalize_name_(name);
		if (!registered_metric_types_.count(nname))
		{
			METLOG(TLVL_INFO) << "Adding int metric named " << nname;
			mmv_metric_t newMetric;
			strcpy(newMetric.name, nname.c_str());
			newMetric.item = registered_metrics_.size();
			newMetric.type = MMV_TYPE_I64;
			newMetric.semantics = MMV_SEM_INSTANT;
			newMetric.dimension = infer_units_(unit);
			newMetric.indom = 0;
			newMetric.helptext = 0;
			newMetric.shorttext = 0;

			registered_metrics_.push_back(newMetric);
			registered_metric_types_[nname] = MMV_TYPE_I64;
			stop_mmv();
		}

		if (registered_metric_types_[nname] != MMV_TYPE_I64)
		{
			METLOG(TLVL_ERROR) << "PCP-MMV Metric: Metric instance has wrong type! Expected " << registered_metric_types_[nname]
			                   << ", got int";
			return;
		}

		if (!mmvAddr_ && check_time_())
		{
			init_mmv();
		}

		if (mmvAddr_)
		{
			auto base = mmv_lookup_value_desc(mmvAddr_, nname.c_str(), 0);
			mmv_set_value(mmvAddr_, base, value);
		}
	}

	/**
   * \brief Send a double metric to PCPMMV
   * \param name Name of the metric
   * \param value Value of the metric
   * \param unit Units of the metric
   */
	void sendMetric_(const std::string& name, const double& value, const std::string& unit) override
	{
		auto nname = normalize_name_(name);
		if (!registered_metric_types_.count(nname))
		{
			METLOG(TLVL_INFO) << "Adding double metric named " << nname;
			mmv_metric_t newMetric;
			strcpy(newMetric.name, nname.c_str());
			newMetric.item = registered_metrics_.size();
			newMetric.type = MMV_TYPE_DOUBLE;
			newMetric.semantics = MMV_SEM_INSTANT;
			newMetric.dimension = infer_units_(unit);
			newMetric.indom = 0;
			newMetric.helptext = 0;
			newMetric.shorttext = 0;

			registered_metrics_.push_back(newMetric);
			registered_metric_types_[nname] = MMV_TYPE_DOUBLE;
			stop_mmv();
		}

		if (registered_metric_types_[nname] != MMV_TYPE_DOUBLE)
		{
			METLOG(TLVL_ERROR) << "PCP-MMV Metric: Metric instance has wrong type! Expected " << registered_metric_types_[nname]
			                   << ", got double";
			return;
		}

		if (!mmvAddr_ && check_time_())
		{
			init_mmv();
		}

		if (mmvAddr_)
		{
			auto base = mmv_lookup_value_desc(mmvAddr_, nname.c_str(), 0);
			mmv_set_value(mmvAddr_, base, value);
		}
	}

	/**
   * \brief Send a float metric to PCPMMV
   * \param name Name of the metric
   * \param value Value of the metric
   * \param unit Units of the metric
   */
	void sendMetric_(const std::string& name, const float& value, const std::string& unit) override
	{
		auto nname = normalize_name_(name);
		if (!registered_metric_types_.count(nname))
		{
			METLOG(TLVL_INFO) << "Adding float metric named " << nname;
			mmv_metric_t newMetric;
			strcpy(newMetric.name, nname.c_str());
			newMetric.item = registered_metrics_.size();
			newMetric.type = MMV_TYPE_FLOAT;
			newMetric.semantics = MMV_SEM_INSTANT;
			newMetric.dimension = infer_units_(unit);
			newMetric.indom = 0;
			newMetric.helptext = 0;
			newMetric.shorttext = 0;

			registered_metrics_.push_back(newMetric);
			registered_metric_types_[nname] = MMV_TYPE_FLOAT;
			stop_mmv();
		}

		if (registered_metric_types_[nname] != MMV_TYPE_FLOAT)
		{
			METLOG(TLVL_ERROR) << "PCP-MMV Metric: Metric instance has wrong type! Expected " << registered_metric_types_[nname]
			                   << ", got float";
			return;
		}

		if (!mmvAddr_ && check_time_())
		{
			init_mmv();
		}

		if (mmvAddr_)
		{
			auto base = mmv_lookup_value_desc(mmvAddr_, nname.c_str(), 0);
			mmv_set_value(mmvAddr_, base, value);
		}
	}

	/**
   * \brief Send an unsigned long metric to PCPMMV
   * \param name Name of the metric
   * \param value Value of the metric
   * \param unit Units of the metric
   */
	void sendMetric_(const std::string& name, const unsigned long int& value, const std::string& unit) override
	{
		auto nname = normalize_name_(name);
		if (!registered_metric_types_.count(nname))
		{
			METLOG(TLVL_INFO) << "Adding unsigned metric named " << nname;
			mmv_metric_t newMetric;
			strcpy(newMetric.name, nname.c_str());
			newMetric.item = registered_metrics_.size();
			newMetric.type = MMV_TYPE_U64;
			newMetric.semantics = MMV_SEM_INSTANT;
			newMetric.dimension = infer_units_(unit);
			newMetric.indom = 0;
			newMetric.helptext = 0;
			newMetric.shorttext = 0;

			registered_metrics_.push_back(newMetric);
			registered_metric_types_[nname] = MMV_TYPE_U64;
			stop_mmv();
		}

		if (registered_metric_types_[nname] != MMV_TYPE_U64)
		{
			METLOG(TLVL_ERROR) << "PCP-MMV Metric: Metric instance has wrong type! Expected " << registered_metric_types_[nname]
			                   << ", got unsigned int";
			return;
		}

		if (!mmvAddr_ && check_time_())
		{
			init_mmv();
		}

		if (mmvAddr_)
		{
			auto base = mmv_lookup_value_desc(mmvAddr_, nname.c_str(), 0);
			mmv_set_value(mmvAddr_, base, value);
		}
	}
};
}  // End namespace artdaq

DEFINE_ARTDAQ_METRIC(artdaq::PCPMMVMetric)

#endif  // End ifndef __PCPMMV_METRIC__
