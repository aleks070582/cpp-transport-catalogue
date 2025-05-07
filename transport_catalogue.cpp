#include "transport_catalogue.h"
#include<iostream>

namespace transport_catalogue {
    using namespace std;

    optional<const set<const Bus*, BusPointerComparator>*> TransportCatalogue::GetStopInfo(const string_view name) const {
        auto p_stop = p_to_stop.find(name);
        if (p_stop == p_to_stop.end()) {
            return nullopt;
        }

        if (!buses_at_stop.contains(name))
        {

            return   &nullbus;
        }
      
        return &(buses_at_stop.at(name));
    }

    optional<BusInfo> TransportCatalogue::GetBusInfo(const string_view name) const {
        auto p_bus = p_to_bus.find(name);
        if (p_bus == p_to_bus.end()) {
            return nullopt;
        }

        BusInfo result{};
        result.stops = static_cast<unsigned int>(p_bus->second->stops.size());
        result.u_stops = static_cast<unsigned int>(unordered_set(p_bus->second->stops.begin(), p_bus->second->stops.end()).size());
        result.lenght = CalculatePath(p_bus->second);
        result.curvature = double(result.lenght) / CalculateGeographicPath(p_bus->second);
        return result;
    }

    void TransportCatalogue::AddStop(const string& name, geo::Coordinates coord)
    {
        if (!p_to_stop.contains(name)) {
            stops.push_back(Stop(name, coord));
            Stop* p_stop = &stops.back();
            p_to_stop[p_stop->name] = p_stop;
            buses_at_stop.insert({ p_stop->name, {} });
        }
        else
        {
            p_to_stop[name]->coord = coord;
        }
    }

    void TransportCatalogue::AddBus(const string& name, const vector<string_view>&& route,bool type) {
        buses.push_back(Bus(name));
        auto p_bus = &buses.back();
        p_bus->is_round_trip = type;
        p_to_bus[p_bus->name] = p_bus;

        for (const auto& stop_name : route) {
            auto p_stop = p_to_stop.at(stop_name);
            p_bus->stops.push_back(p_stop);
            buses_at_stop[p_stop->name].insert(p_bus);
        }
    }

    void TransportCatalogue::AddStopAndDistance(const string& first_stop, const string& second_stop, int distance)
    {

        if (!p_to_stop.contains(first_stop))
        {
            stops.push_back(Stop{ first_stop,{} });
            p_to_stop[first_stop] = &stops.back();
        }
        if (p_to_stop.contains(second_stop))
        {
            distance_between_stops.insert({ {p_to_stop[first_stop], p_to_stop[second_stop]}, distance });
        }
        else {
            stops.push_back(second_stop);
            auto pointer_to_stop = &(stops.back());
            p_to_stop[pointer_to_stop->name] = pointer_to_stop;
            distance_between_stops.insert({ { p_to_stop[first_stop], pointer_to_stop}, distance });
        }

    }

    geo::Coordinates TransportCatalogue::GetStopCoordinates(const std::string_view stop) const
    {
        if (!p_to_stop.contains(stop)) {
            throw logic_error("stop is not exist");
        }
        return p_to_stop.at(stop)->coord;
    }

    set< string_view> TransportCatalogue::GetAllBusesName() const {
        set< string_view> result;
        for (auto& bus : buses) {
            if (p_to_bus.at(bus.name)->stops.empty()) {
                continue;
            }
            result.insert(bus.name);
        }
        return result;
    }

    const std::vector<const Stop*>& TransportCatalogue::GetStops(std::string_view bus) const
    {
        return p_to_bus.at(bus)->stops;
    }

      deque<Stop> TransportCatalogue::GetAllStop() const
    {
        return stops;
    }

      vector<string_view> TransportCatalogue::GetFinalStops(string_view bus_name) const
      {
          if (!p_to_bus.contains(bus_name)) {
              return vector<string_view>{};
          }
          auto& bus = p_to_bus.at(bus_name);
          if (bus->is_round_trip == true) {
              return { { bus->stops.at(0)->name } };
          }
          else if (bus->stops.at(0)->name == bus->stops.at((bus->stops.size() + 1) / 2 - 1)->name) {
              return { { bus->stops.at(0)->name } };
          }
          return { {bus->stops.at(0)->name},{bus->stops.at((bus->stops.size() + 1) / 2-1)->name }};
      }

      vector<const Stop*> TransportCatalogue::GetAllStopWithBus() const
      {
          vector<const Stop*> result{};
          result.reserve(p_to_stop.size());
          for (const auto& stop : p_to_stop) {
              string_view name = stop.first;
              if (buses_at_stop.contains(name)&&!buses_at_stop.at(name).empty()) {
                  result.push_back(stop.second);
              }
              }
          return result;
      }

      int TransportCatalogue::GetDistanceBetweenNearestStops(std::string_view first, std::string_view second) const
      {
          const Stop* f = p_to_stop.at(first);
          const Stop* s = p_to_stop.at(second);
          int result = distance_between_stops.contains({ f,s }) ? distance_between_stops.at({ f,s }) :
              distance_between_stops.at({ s,f });
          return result;
      }

      const std::deque<Bus>& TransportCatalogue::GetAllBusRef() const
      {
          return buses;
      }

      int TransportCatalogue::CalculatePath(std::string_view name) const
      {
          Bus* p_bus = p_to_bus.at(name);

          return CalculatePath(p_bus);
      }

