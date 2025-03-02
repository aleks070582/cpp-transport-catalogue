#include "transport_catalogue.h"

namespace project {
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
          //  set<string_view> result;
          //  for (const auto bus_ptr : buses_at_stop.at(name)) {
           //     result.insert(bus_ptr->name);
           // }
           // auto it = &(buses_at_stop.at(name));
            return &(buses_at_stop.at(name));
        }

        optional<BusInfo> TransportCatalogue::GetBusInfo(const string_view name) const {
            auto p_bus = p_to_bus.find(name);
            if (p_bus == p_to_bus.end()) {
                return nullopt;
            }

            BusInfo result{};
            result.stops = p_bus->second->stops.size();
            result.u_stops = unordered_set(p_bus->second->stops.begin(), p_bus->second->stops.end()).size();
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

        void TransportCatalogue::AddBus(const string& name, const vector<string_view>&& route) {
            buses.push_back(Bus(name));
            auto p_bus = &buses.back();
            p_to_bus[p_bus->name] = p_bus;

            for (const auto& stop_name : route) {
                auto p_stop = p_to_stop.at(stop_name);
                p_bus->stops.push_back(p_stop);
                buses_at_stop[p_stop->name].insert(p_bus);
            }
        }

        void TransportCatalogue::AddStopAndDistance(const string&first_stop,const string&second_stop,int distance )
        {
            
            if (!p_to_stop.contains(first_stop))
            {
                stops.push_back(Stop{ first_stop,{} });
                p_to_stop[first_stop] = &stops.back();
            }
                if (p_to_stop.contains(second_stop))
                {
                    distance_between_stops.insert({ {p_to_stop[first_stop], p_to_stop[second_stop]}, distance});
                }
                else {
                    stops.push_back(second_stop);
                    auto pointer_to_stop = &(stops.back());
                    p_to_stop[pointer_to_stop->name] = pointer_to_stop;
                    distance_between_stops.insert({ { p_to_stop[first_stop], pointer_to_stop}, distance});
                }
            
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
                Stop* first_stop = p_bus->stops[i];
                Stop* second_stop = p_bus->stops[i + 1];
                if (distance_between_stops.contains({ first_stop, second_stop })) {
                    result += distance_between_stops.at({ first_stop, second_stop });
                }
                else if (distance_between_stops.contains({ second_stop, first_stop })) {
                    result += distance_between_stops.at({ second_stop, first_stop });
                }
            }
            return result;
        }
    }
}