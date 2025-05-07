#include "json_reader.h"
using namespace transport_catalogue;
using namespace json;

JsonReader::JsonReader(const Document& document,transport_catalogue::TransportCatalogue&catalog,graph::DirectedWeightedGraph<double>&grap) : catalog_(catalog), document_(document),graph_(grap) {
	assert(document_.GetRoot().IsMap());
	const Node requests = document_.GetRoot().AsMap().find("base_requests")->second;
	assert(requests.IsArray());
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto& request_value = request.AsMap();
		if (request_value.at("type").AsString() != "Stop") {
			continue;
		}
		const std::string name = request_value.at("name").AsString();
		catalog_.AddStop(name, { request_value.at("latitude").AsDouble(),
				request_value.at("longitude").AsDouble() });
		if (const auto distances_to_stops = request_value.at("road_distances").AsMap(); !distances_to_stops.empty()) {
			for (const auto& distance_to_stop : distances_to_stops) {
				assert(distance_to_stop.second.IsInt());
				catalog_.AddStopAndDistance(name, distance_to_stop.first, distance_to_stop.second.AsInt());
			}
		}
	}
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto& request_value = request.AsMap();
		if (request_value.at("type").AsString() != "Bus") {
			continue;
		}
		const std::string name = request_value.at("name").AsString();
		const bool is_roundtrip = request_value.at("is_roundtrip").AsBool();
		std::vector<std::string_view> stops;
		const auto& stops_array = request_value.at("stops").AsArray();
		assert(request_value.at("stops").IsArray());
		for (const auto& stop : stops_array) {
			stops.emplace_back(stop.AsString());
		}
		stops.reserve(stops.size() * 2);
		if (!is_roundtrip) {
			for (auto it = std::next(stops.rbegin()); it != stops.rend(); it = std::next(it)) {
				stops.push_back(*it);
			}
		}

		catalog_.AddBus(name, move(stops), is_roundtrip);
	}
	ParsingSettings();
	ParseStatJsonDocument();
	
}

void JsonReader::ParseStatJsonDocument() {
	assert(document_.GetRoot().IsMap());
	const Node requests = document_.GetRoot().AsMap().find("stat_requests")->second;
	assert(requests.IsArray());
	for (const auto& request : requests.AsArray()) {
		assert(request.IsMap());
		const auto &request_as_map = request.AsMap();
		StatRequest stat_request;
		stat_request.id = request_as_map.at("id").AsInt();
		stat_request.name = (request_as_map.find("name") != request_as_map.end()) ? request_as_map.at("name").AsString() : "";
		stat_request.type = request_as_map.at("type").AsString();
		if (stat_request.type == "Route") {
			stat_request.from = request_as_map.at("from").AsString();
			stat_request.to = request_as_map.at("to").AsString();
		}
		else {
			stat_request.from = "";
			stat_request.to = "";
		}
		stat_requests_.emplace_back(std::move(stat_request));
	}
	
}

svg::Color JsonReader::ParseColorFromJson(const Node& node)
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

