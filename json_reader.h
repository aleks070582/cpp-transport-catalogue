#pragma once
#include"json.h"
#include"json_builder.h"
#include"transport_catalogue.h"
#include"geo.h"
#include"svg.h"
#include"map_renderer.h"
#include<stdexcept>
#include<cassert>
#include<string>
#include<algorithm>
#include<utility>
#include<sstream>
#include"domain.h"
#include"graph.h"
#include"transport_router.h"

using StopInfo = const std::set<const transport_catalogue::Bus*, transport_catalogue::BusPointerComparator>*;
struct StatRequest {
	StatRequest() :id(0),type({}), name({}) {};
	int id;
	std::string type;
	std::string name;
	std::string from;
	std::string to;
};

struct RoutingSettings {
	int bus_wait_time;
	double velocity;
};
class JsonReader {
public:

	JsonReader(const json::Document& document,transport_catalogue::TransportCatalogue&catalog,graph::DirectedWeightedGraph<double>&graph);
	json::Document AnswerToJson();
private:
	JsonReader() = delete;
	json::Node SvgToJson( int id);
	svg::Color ParseColorFromJson(const json::Node& node);
	json::Node RouteToJson(std::optional<std::pair<double, std::vector<transport_router::EdgeInfo>>>value,int id);
	void ParsingSettings();
	json::Node BusInfoToJson(const std::optional<transport_catalogue::BusInfo>& bus_info, int id);
	json::Node StopInfoToJson(const std::optional< StopInfo>& stop, int id);
	void ParseStatJsonDocument();
private:
	
	
	transport_catalogue::TransportCatalogue&catalog_;
	const json::Document& document_;
	graph::DirectedWeightedGraph<double>& graph_;
	
	std::vector<StatRequest> stat_requests_;
	RenderSettings render_set_;
	RoutingSettings routings_settings_;
	std::optional<transport_router::TransportRouter> graph_creator_;
};

