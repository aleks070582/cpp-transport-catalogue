#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>



namespace project
{
    namespace input_reader {
        
        using namespace std;

        struct ParsedStopFromDescription
        {
            ParsedStopFromDescription() : coord({}), stops_and_distance({}) {};
            geo::Coordinates coord;
            unordered_map<string_view, int> stops_and_distance;
        };
        ParsedStopFromDescription ParseDescription(const string_view&command_description);
        void ReadInput(transport_catalogue::TransportCatalogue& transport_catalogue, istream& in)
        {
            int base_request_count;
            in >> base_request_count >> ws;

            {
               InputReader reader;
                for (int i = 0; i < base_request_count; ++i) {
                    string line;
                    getline(in, line);
                    reader.ParseLine(line);
                }
                reader.ApplyCommands(transport_catalogue);
            }
        }

       geo::Coordinates ParseCoordinates( const string_view& str,const string_view&str1) {
            double lat = stod(string(str));
            double lng = stod(string(str1));

            return { lat, lng };
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        string_view Trim(string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        vector<string_view> Split(string_view string, char delim) {
            vector<string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        vector<string_view> ParseRoute(string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            vector<string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { string(line.substr(0, space_pos)),
                    string(line.substr(not_space, colon_pos - not_space)),
                    string(line.substr(colon_pos + 1)) };
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }


        void InputReader::ApplyCommands([[maybe_unused]]transport_catalogue::TransportCatalogue& catalogue) const
        {
            
            vector<CommandDescription> commands_no_const(commands_);
            
            auto it_end_first_part=partition(commands_no_const.begin(), commands_no_const.end(), []( CommandDescription& command)
                {
                    return command.command == "Stop";
                });
                
           for_each(commands_no_const.begin(), it_end_first_part, [&catalogue](const CommandDescription& command)
                {
                   ParsedStopFromDescription temp = ParseDescription(command.description);
                   catalogue.AddStop(command.id, temp.coord);
                   if (!temp.stops_and_distance.empty())
                   {
                       for_each(temp.stops_and_distance.begin(), temp.stops_and_distance.end(), [&command, &catalogue]
                       (const pair<string_view, int>& ch)
                           {
                               catalogue.AddStopAndDistance(command.id, string(ch.first), ch.second);
                           });
                   }
                });
            for_each( it_end_first_part,commands_no_const.end(), [&catalogue](const CommandDescription& command)
                {
                    catalogue.AddBus(command.id, move(ParseRoute(command.description)));
                });
                           
        }
       
       
        ParsedStopFromDescription ParseDescription(const string_view& command_description)
        {
            ParsedStopFromDescription result;
           const  vector<string_view> parsed_description (move(Split(command_description, ',')));
            result.coord = ParseCoordinates(parsed_description[0], parsed_description[1]);
            if (parsed_description.size() >= 3) 
             {
                
                for (size_t i = 2; i < parsed_description.size(); ++i)
                {

                  const  string_view& str = parsed_description[i];
                    string_view  temp = str.substr(0, str.find_last_not_of('m'));
                    int distance = stoi(string(temp));
                    int size_to_stop = str.find("to")+3;//прибавляем к положению to 3 чтобы получить позицию остановки 
                    temp = str.substr(size_to_stop, str.size() - size_to_stop);
                    result.stops_and_distance.insert({temp,distance});
                }
               
            }
            
            return result;
        }

}
}
