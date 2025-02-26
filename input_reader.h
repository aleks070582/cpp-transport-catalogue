#pragma once
#include <string>
#include <string_view>
#include <vector>
#include<cstdlib>
#include<sstream>
#include<iostream>
#include "geo.h"
#include<iomanip>
#include "transport_catalogue.h"
#include<algorithm>
namespace project
{
    namespace input_reader
    {
        void ReadInput(transport_catalogue::TransportCatalogue& transport_catalogue,std::istream& in);
      
        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // Название команды
            std::string id;           // id маршрута или остановки
            std::string description;  // Параметры команды
        };

        class InputReader {
        public:
            /**
             * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
             */
            void ParseLine(std::string_view line);
           
            /**
             * Наполняет данными транспортный справочник, используя команды из commands_
             */
            void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };
    }
}
