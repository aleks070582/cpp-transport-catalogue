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


struct StatRequest {
	StatRequest() :id(0),type({}), name({}) {};
	int id;
	std::string type;
	std::string name;
};
struct RenderSettings {
	RenderSettings() :width(0.0), height(0.0), padding(0.0), line_width(0.0), stop_radius(0.0), bus_label_font_size(0),
		bus_label_offset({ 0.0,0.0 }), stop_label_font_size(0), stop_label_offset({ 0.0,0.0 }),
		underlayer_color(std::monostate{}), underlayer_width(0.0), color_palette({}) {
	};
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	int bus_label_font_size;
	std::pair<double,double> bus_label_offset;
	int stop_label_font_size;
	std::pair<double, double> stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	std::vector<svg::Color> color_palette;

};
using StopInfo = const std::set<const transport_catalogue::Bus*,transport_catalogue::BusPointerComparator>*;
json::Node SvgToJson(const json::Document& document, const transport_catalogue::TransportCatalogue& catalog,int id);
svg::Color ParseColorFromJson(const json::Node& node);
RenderSettings ParsingJsonAndGetSetting(const json::Document& document);
json::Node BusInfoToJson(const std::optional<transport_catalogue::BusInfo>& bus_info, int id);
json::Node StopInfoToJson(const std::optional< StopInfo>& stop, int id);
std::vector<StatRequest> ParseStatJsonDocument(const json::Document& document);
void ParseAndPrintStatJsonDocument(const json::Document& document, const transport_catalogue:: TransportCatalogue& catalog,std::ostream& out);
void ParseRequestJsonDocument(const json::Document& document,transport_catalogue::TransportCatalogue& catalog);