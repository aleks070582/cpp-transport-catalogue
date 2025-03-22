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
        //fstream temp(std::cin);
        json::Document document = json::Load(std::cin);
        transport_catalogue::TransportCatalogue catalog;
        JsonReader reader(document,catalog);
        json::Print(reader.AnswerToJson(),std::cout);
        return 0;
    }