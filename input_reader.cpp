#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>



namespace project
{
    namespace input_reader {
        
        using namespace std;
        pair<geo::Coordinates, optional<vector<pair<string_view, int>>>> ParseDescription(const string_view&command_description);
        pair<string_view, int> ParseDistanceAndStop(const string_view& str);
       
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
            vector<CommandDescription> commands_no_const =commands_;
            
            auto it_end_first_part=partition(commands_no_const.begin(), commands_no_const.end(), []( CommandDescription& command)
                {
                    return command.command == "Stop";
                });
                
           for_each(commands_no_const.begin(), it_end_first_part, [&catalogue](const CommandDescription& command)
                {
                    catalogue.AddStop(command.id, ParseDescription(command.description));
                });
            for_each( it_end_first_part,commands_no_const.end(), [&catalogue](const CommandDescription& command)
                {
                    catalogue.AddBus(command.id, move(ParseRoute(command.description)));
                });
                           
        }
       
        pair<geo::Coordinates, optional<vector<pair<string_view, int>>>> ParseDescription(const string_view& command_description)
        {
            pair<geo::Coordinates, optional<vector<pair<string_view, int>>>> result{};
            const vector<string_view> parsed_description = Split(command_description, ',');
            result.first = ParseCoordinates(parsed_description[0], parsed_description[1]);

            // Если есть информация о расстояниях до остановок
            if (parsed_description.size() >= 3) {
                vector<pair<string_view, int>> distances;
                for (size_t i = 2; i < parsed_description.size(); ++i) {
                    distances.push_back(ParseDistanceAndStop(parsed_description.at(i)));
                }
                result.second = distances; // Присваиваем вектор в optional
            }
            else {
                result.second = nullopt;
            }
            return result;
        }

       pair<string_view, int> ParseDistanceAndStop(const string_view& str)
       {
           
           string_view  result = str.substr(0, str.find_last_not_of('m'));
           int distance = stoi(string(result));
           result = str.substr(str.find("to") + 3, str.size() - (str.find("to")+3));  
           return {result,distance};
       }

}
}
