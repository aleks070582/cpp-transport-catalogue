#include "transport_catalogue.h"

namespace project
{
	namespace transport_catalogue {
		using namespace std;
		StopInfo TransportCatalogue::get_stop_info(string& name) const
		{
			StopInfo result;
			result.exist = true;
			auto temp = find_if(stops.begin(), stops.end(), [&name](const Stop& ch)
				{
					return ch.name == name;
				});
			if (temp == stops.end())
			{
				result.exist = false;
				return result;
			}

			std::string* it = const_cast<string*>(&(temp->name));

			for (const auto& ch : stops_to_bus.at(it))
			{
				result.buses.push_back(ch->name);
			}

			sort(result.buses.begin(), result.buses.end());
			return result;
		}

		BusInfo TransportCatalogue::get_bus_info(const string& name) const
		{
			BusInfo result;
			auto temp = find_if(buses.begin(), buses.end(), [&name](const Bus& ch)
				{
					return ch.name == name;
				});

			if (temp == buses.end())
			{
				return BusInfo{};
			}
			string* it = const_cast<string*>(&(temp->name));
			result.name = name;
			result.number_of_stops = bus_to_stops.at(it).size();

			result.number_of_u_stops = get_unique_stops(it);
			result.route_lenght = calculate_route(it);
			return result;
		}

		void TransportCatalogue::add_bus(string& name)
		{
			buses.push_back({ name });
			auto p = find_bus(name);
			bus_to_stops[&(p->name)];
		}

		void TransportCatalogue::add_bus_route(string& name,vector<string_view>&& route)
		{
			auto bus = find_bus(name);
			for (auto ch : route)
			{
				string temp(ch);
				auto p_stop = find_stop(temp);
				bool already_consist = false;
				for (const Bus* ch : stops_to_bus.at(&(p_stop->name)))
				{
					if (ch->name == bus->name)
					{
						already_consist = true;
					}
				}
				if (already_consist == false)
				{
					stops_to_bus.at(&(p_stop->name)).push_back(bus);

				}

				bus_to_stops.at(&(bus->name)).push_back(p_stop);
			}


		}



		void TransportCatalogue::add_stop(string& name, Coordinates coord)
		{
			stops.push_back({ name, coord });
			auto p = find_stop(name);
			stops_to_bus[&(p->name)];
		}

		double TransportCatalogue::calculate_route(string* p_name) const
		{
			double result = 0;
			for (size_t i = 0; i < bus_to_stops.at(p_name).size() - 1; ++i)
			{
				Coordinates first, second;
				first = bus_to_stops.at(p_name).at(i)->coord;
				second = bus_to_stops.at(p_name).at(i + 1)->coord;
				result += ComputeDistance(first, second);
			}

			return result;
		}


		int TransportCatalogue::get_unique_stops(string* p_name) const
		{

			unordered_set <Stop*> temp({ bus_to_stops.at(p_name).begin(),bus_to_stops.at(p_name).end() });
			return temp.size();
		}

		Bus* TransportCatalogue::find_bus(string& name)
		{
			auto it = std::find_if(buses.begin(), buses.end(), [&name](const Bus& ch)
				{
					return ch.name == name;
				});

			return &(*it);
		}

		Stop* TransportCatalogue::find_stop(string& name)
		{
			auto it = find_if(stops.begin(), stops.end(), [&name](Stop& ch)
				{
					return ch.name == name;
				});
			return &(*it);
		}
	}
}
