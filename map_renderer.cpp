#include "map_renderer.h"

using namespace transport_catalogue;
std::vector<geo::Coordinates> MapRendering::GetRouteByPoint( std::string_view bus_name, const SphereProjector::SphereProjector&projector)
{
	const auto& stops = catalog_.GetStops(bus_name);
	std::vector<geo::Coordinates> stops_coord;
	stops_coord.reserve(stops.size());
	for (const auto& stop : stops) {
		svg::Point temp = projector(stop->coord);
		geo::Coordinates stop_temp;
		stop_temp.lng = temp.y;
		stop_temp.lat = temp.x;
		stops_coord.push_back(stop_temp);
	}
	return stops_coord;
}

std::vector<svg::Text> MapRendering::DrawBusText(std::string_view bus_name, const SphereProjector::SphereProjector& projector) {

	const std::vector<std::string_view> stops_name = catalog_.GetFinalStops(bus_name);
	std::vector<svg::Text> text;
	for (short int i = 0; i < static_cast<short int>(stops_name.size()); ++i) {
		svg::Point coord = projector(catalog_.GetStopCoordinates(stops_name.at(i)));
		svg::Text temp;
		temp.SetData(std::string(bus_name));
		temp.SetPosition(coord);
		text.push_back(temp);
		text.push_back(temp);
	}
	return text;
}



std::vector<svg::Text> MapRendering::DrawStopName(const Stop* stop, const SphereProjector::SphereProjector& proj) {
	std::vector<svg::Text> texts(2);
	std::for_each(texts.begin(), texts.end(),[this, &proj,stop](svg::Text& text){
		text.SetData(stop->name);
		text.SetPosition(proj(stop->coord));
		text.SetOffset({ render_set_.stop_label_offset.first,render_set_.stop_label_offset.second });
		text.SetFontSize(render_set_.stop_label_font_size);
		text.SetFontFamily("Verdana");
	});
	
	svg::Text* text = &texts.at(0);
	text->SetFillColor(render_set_.underlayer_color);
	text->SetStrokeColor(render_set_.underlayer_color);
	text->SetStrokeWidth(render_set_.underlayer_width);
	text->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	text->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text = &texts.at(1);
	text->SetFillColor("black");
	return texts;
}
svg::Circle MapRendering::DrawStop(const Stop* stop, const SphereProjector::SphereProjector&proj) {
	svg::Circle circle;
	circle.SetCenter(proj(stop->coord));
	circle.SetRadius(render_set_.stop_radius);
	circle.SetFillColor("white");
	return circle;
}

void MapRendering::ApplyRenderSettingsToBusRoute(std::vector<svg::Text>& text,int number ) {
	
	std::for_each(text.begin(), text.end(), [this, number](svg::Text& t) {
		t.SetOffset({ render_set_.bus_label_offset.first,render_set_.bus_label_offset.second });
		t.SetFontSize(render_set_.bus_label_font_size);
		t.SetFontFamily("Verdana");
		t.SetFontWeight("bold");
		});
		
	for (short int i = 1; i <=static_cast<short int>(text.size()); ++i) {
		if ((i%2)) {
			auto& r_text = text.at(i-1);
			r_text.SetFillColor(render_set_.underlayer_color);
			r_text.SetStrokeColor(render_set_.underlayer_color);
			r_text.SetStrokeWidth(render_set_.underlayer_width);
			r_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			r_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		else {
			text.at(i-1).SetFillColor(render_set_.color_palette.at(number % render_set_.color_palette.size()));
		}

	}
	return;
}

svg::Polyline MapRendering::BusRouteToPolyline(std::vector<geo::Coordinates>& points,int number)
{
	svg::Polyline polyline;
	for (const auto& coord : points) {
		polyline.AddPoint({ coord.lat,coord.lng });
	}
	polyline.SetStrokeWidth(render_set_.line_width);
	polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	polyline.SetFillColor(svg::none_color);
	polyline.SetStrokeColor(render_set_.color_palette.at(number % render_set_.color_palette.size()));
	return polyline;
}

MapRendering::MapRendering(const RenderSettings& render_settings, const transport_catalogue::TransportCatalogue& catalog) :
	catalog_(catalog), render_set_(render_settings)
{

	auto all_stop = catalog.GetAllStopWithBus();
	SphereProjector::SphereProjector projector(all_stop, render_set_.width,
		render_set_.height, render_set_.padding);
	std::set<std::string_view> buses = catalog.GetAllBusesName();
	assert(!all_stop.empty());
	int number = 0;
	for (const auto& bus : buses) {
		std::vector<geo::Coordinates> coordinates = GetRouteByPoint(bus, projector);
		svg::Polyline polyline = BusRouteToPolyline(coordinates, number);
		map_.Add(polyline);
		++number;
	}
	number = 0;
	for (const auto& bus : buses) {
		auto text = DrawBusText(bus, projector);
		ApplyRenderSettingsToBusRoute(text, number);
		for (short int i = 0; i < static_cast<short int>(text.size()); ++i) {
			map_.Add(text.at(i));
		}
		++number;
	}
	std::sort(all_stop.begin(), all_stop.end(), [](const Stop* lhs, const Stop* rhs) {
		return lhs->name < rhs->name;
		});
	for (const auto& stop : all_stop) {
		assert(stop != nullptr);
		svg::Circle temp = DrawStop(stop, projector);
		map_.Add(temp);
	}
	for (const auto& stop : all_stop) {
		assert(stop != nullptr);
		std::vector<svg::Text> temp = DrawStopName(stop, projector);
		map_.Add(temp.at(0));
		map_.Add(temp.at(1));

	}
}

svg::Document& MapRendering::GetMap()
{
	return map_;
}
