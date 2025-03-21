#include"transport_catalogue.h"
#include"json.h"
#include<iostream>
#include"fstream"
#include"json_reader.h"
#include"map_renderer.h"
    using namespace std;
    int main() {
        /*
         * Примерная структура программы:
         *
         * Считать JSON из stdin
         * Построить на его основе JSON базу данных транспортного справочника
         * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
         * с ответами Вывести в stdout ответы в виде JSON
         */
        fstream temp("s10_final_opentest_3.json");
        json::Document document = json::Load(temp);
        transport_catalogue::TransportCatalogue catalog;
        ParseRequestJsonDocument(document,catalog);    
        ParseAndPrintStatJsonDocument(document, catalog, cout);
        return 0;
    }
