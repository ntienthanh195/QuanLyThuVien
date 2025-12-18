// Lightweight benchmark that scans data/dausach.txt for a keyword to measure I/O+parse cost.
// This does not depend on project internals and can be run cross-platform.

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string keyword = "C++";
    if (argc > 1) keyword = argv[1];

    std::string path = "../data/dausach.txt";
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Cannot open " << path << "\n";
        return 2;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) lines.push_back(line);
    std::cout << "Loaded " << lines.size() << " lines from " << path << "\n";

    using namespace std::chrono;
    auto t0 = high_resolution_clock::now();
    int found = 0;
    for (const auto &L : lines) {
        if (L.find(keyword) != std::string::npos) found++;
    }
    auto t1 = high_resolution_clock::now();
    std::cout << "Keyword='" << keyword << "' found=" << found << "\n";
    std::cout << "Scan time: " << duration_cast<milliseconds>(t1 - t0).count() << " ms\n";
    return 0;
}
