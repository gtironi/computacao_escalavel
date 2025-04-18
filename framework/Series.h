#ifndef SERIES_HPP
#define SERIES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <algorithm>
#include <iomanip>
using namespace std;

class Series {
private:
    string strColumnName;
    string strColumnType;  // Ex.: "int", "double", "bool", "string"
    vector<any> vecColumnData;

    string anyToString(const any& value) const {
        try {
            if (value.type() == typeid(int))      return to_string(any_cast<int>(value));
            if (value.type() == typeid(double))   return to_string(any_cast<double>(value));
            if (value.type() == typeid(bool))     return any_cast<bool>(value) ? "true" : "false";
            if (value.type() == typeid(string))   return any_cast<string>(value);
        } catch (...) {
            return "[err]";
        }
        return "[unknown]";
    }

public:
    Series(const string& columnName, const string& columnType)
        : strColumnName(columnName), strColumnType(columnType) {}

    Series() = default;
    Series(const Series& other) = default;
    Series& operator=(const Series& other) = default;

    bool bAdicionaElemento(const any& elemento) {
        vecColumnData.push_back(anyToString(elemento));
        return true;
    }

    bool bRemovePeloIndex(int iIndex) {
        if (iIndex < 0 || iIndex >= static_cast<int>(vecColumnData.size())) {
            cerr << "Índice fora dos limites." << endl;
            return false;
        }
        vecColumnData.erase(vecColumnData.begin() + iIndex);
        return true;
    }

    any retornaElemento(int iIndex) const {
        if (iIndex >= 0 && iIndex < static_cast<int>(vecColumnData.size()))
            return vecColumnData[iIndex];
        cerr << "Índice fora dos limites." << endl;
        return any();
    }

    size_t iGetSize() const { return vecColumnData.size(); }
    void reserve(size_t n) { vecColumnData.reserve(n); }

    string strGetName() const { return strColumnName; }
    string strGetType() const { return strColumnType; }

    friend ostream& operator<<(ostream& os, const Series& series) {
        os << series.strColumnName << " <" << series.strColumnType << ">: [";
        size_t limit = min(series.vecColumnData.size(), static_cast<size_t>(5));
        for (size_t i = 0; i < limit; ++i) {
            os << series.anyToString(series.vecColumnData[i]);
            if (i < limit - 1) os << ", ";
        }
        if (series.vecColumnData.size() > 5) os << ", ...";
        os << "]\n";
        return os;
    }
};

#endif
