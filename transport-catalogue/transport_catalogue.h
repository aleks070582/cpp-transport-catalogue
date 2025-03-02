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
#include <unordered_set>
#include <optional>
#include <set>

namespace project {
    namespace transport_catalogue {
        struct Bus;
        struct Stop;

        struct PointerPairHash {
            std::size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
                std::size_t h1 = std::hash<const Stop*>{}(p.first);
                std::size_t h2 = std::hash<const Stop*>{}(p.second);
                return h1 * 31 + h2; 
            }
        };

        

        struct BusInfo {
            unsigned int stops;
            unsigned int u_stops;
            int lenght;
            double curvature;
        };

        struct Stop {
            Stop(const std::string& name_, const geo::Coordinates coord_) : name(name_), coord(coord_) {}
            Stop(const std::string& name_) : name(name_), coord({ 0,0 }) {}
            std::string name;
            geo::Coordinates coord;
            bool operator==(const Stop& other) const {
                return name == other.name;
            }
        };

        struct Bus {
            Bus(const std::string& _name) : name(_name), stops{} {}
            std::string name;
            std::vector<Stop*> stops;
        };

        struct BusPointerComparator {
            bool operator ()(const Bus* first, const Bus* second) const
            {
                return first->name < second->name;
            }
        };
        class TransportCatalogue {
        public:
            std::optional<const std::set<const Bus*, BusPointerComparator>*> GetStopInfo(const std::string_view name) const;
            std::optional<BusInfo> GetBusInfo(const std::string_view name) const;
            void AddStop(const std::string& name, const geo::Coordinates coord);
            void AddBus(const std::string& name, const std::vector<std::string_view>&& route);
            void AddStopAndDistance(const std::string& stop1,const std::string&stop2,int distance);
        private:
            
            double CalculateGeographicPath(const Bus* p_bus) const;
            int CalculatePath(const Bus* p_bus) const;
            std::deque<Stop> stops;
            std::deque<Bus> buses;
            std::unordered_map<std::string_view, Bus*> p_to_bus;
            std::unordered_map<std::string_view, Stop*> p_to_stop;
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, PointerPairHash> distance_between_stops;
            std::unordered_map<std::string_view, std::set<const Bus*,BusPointerComparator>> buses_at_stop;
           const std::set<const Bus*, BusPointerComparator> nullbus{};
        };
    }
}