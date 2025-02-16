#include "stat_reader.h"
#include<iomanip>
namespace project 
{
    namespace stat_reader
    {
        using namespace transport_catalogue;
        using namespace std;
        void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, string_view request,
            ostream& output)
        {
            vector<string_view> temp;
            int i = request.find(' ', 0);
            temp.push_back(request.substr(0, i));
            temp.push_back(request.substr((i + 1), request.size() - i));


            if (temp.at(0) == "Bus")
            {
                BusInfo result = tansport_catalogue.get_bus_info(string(temp.at(1)));
                if (result.name == "")
                {
                    output << "Bus " << temp.at(1) << ": not found" << std::endl;
                    return;
                }
                output << "Bus " << result.name << ": " << result.number_of_stops << " stops on route, " <<
                    result.number_of_u_stops << " unique stops, " << setprecision(6) << result.route_lenght << " route length" << endl;
            }
            //Stop X : buses bus1 bus2
            if (temp.at(0) == "Stop")
            {
                std::string name(temp.at(1));
                StopInfo t = tansport_catalogue.get_stop_info(name);
                if (t.exist == false)
                {
                    output << "Stop " << temp.at(1) << ": not found" << endl;
                    return;
                }
                else if (t.buses.empty())
                {
                    output << "Stop " << temp.at(1) << ": no buses" << endl;
                }
                else
                {
                    output << "Stop " << name << ": buses";
                    for (auto& ch : t.buses)
                    {
                        output << " " << ch;
                    }
                    output << endl;
                }
            }
        }
    }
}
