#pragma once
#include"json.h"
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

extern struct RenderSettings render_settings;
struct StatRequest {
	StatRequest() :id(0),type({}), name({}) {};
	int id;
	std::string type;
	std::string name;
};

using StopInfo = const std::set<const transport_catalogue::Bus*,transport_catalogue::BusPointerComparator>*;
json::Node SvgToJson(const json::Document& document, const transport_catalogue::TransportCatalogue& catalog,int id);
svg::Color ParseColorFromJson(const json::Node& node);
RenderSettings ParsingSettings(const json::Document& document);
json::Node BusInfoToJson(const std::optional<transport_catalogue::BusInfo>& bus_info, int id);
json::Node StopInfoToJson(const std::optional< StopInfo>& stop, int id);
std::vector<StatRequest> ParseStatJsonDocument(const json::Document& document);
void ParseAndPrintStatJsonDocument(const json::Document& document, const transport_catalogue:: TransportCatalogue& catalog,std::ostream& out);
void ParseRequestJsonDocument(const json::Document& document,transport_catalogue::TransportCatalogue& catalog);