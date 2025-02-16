#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */

namespace project
{
    namespace input_reader {
        using namespace std;

       geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = stod(std::string(str.substr(not_space2)));

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
            // переписать с использованием find
            for (auto& ch : commands_)
            {
                if (ch.command == "Stop")
                {
                    string name = ch.id;
                    geo::Coordinates temp = ParseCoordinates(ch.description);
                    catalogue.add_stop(name, temp);
                }
            }
            for (const auto& ch : commands_)
            {

                if (ch.command == "Bus")
                {
                    string name = ch.id;

                    catalogue.add_bus(name);
                    catalogue.add_bus_route(name, std::move(ParseRoute(ch.description)));
                }
            }

        }
    }
}
