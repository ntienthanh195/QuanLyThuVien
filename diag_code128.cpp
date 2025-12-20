
#include <iostream>
#include <vector>
#include <string>

// Simulating Code128 class for testing
class Code128 {
public:
    static const int patterns[][6];
    static std::string encodeToString(const std::string& text) {
        bool useC = true;
        for (char c : text) {
            if (c < '0' || c > '9') { useC = false; break; }
        }
        if (useC && (text.length() % 2 != 0)) useC = false;

        int startCode = useC ? 105 : 104;
        int checkSum = startCode;
        std::vector<int> codes;
        codes.push_back(startCode);

        if (useC) {
            for (size_t i = 0; i < text.length(); i += 2) {
                int val = std::stoi(text.substr(i, 2));
                codes.push_back(val);
            }
        } else {
            for (char c : text) {
                int val = (int)c - 32;
                codes.push_back(val);
            }
        }

        for (size_t i = 1; i < codes.size(); ++i) {
            checkSum += codes[i] * i;
        }
        checkSum %= 103;
        codes.push_back(checkSum);
        codes.push_back(106);

        std::string result = "";
        for (int code : codes) {
            const int* p = nullptr;
            int stopPat[] = {2,3,3,1,1,1,2};
            int manualStartB[] = {2,1,1,2,1,4};
            int manualStartC[] = {2,1,1,2,3,2};

            if (code < 103) p = patterns[code];
            else if (code == 104) p = manualStartB;
            else if (code == 105) p = manualStartC;
            
            if (code == 106) {
                bool isBar = true;
                for (int w : stopPat) {
                    for (int k=0; k<w; ++k) result += (isBar ? "1" : "0");
                    isBar = !isBar;
                }
            } else if (p) {
                bool isBar = true;
                for (int i=0; i<6; ++i) {
                    for (int k=0; k<p[i]; ++k) result += (isBar ? "1" : "0");
                    isBar = !isBar;
                }
            }
        }
        return result;
    }
};

const int Code128::patterns[][6] = {
    {2,1,2,2,2,2},{2,2,2,1,2,2},{2,2,2,2,2,1},{1,2,1,2,2,3},{1,2,1,3,2,2},{1,3,1,2,2,2},{1,2,2,2,1,3},{1,2,2,3,1,2},
    {1,3,2,2,1,2},{2,2,1,2,1,3},{2,2,1,3,1,2},{2,3,1,2,1,2},{1,1,2,2,3,2},{1,2,2,1,3,2},{1,2,2,2,3,1},{1,1,3,2,2,2},
    {1,2,3,1,2,2},{1,2,3,2,2,1},{2,2,3,2,1,1},{2,2,1,1,3,2},{2,2,1,2,3,1},{2,1,3,2,1,2},{2,2,3,1,1,2},{3,1,2,1,3,1},
    {3,1,1,2,2,2},{3,2,1,1,2,2},{3,2,1,2,2,1},{3,1,2,2,1,2},{3,2,2,1,1,2},{3,2,2,2,1,1},{2,1,2,1,2,3},{2,1,2,3,2,1},
    {2,3,2,1,2,1},{1,1,1,3,2,3},{1,3,1,1,2,3},{1,3,1,3,2,1},{1,1,2,3,1,3},{1,3,2,1,1,3},{1,3,2,3,1,1},{2,1,1,3,1,3},
    {2,3,1,1,1,3},{2,3,1,3,1,1},{1,1,2,1,3,3},{1,1,2,3,3,1},{1,3,2,1,3,1},{1,1,3,1,2,3},{1,1,3,3,2,1},{1,3,3,1,2,1},
    {3,1,3,1,2,1},{2,1,1,3,3,1},{2,3,1,1,3,1},{2,1,3,1,1,3},{2,1,3,3,1,1},{2,1,3,1,3,1},{3,1,1,1,2,3},{3,1,1,3,2,1},
    {3,3,1,1,2,1},{3,1,2,1,1,3},{3,1,2,3,1,1},{3,3,2,1,1,1},{3,1,4,1,1,1},{2,2,1,4,1,1},{4,3,1,1,1,1},{1,1,1,2,2,4},
    {1,1,1,4,2,2},{1,2,1,1,2,4},{1,2,1,4,2,1},{1,4,1,1,2,2},{1,4,1,2,2,1},{1,1,2,2,1,4},{1,1,2,4,1,2},{1,2,2,1,1,4},
    {1,2,2,4,1,1},{1,4,2,1,1,2},{1,4,2,2,1,1},{2,4,1,2,1,1},{2,2,1,1,1,4},{4,1,3,1,1,1},{2,4,1,1,1,2},{1,3,4,1,1,1},
    {1,1,1,2,4,2},{1,2,1,1,4,2},{1,2,1,2,4,1},{1,1,4,2,1,2},{1,2,4,1,1,2},{1,2,4,2,1,1},{4,1,1,2,1,2},{4,2,1,1,1,2},
    {4,2,1,2,1,1},{2,1,2,1,4,1},{2,1,4,1,2,1},{4,1,2,1,2,1},{1,1,1,1,4,3},{1,1,1,3,4,1},{1,3,1,1,4,1},{1,1,4,1,1,3},
    {1,1,4,3,1,1},{4,1,1,1,1,3},{4,1,1,3,1,1},{1,1,3,1,4,1},{1,1,4,1,3,1},{3,1,1,1,4,1},{4,1,1,1,3,1},
    {2,1,1,4,1,2}, // 103 (StartA)
    {2,1,1,2,1,4}, // 104 (StartB)
    {2,1,1,2,3,2}  // 105 (StartC)
};

int main() {
    std::string testStr = "9786040000675-1";
    std::string result = Code128::encodeToString(testStr);
    std::cout << "Encoded: " << testStr << std::endl;
    std::cout << "Pattern: " << result << std::endl;
    std::cout << "Length: " << result.length() << std::endl;
    return 0;
}
