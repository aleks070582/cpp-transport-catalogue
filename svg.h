#pragma once
#define _USE_MATH_DEFINES 
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <type_traits>
#include <optional>
#include<variant>
namespace svg {
    struct Rgb {
        Rgb() :red(0), green(0), blue(0) {};
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) :red(red_), green(green_), blue(blue_) {};
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    struct Rgba {
        Rgba() :red(0), green(0), blue(0), opacity(1) {};
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_) :red(red_), green(green_), blue(blue_),
            opacity(opacity_) {
        };
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        double opacity;
    };

    using Color = std::variant< std::monostate, std::string, Rgb, Rgba>;
    inline const Color none_color{ std::monostate{} };
    inline const Color NoneColor{ std::monostate{} };


    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND
    };

    constexpr const char* strokelinecaps[] = {
        "butt",
        "round",
        "square",
    };

    constexpr const char* strokelinejoins[] = {
        "arcs",
        "bevel",
        "miter",
        "miter-clip",
        "round"
    };

    template<typename T, typename std::enable_if<
        std::is_same<T, StrokeLineCap>::value || std::is_same<T, StrokeLineJoin>::value, int>::type = 0>
    std::ostream & operator<<(std::ostream & out, const T & value) {
        const char* string = nullptr;
        if constexpr (std::is_same<T, StrokeLineCap>::value) {
            string = strokelinecaps[static_cast<size_t>(value)];
        }
        if constexpr (std::is_same<T, StrokeLineJoin>::value) {
            string = strokelinejoins[static_cast<size_t>(value)];
        }
        out << string;
        return out;
    }



    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap cap) {
            line_cap_ = cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);
        ~Polyline() = default;

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        ~Text() = default;
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point base_point_ = { 0.0, 0.0 };
        Point offset_point_{ 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        ObjectContainer() = default;
        virtual ~ObjectContainer() = default;
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;
        ~Document() = default;
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;
    };
}  // namespace svg

namespace shapes {
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);
    std::vector<svg::Circle> CreateSnowMan(svg::Point pos, double rad);

    class Triangle final : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3) : p1_(p1), p2_(p2), p3_(p3) {}

        void Draw(svg::ObjectContainer& cont) const override {
            cont.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
        }

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star final : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
            : center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad), num_rays_(num_rays) {
        }

        void Draw(svg::ObjectContainer& cont) const override {
            cont.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_).SetFillColor("red")    // Заливка: red
                .SetStrokeColor("black")); // Контур: black

        }

    private:
        svg::Point center_;
        double outer_rad_;
        double inner_rad_;
        int num_rays_;
    };

    class Snowman final : public svg::Drawable {
    public:
        Snowman(svg::Point pos, double rad) : position_(pos), radius_(rad) {}
        void Draw(svg::ObjectContainer& cont) const override;

    private:
        svg::Point position_;
        double radius_;
    };
}