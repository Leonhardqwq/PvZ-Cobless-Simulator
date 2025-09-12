#pragma once
#include <cstddef>
#include <vector>
#include <cstring>
#include <fstream>

template <typename To, typename From>
To bit_cast(const From& src) {
    static_assert(sizeof(To) == sizeof(From), "Size mismatch in bitcast");
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template<typename T>
void write_vector_to_csv(const std::vector<T>& data, const std::string& filename, bool fixed=false) {
    std::ofstream fout(filename);
    if (!fout) return;
    fout << "index,value\n";
    if (fixed)  fout << std::fixed << std::setprecision(5);
    else        fout << std::scientific << std::setprecision(8);
    for (size_t i = 0; i < data.size(); ++i) 
        fout << i << "," << data[i] << "\n";
    fout.close();
}

template<typename T>
void write_2dvector_to_csv(const std::vector<std::vector<T>>& data, const std::string& filename, bool fixed=false) {
    std::ofstream fout(filename);
    if (!fout) return;

    fout << "index";
    for(size_t i =0; i < data.size(); ++i) 
        fout << ",value" << i;
    fout << "\n";

    if (fixed)  fout << std::fixed << std::setprecision(5);
    else        fout << std::scientific << std::setprecision(8);
    for (size_t i = 0; i < data[0].size(); ++i) {
        fout << i;
        for (size_t j = 0; j < data.size(); ++j) 
            fout << "," << data[j][i];
        fout << "\n";
    }
    fout.close();
}

std::pair<double, double> wilson_confidence_interval(double p, int total, double z=1.959963984540054) {
    double a = total + z * z;
    double b = -2 * total * p - z * z;
    double c = total * p * p;
    double lower_bound = (-b - std::sqrt(b * b - 4 * a * c)) / (2 * a);
    double upper_bound = (-b + std::sqrt(b * b - 4 * a * c)) / (2 * a);
    return {lower_bound, upper_bound};
}