json::Node JsonReader::RouteToJson(
	std::optional<std::pair<double, std::vector<transport_router::EdgeInfo>>> opt,
	int id)
{
	if (!opt.has_value()) {
		return json::Builder{}
			.StartDict()
			.Key("request_id").Value(id)
			.Key("error_message").Value("not found")
			.EndDict()
			.Build();
	}

	const auto& edges = opt->second;
	json::Builder result;
	result.StartDict()
		.Key("request_id").Value(id)
		.Key("total_time").Value(opt.value().first)
		.Key("items").StartArray();

	for (size_t i = 0; i < edges.size(); ++i) {
		const auto& e = edges[i];
		if (e.bus == "") {
			result.StartDict()
				.Key("type").Value("Wait")
				.Key("stop_name").Value(std::string(e.first_stop))
				.Key("time").Value(e.weight)
				.EndDict();
		}

		else {

			result
				.StartDict()
				.Key("type").Value("Bus")
				.Key("bus").Value(std::string(e.bus))
				.Key("span_count").Value(e.count)
				.Key("time").Value(e.weight)
				.EndDict();
		}

		
	}
	result.EndArray().EndDict();
	return result.Build();
}
void JsonReader::ParsingSettings(){
	assert(document_.GetRoot().IsMap());
	assert(document_.GetRoot().AsMap().contains("render_settings"));
	
	const Dict dict=document_.GetRoot().AsMap().find("render_settings")->second.AsMap();
	render_set_.width = dict.find("width")->second.AsDouble();
	render_set_.height = dict.find("height")->second.AsDouble();
	render_set_.bus_label_font_size = dict.find("bus_label_font_size")->second.AsInt();
	const Array* temp = &dict.find("bus_label_offset")->second.AsArray();
	render_set_.bus_label_offset = { temp->at(0).AsDouble(),temp->at(1).AsDouble() };
	render_set_.line_width = dict.find("line_width")->second.AsDouble();
	render_set_.padding = dict.find("padding")->second.AsDouble();
	render_set_.stop_label_font_size = dict.find("stop_label_font_size")->second.AsInt();
	render_set_.stop_radius = dict.find("stop_radius")->second.AsDouble();
	temp = &dict.find("stop_label_offset")->second.AsArray();
	render_set_.stop_label_offset = { temp->at(0).AsDouble(),temp->at(1).AsDouble() };
	render_set_.underlayer_width = dict.find("underlayer_width")->second.AsDouble();
	render_set_.underlayer_color = ParseColorFromJson(dict.find("underlayer_color")->second);
	
	temp = &dict.find("color_palette")->second.AsArray();
	for (const auto& color : *temp) {
		render_set_.color_palette.push_back(ParseColorFromJson(color));
	}

	if (document_.GetRoot().AsMap().contains("routing_settings")) {
		Dict render_seetings = document_.GetRoot().AsMap().find("routing_settings")->second.AsMap();
		RoutingSettings rc;
		rc.velocity = render_seetings.at("bus_velocity").AsDouble();
		rc.bus_wait_time = render_seetings.at("bus_wait_time").AsInt();
		routings_settings_ = rc;
		graph_creator_.emplace(graph_, catalog_, rc.velocity, rc.bus_wait_time);
		graph_creator_->AddCatalogToGraph();
	}


}

Node JsonReader::BusInfoToJson( const std::optional<BusInfo>& bus_info,int id) {
	using namespace std::literals::string_literals;
	
	json::Builder result{};
	if (!bus_info) {
		result.StartDict().Key("request_id").Value(id).
			Key("error_message").Value("not found");
			
	}
	else {
		const auto& value = bus_info.value();
		result.StartDict().Key("curvature").Value(value.curvature)
			.Key("request_id").Value(id)
			.Key("route_length").Value(value.lenght)
			.Key("stop_count").Value(static_cast<int>(value.stops))
			.Key("unique_stop_count").Value(static_cast<int>(value.u_stops));
			
	}
	result.EndDict();
	return result.Build();
}

Node JsonReader::StopInfoToJson(const std::optional<StopInfo>& stop, int id) {
	using namespace std::literals::string_literals;
	
	json::Builder result{};
	if (!stop) {
		result.StartDict().Key("request_id").Value(id)
			.Key("error_message").Value("not found");
	}
	else {
		result.StartDict().Key("request_id").Value(id)
			.Key("buses").StartArray();
		const auto& stop_value = *stop.value();
		
		for (const auto& bus : stop_value) {
			result.Value(bus->name);
		}
		result.EndArray();
	}
	result.EndDict();
	return result.Build();
}


json::Document JsonReader::AnswerToJson() {
	
	assert(!stat_requests_.empty());
	json::Builder result{};
	result.StartArray();
	
	for (const auto &request : stat_requests_) {
		if (request.type == "Bus") {
			const std::optional<BusInfo> answer = catalog_.GetBusInfo(request.name);
			result.Value(BusInfoToJson(answer, request.id).GetValue());
		
		}
		else if(request.type=="Stop"){
			const std::optional<StopInfo> answer = catalog_.GetStopInfo(request.name);
			result.Value(StopInfoToJson(answer, request.id).GetValue());
			
		}
		else if(request.type=="Map"){
			result.Value(SvgToJson(request.id).GetValue());
			
		}
		else { //request.type=="Route"
			result.Value(RouteToJson(graph_creator_->FindRoute(request.from, request.to), request.id).GetValue());
		}
	}
	result.EndArray();
	
	return Document(result.Build());
}

Node JsonReader::SvgToJson(int id) {
	std::ostringstream temp;
	MapRendering render(this->render_set_, catalog_);
	svg::Document &map = render.GetMap();
	map.Render(temp);
	json::Builder result{};
	result.StartDict().Key("request_id").Value(id).Key("map").Value(temp.str()).EndDict();

	return result.Build();
}

