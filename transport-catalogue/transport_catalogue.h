#pragma once
#include<deque>
#include<string>
#include<vector>
#include<unordered_map>
#include<iterator>
#include<functional>
#include<algorithm>
#include<utility>
#include"geo.h"
#include<unordered_set>
#include<optional>
#include<set>
namespace project
{
	
	namespace transport_catalogue
	{
		struct Bus;
		struct Stop;
		
		
		struct BusInfo
		{
			unsigned int stops;
			unsigned int u_stops;
			double lenght;
		};
		struct Stop

		{
			Stop(const std::string&name_,const geo::Coordinates coord_) :name(name_),
				coord(coord_)//,buses{}
			{
			};
			std::string name;
			geo::Coordinates coord;
		//	std::vector<Bus*> buses;
		};
		struct Bus
		{
			Bus(const std::string& _name) :name(_name), stops{}
			{
			};
			std::string name;
			std::vector<Stop*> stops;
		}; 
		
		class TransportCatalogue
		{
		public:
		 std::optional<std::set<std::string_view>> GetStopInfo(const std::string_view name) const;
		 std::optional<BusInfo> GetBusInfo(const std::string_view name) const;
			void AddStop(const std::string& name, const geo::Coordinates coord);
			void AddBus(const std::string& name, const std::vector<std::string_view>&& route);
		private:
			double CalculatePath(const Bus* p_bus) const;
			std::deque<Stop> stops;
			std::deque<Bus> buses;
			std::unordered_map<std::string_view, Bus*> p_to_bus;
			std::unordered_map<std::string_view, Stop*> p_to_stop;
			std::unordered_map < std::string_view, std::unordered_set<Bus*>> buses_at_stop;
		};
	}
}
