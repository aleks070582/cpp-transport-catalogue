#include <iostream>
#include <string>
#include<sstream>
#include "input_reader.h"
#include "stat_reader.h"
#include<fstream>
using namespace std;




int main() {
    project::transport_catalogue:: TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        project::input_reader::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
       project::stat_reader::ParseAndPrintStat(catalogue, line, cout);
    }
}
