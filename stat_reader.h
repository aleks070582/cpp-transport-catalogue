#pragma once
#include<iostream>
#include <iosfwd>
#include <string_view>
#include<utility>
#include "transport_catalogue.h"
#include<set>
#include<sstream>

namespace project {
    namespace stat_reader
    {
        void ReadInput(std::istream& in, std::ostream &out, const transport_catalogue::TransportCatalogue&
            transport_catalogue);

        void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
            std::ostream& output);   
    }
}
