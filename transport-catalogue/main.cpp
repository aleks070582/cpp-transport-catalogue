#include <iostream>
#include <string>
#include<sstream>
#include "input_reader.h"
#include "stat_reader.h"
#include<fstream>
using namespace std;




int main() {
   
    project::transport_catalogue::TransportCatalogue catalogue;
    project::input_reader::ReadInput(catalogue, cin);
    project::stat_reader::ReadInput(cin, cout, catalogue);
    
}
