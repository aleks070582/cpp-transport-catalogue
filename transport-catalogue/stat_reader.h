#pragma once
#include<iostream>
#include <iosfwd>
#include <string_view>
#include<utility>
#include "transport_catalogue.h"
namespace project {
    namespace stat_reader
    {

        void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
            std::ostream& output);
    }
}
