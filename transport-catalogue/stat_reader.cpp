#include "stat_reader.h"
#include<iomanip>

namespace project 
{
    namespace stat_reader
    {
        using namespace transport_catalogue;
        using namespace std;
      
        void ReadInput(std::istream& in, std::ostream& out, const transport_catalogue::TransportCatalogue& transport_catalogue)
        {
            int stat_request_count;
            in >> stat_request_count >> ws;
            for (int i = 0; i < stat_request_count; ++i) {
                string line;
                getline(in, line);
                ParseAndPrintStat(transport_catalogue, line, out);
            }
        }

        void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, string_view request,
            ostream& output)
        {
            int temp=request.find(' ');
            string_view command, name;
            command = request.substr(0, temp);
            name = request.substr(temp + 1, request.size() - 1);
            if (command == "Bus")
            {
               const optional<BusInfo> bus_info=transport_catalogue.GetBusInfo (name);
                       
                output << "Bus " << name << ": ";
                if (!bus_info)
                {
                    //Bus X : not found
                    output << "not found"<<endl;
                }
                else
                {
                  const auto&bus_info_value = bus_info.value();    
                   // Bus X : R stops on route, U unique stops, L route length
                    output << bus_info_value.stops << " stops on route, " << bus_info_value.u_stops <<
                        " unique stops, "  <<bus_info_value.lenght << " route length, " << setprecision(6)<<
                        bus_info_value.curvature << " curvature" << endl;
                }
            }
            if (command == "Stop")
            {
                optional<const set<const Bus*, BusPointerComparator>*> stop_info=transport_catalogue.GetStopInfo(name);
                output << "Stop " << name << ":";
                if (!stop_info)
                {
                    output << " not found" << endl;
                }
                else
                {
                    if (stop_info.value()->empty())
                    {
                        output << " no buses" << endl;
                    }
                    else
                    {
                        output << " buses";
                        for (const auto&ch : *stop_info.value())
                        {
                            output << " " << ch->name;
                        }
                        output << endl;
                    }
                }
            }
            
        }
    }
}
