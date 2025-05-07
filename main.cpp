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
       // fstream temp("112.txt");
        json::Document document = json::Load(cin);
        transport_catalogue::TransportCatalogue catalog;
        graph::DirectedWeightedGraph<double> graph;
        JsonReader reader(document,catalog,graph);
        json::Print(reader.AnswerToJson(),std::cout);
        return 0;
    }