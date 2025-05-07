#pragma once
#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <functional>
#include <algorithm>
#include <utility>
#include "geo.h"
#include"graph.h"
#include <unordered_set>
#include <optional>
#include <set>
#include<stdexcept>
#include<variant>
#include"router.h"
#include"transport_catalogue.h"

namespace transport_router {
    namespace tc = transport_catalogue;
    struct SizeTPairHash {
        size_t operator()(const std::pair<size_t, size_t>& p) const {
            size_t h1 = std::hash<size_t>{}(p.first);
            size_t h2 = std::hash<size_t>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };
    struct EdgeInfo {
        std::string_view first_stop;
        std::string_view second_stop;
        double weight = 0;
        std::string_view bus;
        int count = 0;
    };

    class GraphCreater {

        graph::DirectedWeightedGraph<double>& graph_;
        const tc::TransportCatalogue& catalog_;
        const int wait_time_;
        const double velocity_;
        std::unordered_map<std::string_view, std::pair<size_t, size_t>> id_by_vertex;
        std::unordered_map<std::pair<size_t, size_t>, std::string_view, SizeTPairHash> vertex_by_id;
        std::vector<size_t> vertexes_id;
        std::unordered_map<size_t, EdgeInfo> edge_info_by_id;
        void CreateAllVertexId();
        void CreateTranferEdges();
        void CreateBusesRouteEdges();
    //    std::pair<double, int> GetWeightBetweenStops(size_t firs_id, size_t second_id, const tc:: Bus& bus);
        std::optional<graph::Router<double>> router_;
    public:
        GraphCreater(graph::DirectedWeightedGraph<double>& gr, const tc::TransportCatalogue& catalog,
            double velosity, int wait_time) :graph_(gr), catalog_(catalog), wait_time_(wait_time),
            velocity_(velosity) { };
        void AddCatalogToGraph();
        std::optional<std::pair<double, std::vector<EdgeInfo>>> FindRoute(std::string first, std::string second);
    };
}