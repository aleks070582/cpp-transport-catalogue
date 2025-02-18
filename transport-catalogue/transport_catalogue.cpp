#include "transport_catalogue.h"

namespace project
{
	namespace transport_catalogue
	{
		using namespace std;

		
		 optional<set<string_view>> TransportCatalogue::GetStopInfo(const string_view name) const
		{ 
			
			auto p_stop = p_to_stop.find(name);
			if (p_stop == p_to_stop.end())
			{
				return nullopt;
			}
			 set<string_view> result{};
			for ( const auto buses_ :buses_at_stop.at(name))
			{
				result.insert(buses_->name);
			}
			return result;
		}

		 optional<BusInfo> TransportCatalogue::GetBusInfo(const string_view name) const
		{
			auto p_bus = p_to_bus.find(name);
			if (p_bus==p_to_bus.end())
			{
				return nullopt;
			}
		    BusInfo result{};
			result.stops = p_bus->second->stops.size();
			result.u_stops = unordered_set(p_bus->second->stops.begin(), p_bus->second->stops.end()).size();
			result.lenght = CalculatePath(&(*p_bus->second));
			return result ;
		}

		void TransportCatalogue::AddStop(const string& name, const geo::Coordinates coord)
		{
			
			stops.emplace_back(Stop(name,coord));
			auto p_stop = &stops.back();
			p_to_stop[p_stop->name] = p_stop;
			buses_at_stop.insert({ p_stop->name,{} });

		}

		void TransportCatalogue::AddBus(const string& name,const vector<string_view>&& route)
		{
			buses.emplace_back(Bus(name));
			auto p_bus = &buses.back();
			p_to_bus[p_bus->name] = p_bus;
			for (const auto& stops : route)
			{
				auto p_stop = p_to_stop.at(string(stops));
				p_bus->stops.push_back(p_stop);
				buses_at_stop[p_stop->name].insert(p_bus);
			}

		}

		double TransportCatalogue::CalculatePath(const Bus* p_bus) const
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