      const std::deque<Stop>& TransportCatalogue::GetAllStopRef() const
      {
          return stops;
      }

     
    double TransportCatalogue::CalculateGeographicPath(const Bus* p_bus) const {
        if (p_bus->stops.size() < 2) {
            return 0;
        }

        double result = 0;
        for (size_t i = 0; i < p_bus->stops.size() - 1; ++i) {
            auto first_coord = p_bus->stops[i]->coord;
            auto second_coord = p_bus->stops[i + 1]->coord;
            result += geo::ComputeDistance(first_coord, second_coord);
        }
        return result;
    }

    int TransportCatalogue::CalculatePath(const Bus* p_bus) const {
        if (p_bus->stops.size() < 2) {
            return 0;
        }

        int result = 0;
        for (size_t i = 0; i < p_bus->stops.size() - 1; ++i) {
            const Stop* first_stop = p_bus->stops[i];
            const Stop* second_stop = p_bus->stops[i + 1];
            if (distance_between_stops.contains({ first_stop, second_stop })) {
                result += distance_between_stops.at({ first_stop, second_stop });
            }
            else if (distance_between_stops.contains({ second_stop, first_stop })) {
                result += distance_between_stops.at({ second_stop, first_stop });
            }
        }
        return result;
    }
    
    
/*
    void GraphCreater::CreateAllVertexId() {
        size_t id = 0;
        vertexes_id.reserve(catalog_.stops.size() * 2);
        for (const Stop& stop : catalog_.stops) {
            vertexes_id.push_back(id);       // IN
            vertexes_id.push_back(id + 1);   // OUT
            vertex_by_id[{id, id + 1}] = stop.name;  // in → out
            id_by_vertex[stop.name] = { id, id + 1 };
            id += 2;
        }
    }


    void GraphCreater::CreateTranferEdges() {
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
           // if(stop_name=="Sheremetyevo"||stop_name=="Khimki"){
           //        cout << "wait " << id << " stop " << stop_name << " weight " << wait_time_ << endl;
            //}
       
        }
    }

    pair<double,int> GraphCreater::GetWeightBetweenStops(size_t first_id, size_t second_id, const Bus& bus) {
        int distance = 0;
        int count = 0;
        for (size_t i = first_id; i < second_id; ++i) {
            distance += catalog_.GetDistanceBetweenNearestStops(bus.stops.at(i)->name, bus.stops.at(i + 1)->name);
            ++count;
        }
        constexpr const double koefficient = 0.06;
        return { (static_cast<double>(distance) / velocity_) * koefficient,count };
    }

    void GraphCreater::CreateBusesRouteEdges() {
        for (const Bus& bus : catalog_.buses) {
            if (!bus.is_round_trip) {
                const size_t reverse_stop = bus.stops.size() / 2 ;

                // Прямое направление
                for (size_t i = 0; i < reverse_stop; ++i) {
                    double total_weight = 0.0;
                    int total_span = 0;
                    for (size_t ii = i + 1; ii <= reverse_stop; ++ii) {
                        total_weight += static_cast<double>(
                            catalog_.GetDistanceBetweenNearestStops(bus.stops[ii - 1]->name, bus.stops[ii]->name)
                            ) / velocity_ * 0.06;
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
                   //    if (bus.name == "2k") {
                            cout << "bus.name " << bus.name << " f_stop " << first_stop_name << " s_stop " << second_stop_name << " weight " << edge_info.weight << endl;
                      //  }
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
                            ) / velocity_ * 0.06;
                        ++total_span;

                   //     const auto& first_stop_name = bus.stops.at(i)->name;
                   //     const auto& second_stop_name = bus.stops.at(ii)->name;
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
             //           if (bus.name == "2k"&& ed=="Sheremetyevo"||f) {
                           cout << "bus.name " << bus.name << " f_stop " << first_stop_name << " s_stop " << second_stop_name << " weight " << edge_info.weight << endl;
              //          }
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
                            ) / velocity_ * 0.06;
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
                //        if (bus.name == "2k") {
                           cout << "bus.name " << bus.name << " f_stop " << first_stop_name << " s_stop " << second_stop_name << " weight " << edge_info.weight << endl;
                 //       }
                    }
                }

                // Замыкающее кольцо
                const std::string_view first_stop_name = bus.stops.at(0)->name;
                const auto first_vertex_id = id_by_vertex.at(first_stop_name);
                graph::Edge<double> edge;
                edge.from = first_vertex_id.second;
                edge.to = first_vertex_id.first;
                edge.weight = catalog_.CalculatePath(catalog_.p_to_bus.at(bus.name));
                edge.weight = (edge.weight / velocity_) * 0.06;
                size_t edge_id = graph_.AddEdge(edge);

                EdgeInfo edge_info;
                edge_info.count = bus.stops.size();
                edge_info.first_stop = first_stop_name;
                edge_info.second_stop = first_stop_name;
                edge_info.bus = bus.name;
                edge_info.weight = edge.weight;
                edge_info_by_id[edge_id] = edge_info;
            //    if (bus.name == "2k") {
                   cout << "bus.name " << bus.name << " f_stop " << first_stop_name << " s_stop " << edge_info.second_stop << " weight " << edge_info.weight << endl;
             //   }
            }
        }
    }
    void GraphCreater::AddCatalogToGraph() {
      //  cout << "vertex" << endl;
        CreateAllVertexId();
     //   cout << "waitedges" << endl;
        CreateTranferEdges();
     //   cout << "buses_roue" << endl;
        CreateBusesRouteEdges();
     //   cout << "create_route" << endl;
        router_.emplace(graph_); 
    }

    std::optional<std::pair<double, std::vector<EdgeInfo>>> GraphCreater::FindRoute(std::string first, std::string second)
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

        return make_pair(info.weight , r_v);
    }

   
   */
}
