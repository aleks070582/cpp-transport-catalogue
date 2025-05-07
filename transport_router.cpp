#include "transport_router.h"

namespace transport_router {

    using namespace std;
    using namespace transport_catalogue;


void TransportRouter::CreateAllVertexId() {
    size_t id = 0;
   const deque<Stop>&stops = catalog_.GetAllStopRef();
    vertexes_id.reserve(stops.size() * 2);
    for (const  tc::Stop& stop : stops) {
        vertexes_id.push_back(id);       // IN
        vertexes_id.push_back(id + 1);   // OUT
        vertex_by_id[{id, id + 1}] = stop.name;  // in → out
        id_by_vertex[stop.name] = { id, id + 1 };
        id += 2;
    }
}


void TransportRouter::CreateTranferEdges() {
    for (const auto& [ids, stop_name] : vertex_by_id) {
        graph::Edge<double> edge;
        edge.from = ids.first;
        edge.to = ids.second;
        edge.weight = wait_time_;
        size_t id = graph_.AddEdge(edge);
        EdgeInfo edge_info;
        edge_info.bus = "";
        edge_info.first_stop = stop_name;
        edge_info.second_stop = stop_name;
        edge_info.weight = wait_time_;
        edge_info_by_id[id] = edge_info;
        
    }
}

void TransportRouter::CreateBusesRouteEdges() {
  const  deque<transport_catalogue::Bus>& buses = catalog_.GetAllBusRef();
    for (const tc::Bus& bus : buses) {
        if (!bus.is_round_trip) {
            const size_t reverse_stop = bus.stops.size() / 2;

            // Прямое направление
            for (size_t i = 0; i < reverse_stop; ++i) {
                double total_weight = 0.0;
                int total_span = 0;
                for (size_t ii = i + 1; ii <= reverse_stop; ++ii) {
                    total_weight += static_cast<double>(
                        catalog_.GetDistanceBetweenNearestStops(bus.stops[ii - 1]->name, bus.stops[ii]->name)
                        ) / velocity_ * koef;
                    ++total_span;

                    const auto& first_stop_name = bus.stops.at(i)->name;
                    const auto& second_stop_name = bus.stops.at(ii)->name;
                    const auto& first_vertex_id = id_by_vertex.at(first_stop_name);
                    const auto& second_vertex_id = id_by_vertex.at(second_stop_name);

                    graph::Edge<double> edge;
                    edge.from = first_vertex_id.second;
                    edge.to = second_vertex_id.first;
                    edge.weight = total_weight;

                    size_t edge_id = graph_.AddEdge(edge);

                    EdgeInfo edge_info;
                    edge_info.first_stop = first_stop_name;
                    edge_info.second_stop = second_stop_name;
                    edge_info.bus = bus.name;
                    edge_info.weight = total_weight;
                    edge_info.count = total_span;
                    edge_info_by_id[edge_id] = edge_info;
                   
                }
            }

            // Обратное направление
            for (size_t i = reverse_stop; i < bus.stops.size() - 1; ++i) {
                double total_weight = 0.0;
                int total_span = 0;
                for (size_t ii = i + 1; ii < bus.stops.size(); ++ii) {
                    const auto& first_stop_name = bus.stops.at(i)->name;
                    const auto& second_stop_name = bus.stops.at(ii)->name;
                    total_weight += static_cast<double>(
                        catalog_.GetDistanceBetweenNearestStops(bus.stops[ii - 1]->name, bus.stops[ii]->name)
                        ) / velocity_ * koef;
                    ++total_span;

                    const auto& first_vertex_id = id_by_vertex.at(first_stop_name);
                    const auto& second_vertex_id = id_by_vertex.at(second_stop_name);

                    graph::Edge<double> edge;
                    edge.from = first_vertex_id.second;
                    edge.to = second_vertex_id.first;
                    edge.weight = total_weight;

                    size_t edge_id = graph_.AddEdge(edge);

                    EdgeInfo edge_info;
                    edge_info.first_stop = first_stop_name;
                    edge_info.second_stop = second_stop_name;
                    edge_info.bus = bus.name;
                    edge_info.weight = total_weight;
                    edge_info.count = total_span;
                    edge_info_by_id[edge_id] = edge_info;
                   
                }
            }

        }
        else {
            // Кольцевой маршрут
            for (size_t i = 0; i < bus.stops.size() - 1; ++i) {
                double total_weight = 0.0;
                int total_span = 0;
                for (size_t ii = i + 1; ii < bus.stops.size(); ++ii) {
                    total_weight += static_cast<double>(
                        catalog_.GetDistanceBetweenNearestStops(bus.stops[ii - 1]->name, bus.stops[ii]->name)
                        ) / velocity_ * koef;
                    ++total_span;

                    const auto& first_stop_name = bus.stops.at(i)->name;
                    const auto& second_stop_name = bus.stops.at(ii)->name;
                    const auto& first_vertex_id = id_by_vertex.at(first_stop_name);
                    const auto& second_vertex_id = id_by_vertex.at(second_stop_name);

                    graph::Edge<double> edge;
                    edge.from = first_vertex_id.second;
                    edge.to = second_vertex_id.first;
                    edge.weight = total_weight;

                    size_t edge_id = graph_.AddEdge(edge);

                    EdgeInfo edge_info;
                    edge_info.first_stop = first_stop_name;
                    edge_info.second_stop = second_stop_name;
                    edge_info.bus = bus.name;
                    edge_info.weight = total_weight;
                    edge_info.count = total_span;
                    edge_info_by_id[edge_id] = edge_info;
                 
                }
            }

            // Замыкающее кольцо
            const std::string_view first_stop_name = bus.stops.at(0)->name;
            const auto first_vertex_id = id_by_vertex.at(first_stop_name);
            graph::Edge<double> edge;
            edge.from = first_vertex_id.second;
            edge.to = first_vertex_id.first;
            edge.weight = catalog_.CalculatePath(bus.name);
            edge.weight = (edge.weight / velocity_) * koef;
            size_t edge_id = graph_.AddEdge(edge);

            EdgeInfo edge_info;
            edge_info.count = bus.stops.size();
            edge_info.first_stop = first_stop_name;
            edge_info.second_stop = first_stop_name;
            edge_info.bus = bus.name;
            edge_info.weight = edge.weight;
            edge_info_by_id[edge_id] = edge_info;
        
        }
    }
}
void TransportRouter::AddCatalogToGraph() {
    CreateAllVertexId();
    CreateTranferEdges();
    CreateBusesRouteEdges();
    router_.emplace(graph_);
}

TransportRouter::TransportRouter(const tc::TransportCatalogue& catalog, double velocity, int wait_time) : 
    catalog_(catalog), wait_time_(wait_time), velocity_(velocity)
{
    AddCatalogToGraph();
}

std::optional<Route> TransportRouter::FindRoute(std::string first, std::string second)
{
    if (!router_) {
        throw std::logic_error("Router has not been initialized");
    }

    size_t first_id = id_by_vertex.at(first).first;
    size_t second_id = id_by_vertex.at(second).first;
    auto opt_info = router_->BuildRoute(first_id, second_id);

    if (!opt_info) {
        return std::nullopt;
    }

    const auto& info = *opt_info;
    std::vector<EdgeInfo> r_v;
    for (const graph::EdgeId id : info.edges) {
        r_v.push_back(edge_info_by_id.at(id));
    }

    erase_if(r_v, [](EdgeInfo& value) {
        return value.weight == 0.0;
        });

    return Route(info.weight, r_v );
}



}

