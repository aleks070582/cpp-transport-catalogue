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
				coord(coord_),buses{}
			{
			};
			std::string name;
			geo::Coordinates coord;
			std::vector<Bus*> buses;
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
			const std::optional<std::vector<std::string_view>> get_stop_info(const std::string& name) const;
		    std::optional<BusInfo> get_bus_info(const std::string& name) const;
			void add_stop(const std::string& name, const geo::Coordinates coord);
			void add_bus(const std::string& name, const std::vector<std::string_view>&& route);
		private:
			double calculate_path(const Bus* p_bus) const;
			std::deque<Stop> stops;
			std::deque<Bus> buses;
			std::unordered_map<std::string, Bus*> p_to_bus;
			std::unordered_map<std::string, Stop*> p_to_stop;
		};
	}
}
