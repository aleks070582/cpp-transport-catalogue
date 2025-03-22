#include "json_reader.h"

std::vector<StatRequest> ParseStatJsonDocument(const Document& document) {
	assert(document.GetRoot().IsMap());
	const Node requests = document.GetRoot().AsMap().find("stat_requests")->second;
	assert(requests.IsArray());
	std::vector<StatRequest> stat_requests;
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto &request_as_map = request.AsMap();
		StatRequest stat_request;
		stat_request.id = request_as_map.at("id").AsInt();
		stat_request.name = (request_as_map.find("name") != request_as_map.end()) ? request_as_map.at("name").AsString() : "";
		stat_request.type = request_as_map.at("type").AsString();
		stat_requests.emplace_back(std::move(stat_request));
	}
	return stat_requests;
}

svg::Color ParseColorFromJson(const Node& node)
{
	if (node.IsString()) {
		return svg::Color(node.AsString());
	}
	if (size_t node_size = node.AsArray().size(); node_size < 5) {
		uint8_t c[3];
		for (size_t i = 0; i < 3; ++i) {
			c[i] = static_cast<uint8_t>(node.AsArray().at(i).AsInt());
		}
		if (node_size == 4) {
			double alfa = node.AsArray().at(3).AsDouble();
			return svg::Color(svg::Rgba({ c[0],c[1],c[2],alfa }));
		}
		else {
			return svg::Color(svg::Rgb({ c[0],c[1],c[2] }));
		}
	}
	return svg::Color();
}

RenderSettings ParsingJsonAndGetSetting(const Document& document){
	assert(document.GetRoot().IsMap());
	assert(document.GetRoot().AsMap().contains("render_settings"));
	RenderSettings render_set;
	const Dict dict=document.GetRoot().AsMap().find("render_settings")->second.AsMap();
	render_set.width = dict.find("width")->second.AsDouble();
	render_set.height = dict.find("height")->second.AsDouble();
	render_set.bus_label_font_size = dict.find("bus_label_font_size")->second.AsInt();
	const Array* temp = &dict.find("bus_label_offset")->second.AsArray();
	render_set.bus_label_offset = { temp->at(0).AsDouble(),temp->at(1).AsDouble() };
	render_set.line_width = dict.find("line_width")->second.AsDouble();
	render_set.padding = dict.find("padding")->second.AsDouble();
	render_set.stop_label_font_size = dict.find("stop_label_font_size")->second.AsInt();
	render_set.stop_radius = dict.find("stop_radius")->second.AsDouble();
	temp = &dict.find("stop_label_offset")->second.AsArray();
	render_set.stop_label_offset = { temp->at(0).AsDouble(),temp->at(1).AsDouble() };
	render_set.underlayer_width = dict.find("underlayer_width")->second.AsDouble();
	render_set.underlayer_color = ParseColorFromJson(dict.find("underlayer_color")->second);
	
	temp = &dict.find("color_palette")->second.AsArray();
	for (const auto& color : *temp) {
		render_set.color_palette.push_back(ParseColorFromJson(color));
	}
	return render_set;
}

Node BusInfoToJson( const std::optional<BusInfo>& bus_info,int id) {
	using namespace std::literals::string_literals;
	json::Dict dict;
	if (!bus_info) {
		dict["request_id"s] = id;
		dict["error_message"s] = "not found"s;
	}
	else {
		const auto& value=bus_info.value();
		dict["curvature"s] = value.curvature,
		dict["request_id"s] = id;
		dict["route_length"s] = value.lenght;
		dict["stop_count"s] =static_cast<int>(value.stops);
		dict["unique_stop_count"s] = static_cast<int>(value.u_stops);

	}
	return Node(dict);
}

Node StopInfoToJson(const std::optional<StopInfo>& stop, int id) {
	using namespace std::literals::string_literals;
	Dict dict;
	if (!stop) {
		dict["request_id"s] = id;
		dict["error_message"s] = "not found"s;
	}
	else {
		dict["request_id"s] = id;
		Array buses;
		const auto& stop_value =*stop.value();
		buses.reserve(stop_value.size());
		for (const auto& bus : stop_value) {
			buses.emplace_back(bus->name);
		}
		dict["buses"] = std::move(buses);
	}
	return Node(dict);
}


void ParseAndPrintStatJsonDocument(const json::Document& document, const TransportCatalogue& catalog,std::ostream& out) {
	
	std::vector<StatRequest> requests=ParseStatJsonDocument(document);
	assert(!requests.empty());
	Array array;
	for (const auto &request : requests) {
		if (request.type == "Bus") {
			const std::optional<BusInfo> answer = catalog.GetBusInfo(request.name);
			array.emplace_back(BusInfoToJson( answer,request.id));
		}
		else if(request.type=="Stop"){
			const std::optional<StopInfo> answer = catalog.GetStopInfo(request.name);
			array.emplace_back(StopInfoToJson(answer, request.id));
		}
		else {// request.type=="Map"
			
			array.emplace_back(SvgToJson(document,catalog,request.id));
		}
	}
	Document result_doc((Node(array)));
	json::Print(result_doc, out);
}

Node SvgToJson(const json::Document& document,const transport_catalogue::TransportCatalogue& catalog,int id) {
	const RenderSettings render_set = ParsingJsonAndGetSetting(document);
	std::ostringstream temp;
	DrawAllBusRoute(render_set, catalog,temp);
	Dict dict;
	dict["request_id"] = id;
	dict["map"] = temp.str();
	return Node(dict);
}

void ParseRequestJsonDocument(const Document& document, TransportCatalogue&catalog)
{
	
	assert(document.GetRoot().IsMap());
	const Node requests=document.GetRoot().AsMap().find("base_requests")->second;
	assert(requests.IsArray());
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto &request_value = request.AsMap();
		if (request_value.at("type").AsString() != "Stop") {
			continue;
		}
		const std::string name = request_value.at("name").AsString();
		catalog.AddStop(name, { request_value.at("latitude").AsDouble(),
				request_value.at("longitude").AsDouble() });
		if (const auto distances_to_stops=request_value.at("road_distances").AsMap();!distances_to_stops.empty()){
			for (const auto& distance_to_stop : distances_to_stops)	{
				assert(distance_to_stop.second.IsInt());
				catalog.AddStopAndDistance(name, distance_to_stop.first, distance_to_stop.second.AsInt());
		   }
		}
	}
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto &request_value = request.AsMap();
		if (request_value.at("type").AsString() != "Bus") {
			continue;
		}
		const std::string name = request_value.at("name").AsString();
		const bool is_roundtrip = request_value.at("is_roundtrip").AsBool();
		std::vector<std::string_view> stops;
		const auto &stops_array = request_value.at("stops").AsArray();
		assert(request_value.at("stops").IsArray());
		for (const auto& stop : stops_array) {
			stops.emplace_back(stop.AsString());  
		}
		stops.reserve(stops.size() * 2);
		if (!is_roundtrip) {
			for (auto it = std::next(stops.rbegin()); it != stops.rend(); it=std::next(it)) {
				stops.push_back(*it);  
			}
		}
	
		catalog.AddBus(name,move(stops),is_roundtrip);
	}





}
