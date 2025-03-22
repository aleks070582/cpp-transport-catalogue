#pragma once
#include"transport_catalogue.h"
#include"svg.h"
#include"domain.h"
#include<iostream>
#include"geo.h"
#include"algorithm"
#include<assert.h>
#include<memory>

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
    std::pair<double, double> bus_label_offset;
    int stop_label_font_size;
    std::pair<double, double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;

};
namespace SphereProjector {
    using namespace transport_catalogue;
    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {

    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        SphereProjector(const std::vector<const Stop*>& stops,  double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (stops.begin()==stops.end()) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(stops.begin(),stops.end(),
                [](const Stop* lhs, const Stop* rhs) { 
                    return lhs->coord.lng< rhs->coord.lng; });
            
            min_lon_ = (*left_it)->coord.lng;
            const double max_lon = (*right_it)->coord.lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(stops.begin(),stops.end(),
                [](const Stop* lhs, const Stop* rhs) { return lhs->coord.lat < rhs->coord.lat; });
            const double min_lat = (*bottom_it)->coord.lat;
            max_lat_ = (*top_it)->coord.lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
}

class MapRendering {
public:
    MapRendering(const RenderSettings& render_settings, const transport_catalogue::TransportCatalogue& catalog);
    svg::Document& GetMap();
 private:
     std::vector<geo::Coordinates> GetRouteByPoint(std::string_view bus_name, const SphereProjector::SphereProjector& projector);
     std::vector<svg::Text> DrawBusText(std::string_view bus_name, const  SphereProjector::SphereProjector& projector);
     void ApplyRenderSettingsToBusRoute(std::vector<svg::Text>& text, int number);
     std::vector<svg::Text> DrawStopName(const transport_catalogue::Stop* stop, const SphereProjector::SphereProjector& proj);
     svg::Circle DrawStop(const transport_catalogue::Stop* stop, const SphereProjector::SphereProjector& proj);
     svg::Polyline BusRouteToPolyline(std::vector<geo::Coordinates>& points, int number);
    const transport_catalogue::TransportCatalogue&catalog_;
    const RenderSettings &render_set_;
     svg::Document map_;
};