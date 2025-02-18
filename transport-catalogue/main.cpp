#include <iostream>
#include <string>
#include<sstream>
#include "input_reader.h"
#include "stat_reader.h"
#include<fstream>
using namespace std;




int main() {
    project::transport_catalogue::TransportCatalogue catalogue;
    fstream temp("tsB_case1_input.txt");
    project::input_reader::ReadInput(catalogue, temp);

    project::stat_reader::ReadInput(temp, cout, catalogue);
    
}
