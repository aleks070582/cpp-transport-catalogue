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

namespace project
{
	using namespace geo;
	namespace transport_catalogue
	{
		struct StopInfo
		{
			bool exist;
			std::vector <std::string> buses;

		};

		struct Stop
		{
			std::string name;
			Coordinates coord;
		};

		struct Bus
		{
			std::string name;
		};

		struct BusInfo
		{
			std::string name = "";
			int number_of_stops = 0;
			int number_of_u_stops = 0;// кол-во уникальных остановок;
			double route_lenght = 0;
		};
		class TransportCatalogue
		{
		public:
			StopInfo get_stop_info(std::string& name) const;
			BusInfo get_bus_info(const std::string& name) const;
			void add_bus(std::string& name);
			void add_bus_route(std::string& name, std::vector<std::string_view>&& route);
			void add_stop(std::string& name, Coordinates coord);
		private:
			double calculate_route(std::string* p_name) const;
			int get_unique_stops(std::string* p_name) const;
			Bus* find_bus(std::string& name);
			Stop* find_stop(std::string& name);
			std::deque<Stop> stops;
			std::deque<Bus> buses;
			std::unordered_map<std::string*, std::vector<Bus*>> stops_to_bus;
			std::unordered_map<std::string*, std::vector<Stop*>> bus_to_stops;
		};
	}
}
