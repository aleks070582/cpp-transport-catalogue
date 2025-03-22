#include "transport_catalogue.h"


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
}
