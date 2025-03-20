#include "svg.h"

namespace svg {

    using namespace std::literals;
    std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        switch (color.index()) {
        case 1: {
            out << std::get<std::string>(color);
            break;
        }
        case 0: {
            out << "none";
            break;
        }
        case 2: {
            const auto& rgb = std::get<Rgb>(color);
            out << "rgb(" << static_cast<int>(rgb.red) << "," << static_cast<int>(rgb.green) <<
                "," << static_cast<int>(rgb.blue) << ")";
            break;
        }
        case 3: {
            const auto& rgba = std::get<Rgba>(color);
            out << "rgba(" << static_cast<int>(rgba.red) << "," << static_cast<int>(rgba.green) << "," <<
                static_cast<int>(rgba.blue) << "," << rgba.opacity << ")";
            break;
        }
        default: {
            out << "неизвестное значение";
            break;
        }
        }
        return out;
    }
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            out << points_[0].x << "," << points_[0].y;
            for (size_t i = 1; i < points_.size(); ++i) {
                out << " " << points_[i].x << "," << points_[i].y;
            }
        }
        out << "\"";
        RenderAttrs(context.out);
        out << " />";
    }

    std::string EscapeXML(const std::string& str) {
        std::string result;
        for (char ch : str) {
            switch (ch) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '\'': result += "&apos;"; break;
            case '"': result += "&quot;"; break;
            default: result += ch; break;
            }
        }
        return result;
    }

    Text& Text::SetPosition(Point pos) {
        base_point_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_point_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        out << " x=\"" << base_point_.x << "\" y=\"" << base_point_.y << "\"";
        out << " dx=\"" << offset_point_.x << "\" dy=\"" << offset_point_.y << "\"";
        out << " font-size=\"" << size_ << "\"";

        if (!font_family_.empty()) {
            out << " font-family=\"" << EscapeXML(font_family_) << "\"";
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\"" << EscapeXML(font_weight_) << "\"";
        }

        RenderAttrs(context.out);

        out << ">";
        out << EscapeXML(data_);
        out << "</text>";
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
        RenderContext context(out, 2);
        for (const auto& object : objects_) {
            object->Render(context);
        }
        out << "</svg>\n";
    }


}  // namespace svg

namespace shapes {
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }

    std::vector<svg::Circle> CreateSnowMan(svg::Point pos, double rad) {
        using namespace svg;

        Circle first, second, third;
        first.SetCenter({ pos.x,pos.y + 5 * rad });
        first.SetRadius(rad * 2);
        second.SetCenter({ pos.x,pos.y + 2 * rad });
        second.SetRadius(rad * 1.5);
        third.SetCenter(pos);
        third.SetRadius(rad);

        return { first, second, third };
    }

    void Snowman::Draw(svg::ObjectContainer& cont) const {
        std::vector<svg::Circle> temp = CreateSnowMan(position_, radius_);
        for (auto& circle : temp) {
            cont.Add(circle
                .SetFillColor("rgb(240,240,240)")  // Заливка: rgb(240,240,240)
                .SetStrokeColor("black"));           // Контур: black
            // Ширина обводки
        }
    }
}
