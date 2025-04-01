#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>

using namespace std;

#define DTYPES \
    int, double, string, bool, char
#define VDTYPES variant<DTYPES>

map<string, string> TYPEMAP = {
    {typeid(int).name(), "int"},
    {typeid(double).name(), "double"},
    {typeid(string).name(), "string"},
    {typeid(bool).name(), "bool"},
    {typeid(char).name(), "char"}
};

struct DataColumn
{
    string columnName;
    string columnType = "none";
    vector<VDTYPES> columnData;

    DataColumn(const string& name, const string& type)
        : columnName(name), columnType(type) {}

    void addData(const VDTYPES& data) {
        if (columnType != TYPEMAP[typeid(data).name()]) {
            cout << "Error: Data type mismatch in column " << columnName << endl;
        }

        columnData.push_back(data);
    }
};

struct DataFrame {
    vector<DataColumn> columns;
    vector<string> columnNames;
};


#endif