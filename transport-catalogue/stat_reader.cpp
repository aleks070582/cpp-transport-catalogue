#include "stat_reader.h"
#include<iomanip>
namespace project 
{
    namespace stat_reader
    {
        using namespace transport_catalogue;
        using namespace std;
        void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, string_view request,
            ostream& output)
        {
            int temp=request.find(' ');
            string_view first_word, second_word;
            first_word = request.substr(0, temp);
            second_word = request.substr(temp + 1, request.size() - 1);
            if (first_word == "Bus")
            {
                optional<BusInfo> bus_info = transport_catalogue.get_bus_info(string(second_word));
                output << "Bus " << second_word << ": ";
                if (!bus_info)
                {
                    //Bus X : not found
                    output << "not found"<<endl;
                }
                else
                {
                   // Bus X : R stops on route, U unique stops, L route length
                    output << bus_info.value().stops << " stops on route, " << bus_info.value().u_stops <<
                        " unique stops, " << setprecision(6) << bus_info.value().lenght << " route length"<<endl;
                }
            }
            if (first_word == "Stop")
            {
                optional<vector<string_view>> stop_info = transport_catalogue.get_stop_info(string(second_word));
                output << "Stop " << second_word << ":";
                if (!stop_info)
                {
                    output << " not found" << endl;
                }
                else
                {
                    if (stop_info.value().size() == 0)
                    {
                        output << " no buses" << endl;
                    }
                    else
                    {
                        output << " buses";
                        for (const auto& ch : stop_info.value())
                        {
                            output << " " << ch;
                        }
                        output << endl;
                    }
                }
            }
            
        }
    }
}
