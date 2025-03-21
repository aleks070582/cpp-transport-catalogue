#include "map_renderer.h"

std::vector<geo::Coordinates> GetRouteByPoint(const TransportCatalogue& catalog, std::string_view bus_name,
	const SphereProjector::SphereProjector&projector)
{
	const auto& stops = catalog.GetStops(bus_name);
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

std::vector<svg::Text> DrawBusText(std::string_view bus_name, const TransportCatalogue& catalog, 
	const SphereProjector::SphereProjector& projector) {

	const std::vector<std::string_view> stops_name = catalog.GetFinalStops(bus_name);
	std::vector<svg::Text> text;
	for (short int i = 0; i < stops_name.size(); ++i) {
		svg::Point coord = projector(catalog.GetStopCoordinates(stops_name.at(i)));
		svg::Text temp;
		temp.SetData(std::string(bus_name));
		temp.SetPosition(coord);
		text.push_back(temp);
		text.push_back(temp);
	}
	return text;
}

void DrawAllBusRoute(const RenderSettings& render_settings, const TransportCatalogue& catalog, std::ostream& out)
{
	svg::Document map;
	auto all_stop = catalog.GetAllStopWithBus();
	SphereProjector::SphereProjector projector(all_stop, render_settings.width,
		render_settings.height, render_settings.padding);
	std::set<std::string_view> buses = catalog.GetAllBusesName();
	assert(!all_stop.empty());
	int number = 0;
	for (const auto& bus : buses) {
		std::vector<geo::Coordinates> coordinates = GetRouteByPoint(catalog, bus,projector);
		svg::Polyline polyline=BusRouteToPolyline(coordinates,number,render_settings);
		map.Add(polyline);
		++number;
	}
	number = 0;
	for (const auto& bus : buses) {
		auto text=DrawBusText(bus, catalog, projector);
		ApplyRenderSettingsToBusRoute(text,number,render_settings);
		for (short int i = 0;i<text.size(); ++i) {
			map.Add(text.at(i));
		}
		++number;
	}
	std::sort(all_stop.begin(), all_stop.end(), [](const Stop* lhs, const Stop* rhs) {
		return lhs->name < rhs->name;
		});
	for (const auto& stop : all_stop) {
		assert(stop != nullptr);
		svg::Circle temp = DrawStop(stop, render_settings,projector);
		map.Add(temp);
	}
	for (const auto& stop:all_stop) {
		assert(stop != nullptr);
		std::vector<svg::Text> temp = DrawStopName(stop, render_settings, projector);
		map.Add(temp.at(0));
		map.Add(temp.at(1));
		
	}
	map.Render(out);
}

std::vector<svg::Text> DrawStopName(const Stop* stop, const RenderSettings& render_settings, const SphereProjector::SphereProjector& proj) {
	std::vector<svg::Text> texts(2);
	std::for_each(texts.begin(), texts.end(),[&render_settings, &proj,stop](svg::Text& text){
		text.SetData(stop->name);
		text.SetPosition(proj(stop->coord));
		text.SetOffset({ render_settings.stop_label_offset.first,render_settings.stop_label_offset.second });
		text.SetFontSize(render_settings.stop_label_font_size);
		text.SetFontFamily("Verdana");
	});
	svg::Text* text = &texts.at(0);
	text->SetFillColor(render_settings.underlayer_color);
	text->SetStrokeColor(render_settings.underlayer_color);
	text->SetStrokeWidth(render_settings.underlayer_width);
	text->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	text->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text = &texts.at(1);
	text->SetFillColor("black");
	return texts;
}
svg::Circle DrawStop(const Stop* stop, const RenderSettings& render_settings,const SphereProjector::SphereProjector&proj) {
	svg::Circle circle;
	circle.SetCenter(proj(stop->coord));
	circle.SetRadius(render_settings.stop_radius);
	circle.SetFillColor("white");
	return circle;
}

void ApplyRenderSettingsToBusRoute(std::vector<svg::Text>& text,int number,const RenderSettings&render_settings ) {
	std::for_each(text.begin(), text.end(), [&render_settings, number](svg::Text& t) {
		t.SetOffset({ render_settings.bus_label_offset.first,render_settings.bus_label_offset.second });
		t.SetFontSize(render_settings.bus_label_font_size);
		t.SetFontFamily("Verdana");
		t.SetFontWeight("bold");
		});
	for (short int i = 1; i <=text.size(); ++i) {
		if ((i%2)) {
			auto& r_text = text.at(i-1);
			r_text.SetFillColor(render_settings.underlayer_color);
			r_text.SetStrokeColor(render_settings.underlayer_color);
			r_text.SetStrokeWidth(render_settings.underlayer_width);
			r_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			r_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		else {
			text.at(i-1).SetFillColor(render_settings.color_palette.at(number % render_settings.color_palette.size()));
		}

	}
	return;
}

svg::Polyline BusRouteToPolyline(std::vector<geo::Coordinates>& points,int number,const RenderSettings& render_settings)
{
	svg::Polyline polyline;
	for (const auto& coord : points) {
		polyline.AddPoint({ coord.lat,coord.lng });
	}
	polyline.SetStrokeWidth(render_settings.line_width);
	polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	polyline.SetFillColor(svg::none_color);
	polyline.SetStrokeColor(render_settings.color_palette.at(number % render_settings.color_palette.size()));
	return polyline;
}
