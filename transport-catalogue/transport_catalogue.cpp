#include "transport_catalogue.h"

namespace project
{
	namespace transport_catalogue
	{
		using namespace std;

		
		const optional<vector<string_view>> TransportCatalogue::get_stop_info(const string& name) const
		{ 
			
			auto p_stop = p_to_stop.find(name);
			if (p_stop == p_to_stop.end())
			{
				return nullopt;
			}
			vector<string_view> result{};
			for (const auto ch : p_stop->second->buses)
			{
				result.push_back(string_view(ch->name));
			}
			sort(result.begin(), result.end());
			return result;
		}

		optional<BusInfo> TransportCatalogue::get_bus_info(const string& name) const
		{
			auto p_bus = p_to_bus.find(name);
			if (p_bus==p_to_bus.end())
			{
				return nullopt;
			}
			BusInfo result;
			result.stops = p_bus->second->stops.size();
			result.u_stops = unordered_set(p_bus->second->stops.begin(), p_bus->second->stops.end()).size();
			result.lenght = calculate_path(&(*p_bus->second));
			return result ;
		}

		void TransportCatalogue::add_stop(const string& name, const geo::Coordinates coord)
		{
			
			stops.emplace_back(Stop(name,coord));
			auto p_stop = &stops.back();
			p_to_stop[p_stop->name] = p_stop;

		}

		void TransportCatalogue::add_bus(const string& name,const vector<string_view>&& route)
		{
			buses.emplace_back(Bus(name));
			auto p_bus = &buses.back();
			p_to_bus[p_bus->name] = p_bus;
			for (const auto& ch : route)
			{
				auto p_stop = p_to_stop.at(string(ch));
				p_bus->stops.push_back(p_stop);
				bool already_exist = false;
				for ( Bus* temp : p_stop->buses)
				{
					if (temp->name == p_bus->name)
					{
						already_exist = true;
				    }
				}
				if (already_exist == false)
				{
					p_stop->buses.push_back(p_bus);
				}
			}

		}

		double TransportCatalogue::calculate_path(const Bus* p_bus) const
		{
			double result=0;
			for (size_t i = 0; i < p_bus->stops.size() - 1; ++i)
			{
				auto first_coord = p_bus->stops.at(i)->coord;
				auto second_coord = p_bus->stops.at(i + 1)->coord;
				result +=geo::ComputeDistance(first_coord, second_coord);
			}
			return result;
		}
	}
}
