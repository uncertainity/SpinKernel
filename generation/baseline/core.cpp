#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace std;

template<typename T>
inline std::string type_name() {
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
        std::free
    };
    return status == 0 ? res.get() : typeid(T).name();
}

template<typename T>
inline std::string shape_of(const T&) { return "[1]"; }

template<typename T>
inline std::string shape_of(const std::vector<T>& v) { return "[" + std::to_string(v.size()) + "]"; }

template<typename T>
inline std::string shape_of(const std::vector<std::vector<T>>& v) {
    size_t rows = v.size();
    size_t cols = rows ? v[0].size() : 0;
    return "[" + std::to_string(rows) + "x" + std::to_string(cols) + "]";
}

template<typename T, size_t N>
inline std::string shape_of(const std::array<T, N>&) { return "[" + std::to_string(N) + "]"; }

template<typename T, size_t R, size_t C>
inline std::string shape_of(const std::array<std::array<T, C>, R>&) {
    return "[" + std::to_string(R) + "x" + std::to_string(C) + "]";
}

struct Trace {
    static inline int depth = 0;
    std::string fn;

    Trace(const char* function_name) : fn(function_name) {
        std::cout << std::string(depth * 2, ' ') << "->" << fn << "\n";
        depth++;
    }

    template<typename T>
    void input(const std::string& name, const T& value) {
        std::cout << std::string(depth * 2, ' ')
                  << "input " << name << ": " << type_name<T>()
                  << " shape:" << shape_of(value) << "\n";
    }

    template<typename T>
    void output(const std::string& name, const T& value) {
        std::cout << std::string(depth * 2, ' ')
                  << "output " << name << ": " << type_name<T>()
                  << " shape:" << shape_of(value) << "\n";
    }

    template<typename T>
    void GlobalandStruct(const std::string& name, const T& value) {
        std::cout << std::string(depth * 2, ' ')
                  << "Global/Struct " << name << ": " << type_name<T>()
                  << " shape:" << shape_of(value) << "\n";
    }

    ~Trace() {
        depth--;
        std::cout << std::string(depth * 2, ' ') << "<- " << fn << "\n";
    }
};

enum Symbol {
    HV1, HV2, HV3, HV4, HV5,
    LV1, LV2, LV3, LV4, LV5, LV6,
    WILD, SCAT, COR, COLLECT
};

enum class FeatureChoice { NONE, FREE_SPINS, BLITZ_SPINS };
enum class ScatterEvaluationRule { ANYWHERE, LEFT_TO_RIGHT };

constexpr int NO_OF_REELS = 6;
constexpr int NO_OF_ROWS = 4;
constexpr int STANDARD_SYMBOL_COUNT = 11;
constexpr int PAYTABLE_SYMBOL_COUNT = 12;
constexpr int BASE_BET_FIXED_COINS = 20;

using PayWindow = std::vector<std::vector<Symbol>>;
using ReelSet = std::vector<std::vector<Symbol>>;
using Cell = std::pair<int, int>;

inline const std::array<Symbol, STANDARD_SYMBOL_COUNT> StandardSymbols = {
    HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6
};

inline const std::array<std::array<int, 7>, PAYTABLE_SYMBOL_COUNT> PayTable = {{
    {{0, 0, 20, 30, 40, 50, 100}},
    {{0, 0, 10, 20, 30, 40, 50}},
    {{0, 0, 10, 20, 30, 40, 50}},
    {{0, 0, 4, 16, 24, 30, 40}},
    {{0, 0, 4, 16, 24, 30, 40}},
    {{0, 0, 0, 10, 20, 24, 30}},
    {{0, 0, 0, 10, 20, 24, 30}},
    {{0, 0, 0, 8, 16, 20, 24}},
    {{0, 0, 0, 8, 16, 20, 24}},
    {{0, 0, 0, 6, 10, 16, 20}},
    {{0, 0, 0, 6, 10, 16, 20}},
    {{0, 0, 0, 20, 40, 60, 100}}
}};

inline const ReelSet BG1_Reels = {
    {LV2, LV4, LV3, HV2, HV1, LV2, LV4, LV2, LV3, LV4, LV6, COLLECT, LV2, LV3, HV3, LV2, LV3, HV1, LV4, LV2, LV2, HV2, LV4, HV1, LV2, LV4, LV3, LV4, SCAT, LV2, HV4, LV3, HV5, LV4, LV5, HV5, LV3, HV1, LV6, LV2, LV4, LV3, LV4, LV4, HV1, LV2, HV3, LV3, LV2, HV1, LV3, LV4, LV3, LV4, HV1, LV2, HV4, LV2, LV3, LV4, HV1, LV2, LV3, LV2, LV4, LV3, HV1, LV4, LV2, SCAT, LV3, LV4, LV6, LV3, LV2, LV3, HV1, HV1, HV1, HV2, LV3, LV4, LV2, LV5, HV1, LV1, LV2, COR, COR},
    {HV5, LV1, LV6, HV1, HV1, HV1, LV5, HV5, HV5, LV3, LV1, COR, COR, HV3, LV1, LV3, HV2, HV3, LV4, HV5, LV5, LV1, HV5, HV2, LV1, HV5, LV5, WILD, LV1, HV5, HV5, LV5, LV1, HV4, HV5, LV5, LV1, HV5, LV2, HV5, LV5, SCAT, LV1, LV5, LV5, HV5, LV5, WILD, LV5, LV1, HV5, LV5, LV5, LV1, LV5, HV4, LV5, HV5, LV1, HV5, HV5, LV1, LV5, LV1, HV3, HV2, LV1, LV5, LV1, LV5, HV5, LV1, HV1, LV1, LV5, LV1, HV5, LV5, LV5, LV1, LV5, HV5},
    {LV4, HV2, HV4, LV4, LV4, HV2, LV5, LV5, HV3, LV4, LV6, LV5, LV5, LV3, SCAT, LV4, LV5, HV1, HV4, LV5, LV6, LV5, LV5, HV4, HV1, SCAT, LV5, LV4, HV3, LV4, HV4, LV5, LV4, HV4, LV5, HV4, LV5, HV4, HV4, LV3, LV5, LV5, HV4, LV5, WILD, LV4, HV4, LV5, HV4, LV6, LV1, LV5, LV5, HV4, HV4, HV2, HV4, LV3, LV3, HV1, LV2, HV4, HV5, LV6, LV4, HV2, HV4, LV4, LV4, HV2, LV5, LV5, SCAT, HV3, LV4, LV6, LV5, LV5, LV3, HV1, HV1, HV1, HV4, LV5, SCAT, LV5, LV5, HV4, HV1, LV5, LV4, HV3, LV4, HV4, LV5, WILD, HV4, LV5, HV4, LV5, HV4, HV4, LV3, LV5, LV5, HV4, LV5, WILD, LV4, HV4, LV5, HV4, LV6, LV1, LV5, LV5, HV4, HV4, HV2, HV4, LV3, LV3, HV1, LV2, HV4, HV5, LV6, COR, COR, COR},
    {HV1, HV1, HV1, LV2, LV5, LV2, LV3, LV5, LV1, WILD, LV5, LV2, LV5, LV3, LV6, LV6, LV5, LV3, LV5, LV2, HV1, LV6, LV3, LV5, HV1, HV5, HV4, HV3, HV3, SCAT, HV3, HV3, LV5, LV5, HV2, LV5, LV5, HV3, LV1, LV5, LV1, LV4, HV1, LV1, LV1, LV5, LV1, WILD, LV5, LV1, LV1, LV5, HV3, LV1, LV1, HV1, LV5, HV2, LV2, HV3, LV3, COR, COR, COR, LV1},
    {LV2, LV2, HV5, HV3, HV3, LV3, LV3, LV1, HV4, HV3, LV5, LV5, LV5, WILD, HV2, HV2, HV3, LV1, LV1, LV4, HV5, LV6, HV1, LV1, LV1, LV3, LV2, HV4, HV5, LV1, SCAT, LV5, LV5, HV1, HV4, HV4, LV5, HV4, HV3, LV1, LV1, HV5, HV4, HV4, HV5, HV3, LV3, LV5, HV3, HV1, LV1, HV5, HV2, HV3, LV1, LV4, HV1, HV1, HV1, LV4, HV2, LV2, LV4, HV5, LV3, HV1, WILD, LV2, LV2, HV5, LV3, HV2, HV2, LV5, LV5, HV4, HV3, HV5, LV2, LV2, COR, COR, COR},
    {COR, LV3, COLLECT, HV4, HV3, HV5, LV5, COR, COR, HV2, HV2, LV5, LV3, COLLECT, HV3, LV3, LV5, HV3, HV1, HV5, HV4, HV3, SCAT, HV4, LV1, HV5, HV4, HV4, LV1, SCAT, LV1, HV4, HV5, LV2, HV3, HV3, HV5, LV5, HV1, HV1, HV1, LV2, HV3, LV1, LV5, LV6, LV1, LV4, WILD, HV3, HV4, LV1, LV5, HV1, LV1, HV4, HV1, LV1, LV1, HV3, HV3, HV3, LV1, HV1, LV1, LV5, HV2, LV4, LV1, HV5, HV1, LV6, LV2, LV2, LV2, HV3, HV2, LV1, WILD, LV3, LV2, HV5}
};
inline const ReelSet BG2_Reels = {
    {HV1, LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5},
    {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, LV1, LV4, HV2, LV3, LV2, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5},
    {LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, HV2, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5},
    {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, HV1, LV1, LV4, HV2, LV3, LV2, HV3, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5},
    {HV1, LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, HV2, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5},
    {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, HV1, LV1, LV4, HV2, LV3, LV2, HV3, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5}
};
inline const ReelSet FG1_Reels = {
    {HV2, LV2, LV3, HV3, HV1, LV2, HV4, HV3, LV4, LV2, LV3, HV3, LV4, HV2, LV2, LV4, LV3, HV3, HV2, LV2, LV1, LV6, LV3, LV4, HV1, HV2, HV3, HV4, LV5, LV2, LV4, LV3, HV1, HV2, HV5, LV2, LV4, LV3, LV2, LV6, HV2, LV4, LV3, LV4, HV1, LV2, HV2, LV1, LV3, LV4, HV1, LV2, LV3, HV5, LV2, LV3, HV2, LV4, LV2, LV6, LV3, LV4, LV2, HV3, LV2, LV3, LV6, LV4, LV3, HV2, LV4, LV4, HV4, LV3, LV2, LV3, LV6, LV4, LV3, HV5, LV3, LV4, LV2, LV4, LV3, HV2, HV1, LV1, LV4, LV2, LV3, LV4, HV2, LV2, LV2, HV3, LV4, LV2, LV3, HV3, LV4, LV2, HV4, LV3, LV4, HV2, LV2, LV4, LV3, LV4, LV6, LV2, LV1, LV3, LV3, LV4, LV2, LV5, HV1, LV6, LV5, LV2, LV4, LV3, LV4, HV1, HV2, LV4, LV4, LV2, LV3, LV3, HV1, LV4, LV2, SCAT, LV3, LV4, HV1, HV3, LV2, LV3, HV1, SCAT},
    {LV1, LV6, LV1, LV5, SCAT, LV1, LV4, HV1, LV1, LV4, LV1, WILD, LV5, LV1, LV4, LV2, LV4, HV2, HV4, HV2, LV6, LV4, HV4, HV1, HV5, LV3, LV3, LV1, LV5, WILD, LV4, LV1, LV5, HV1, LV6, HV5, SCAT, HV1, LV1, HV3, HV2, LV1, LV1, HV1, LV1, HV5, LV1, HV4, HV2, LV1, HV5, HV2, HV1, HV3, HV4, HV2, LV5, LV5, HV4, HV2, HV1, LV4, HV4, LV4, HV5, LV3, HV1, LV1, LV5, LV3, WILD, LV1, HV4, LV5, HV1, HV5, HV4, HV1, SCAT, HV3, HV2, LV1, LV1, HV1, LV1, HV5, LV1, HV4, LV4, LV1, HV5},
    {LV4, HV5, LV3, LV4, LV6, WILD, LV2, LV3, LV4, HV4, HV1, LV4, HV5, SCAT, HV3, HV2, LV4, LV6, HV4, HV4, HV4, LV5, HV1, HV2, LV5, LV4, LV5, LV4, LV6, LV5, LV3, HV4, LV5, HV4, LV3, HV3, HV4, LV2, HV1, LV5, LV3, LV5, HV4, LV5, HV2, HV1, HV4, LV4, HV3, LV4, LV6, LV2, SCAT, HV4, LV2, LV3, LV5, WILD, LV6, LV2, LV5, HV4, LV1, HV3, LV4, HV5, HV4, LV4, HV2, LV4, HV4, HV1, LV4, HV5, HV2, HV3, HV2, LV4, HV1, LV6, LV2, LV5, WILD, LV1, LV4, LV6, LV5, HV1, HV2, HV4, LV5, HV4, LV3, HV3, HV4, LV2, HV1, HV1, HV4, LV4, HV5},
    {HV1, HV3, LV6, LV5, HV1, HV5, LV5, SCAT, HV1, HV2, HV4, HV3, LV5, HV5, HV4, HV2, LV6, LV5, LV4, WILD, LV3, LV1, LV4, LV1, LV1, SCAT, HV4, LV1, LV4, HV1, HV5, LV1, HV2, LV2, LV3, LV6, WILD, LV2, LV1, LV5, HV1, LV1, HV3, HV4, LV5, HV2, LV4, HV1, HV3, HV4, HV5, LV1, LV5, LV4, LV6, WILD, LV1, LV4, LV5, LV6},
    {LV2, HV2, HV5, HV3, LV3, HV1, LV1, HV4, SCAT, LV5, LV4, HV1, HV3, HV2, HV2, HV1, LV1, LV2, LV4, HV5, HV1, HV5, HV4, HV3, LV5, HV1, SCAT, HV2, LV1, HV5, HV1, HV4, HV2, HV3, LV3, LV5, HV3, HV1, LV1, HV5, HV2, HV3, LV1, LV4, LV6, LV2, HV1, LV4, HV2, LV2, LV4, HV5, LV3, HV4, HV1, LV2, LV2, HV5, LV3, HV2, HV2, LV5, LV5, HV4, HV1, LV5, LV5, LV5, LV4, HV3, LV3, LV3, HV4, LV2, HV1, HV1, LV2, LV3, HV3, LV3, HV2, HV5, LV3, SCAT, HV2, HV4, HV1, HV5, HV2, HV5, HV1, HV2, LV3, LV5, LV5, HV3, HV1, HV3, LV3, HV1, HV2, LV3, LV1, HV4, HV3, HV1, HV3, HV3, LV2, HV1, LV6, LV1, HV5, HV4, LV5, LV3, HV1, HV3, HV2, HV1, LV3, HV5, LV5, HV4, LV5, HV4, LV5, LV3, HV1, HV2, LV3, HV3, HV3, LV2, HV1, HV5, HV2, HV3, HV4, HV5, LV1, HV1, HV2, HV1, LV5, HV3, LV2, LV6, HV4, LV5, HV2, HV5, HV1, HV4, HV2, HV1, HV1, HV3, LV5, LV2, LV5, LV3, HV1, HV2, LV3, LV5, HV3},
    {HV4, LV3, HV2, LV1, HV3, HV5, LV5, HV1, HV3, HV2, HV2, LV5, LV3, LV6, HV3, LV3, LV5, HV3, HV1, HV5, HV4, HV3, LV1, HV2, LV1, HV5, HV3, HV4, LV1, LV6, LV1, HV4, HV5, SCAT, HV3, HV5, LV5, HV1, HV2, HV4, HV3, LV1, LV5, HV1, LV1, LV4, HV2, HV3, HV4, LV1, LV5, HV1, LV1, HV4, LV6, LV1, LV1, HV3, HV3, HV3, LV1, HV1, LV1, LV5, HV2, LV4, LV1, HV5, HV1, HV5, LV1, LV1, LV1, HV3, HV2, LV1, LV1, LV1, HV1, HV5, LV5, HV4, LV4, HV5, LV1, LV1, HV1, HV3, LV5, HV3, HV1, HV5, LV5, HV3, HV5, LV5, HV1, HV5, HV3, LV1, LV6, HV3, HV5, LV1, HV3, HV4, HV5, LV1, HV4, LV1, HV5, HV4, HV4, LV1, LV1, LV4, HV4, HV5, SCAT, HV3, HV4, LV6, HV2, HV3, HV4, HV5, HV3, HV5, HV4, HV2, HV3, HV5, HV4, HV3, HV1, HV1, HV5, LV1, HV3, HV5, HV5, HV4, HV3, LV1, HV4, HV3, LV2, LV2, HV4, LV4, HV5, LV1, HV4, HV3, LV2, HV4, HV3, HV5, HV3, HV2, LV2, HV5, HV4, SCAT, HV3, HV5, HV5, HV4, LV5, HV2, HV1}
};

inline const std::vector<int> BG1_ReelSize = {89, 82, 130, 65, 83, 82};
inline const std::vector<int> BG2_ReelSize = {26, 25, 26, 27, 27, 27};
inline const std::vector<int> FG1_ReelSize = {144, 91, 101, 60, 167, 171};

struct WeightedDoubleTable { std::vector<double> values; std::vector<int> weights; };

struct ProbTable {
    std::vector<int> ReelsetWeights{58, 42};
    std::vector<int> corTrigger{1, 9};
    std::vector<int> numCorWeight{240, 200, 150, 100, 80};
    std::vector<int> wildMult{480, 545, 360};
    std::vector<int> freeGamenumCollect{75, 25};
    std::vector<int> freeGameCOROccur{26, 74};
    std::vector<int> goodCORTable{0, 0, 0, 0, 0, 0, 0, 606, 893, 600, 600, 500, 500, 500, 500, 400, 300, 100, 30, 10, 10, 3};
    std::vector<int> badCORTable{17500, 7000, 3000, 2500, 2000, 2000, 1500, 1000, 1000, 500, 500, 100, 100, 50, 50, 50, 30, 20, 10, 0, 0, 0};
    std::vector<double> CORValues{0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 7, 7.5, 8, 9, 10, 12.5, 25, 50, 125, 250, 500, 2000};
    WeightedDoubleTable baseCORValueWeights{
        {0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 7, 7.5, 8, 9, 10, 12.5, 25, 50, 125, 250, 500, 2000},
        {7000, 6000, 6000, 5000, 3000, 3000, 2500, 2000, 1000, 1000, 1000, 100, 100, 50, 50, 50, 30, 20, 10, 10, 5, 1}
    };
};

inline ProbTable probTable;
inline ScatterEvaluationRule freeSpinsScatterEvaluationRule = ScatterEvaluationRule::ANYWHERE;
inline std::map<Cell, double> g_corValues;
inline std::map<Cell, int> g_wildMultipliers;
inline std::vector<int> g_forcedWeightedIndexes;
inline std::vector<int> g_forcedStops;

struct WinBreakdown {
    int ways_win = 0;
    int scatter_win = 0;
    int scatter_count = 0;
    std::map<Symbol, int> symbol_hits;
};

struct CollectResult {
    int collect_count = 0;
    double cor_value_x_bet_sum = 0.0;
    int collect_win = 0;
};

struct FeatureResult {
    int session_win = 0;
    int spins_played = 0;
    int retrigger_count = 0;
};

struct SpinResult {
    int base_win = 0;
    int scatter_win = 0;
    int collect_win = 0;
    int feature_win = 0;
    int total_win = 0;
    int scatter_count = 0;
};

inline void clearForcedState() {
    g_corValues.clear();
    g_wildMultipliers.clear();
    g_forcedWeightedIndexes.clear();
    g_forcedStops.clear();
}

inline void setForcedCorValue(int row, int reel, double value) { g_corValues[{row, reel}] = value; }
inline void setForcedWildMultiplier(int row, int reel, int value) { g_wildMultipliers[{row, reel}] = value; }
inline void setForcedWeightedIndexes(const std::vector<int>& indexes) { g_forcedWeightedIndexes = indexes; }
inline void setForcedStops(const std::vector<int>& stops) { g_forcedStops = stops; }

inline bool isStandard(Symbol s) { return s >= HV1 && s <= LV6; }
inline int paytableRow(Symbol s) { return s == SCAT ? 11 : static_cast<int>(s); }

inline void validateWindow(const PayWindow& pay_window) {
    if (pay_window.size() != NO_OF_ROWS) throw std::runtime_error("pay_window row count is not 4");
    for (const auto& row : pay_window) {
        if (row.size() != NO_OF_REELS) throw std::runtime_error("pay_window reel count is not 6");
    }
}

inline int sampleWeighted(const std::vector<int>& weights, std::mt19937& rng) {
    if (!g_forcedWeightedIndexes.empty()) {
        int idx = g_forcedWeightedIndexes.front();
        g_forcedWeightedIndexes.erase(g_forcedWeightedIndexes.begin());
        if (idx < 0 || idx >= static_cast<int>(weights.size())) throw std::runtime_error("forced weighted index out of range");
        return idx;
    }
    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return dist(rng);
}

inline PayWindow generatePayWindow(const ReelSet& ReelSetArg, const std::vector<int>& ReelSize, std::mt19937& rng) {
    Trace trace(__func__);
    trace.input("ReelSet", ReelSetArg);
    trace.input("ReelSize", ReelSize);
    trace.GlobalandStruct("NO_OF_ROWS", NO_OF_ROWS);
    trace.GlobalandStruct("NO_OF_REELS", NO_OF_REELS);

    if (ReelSetArg.size() != NO_OF_REELS || ReelSize.size() != NO_OF_REELS) {
        throw std::runtime_error("ReelSet/ReelSize shape mismatch");
    }
    PayWindow pay_window(NO_OF_ROWS, std::vector<Symbol>(NO_OF_REELS));
    for (int reel = 0; reel < NO_OF_REELS; ++reel) {
        if (static_cast<int>(ReelSetArg[reel].size()) != ReelSize[reel]) {
            throw std::runtime_error("ReelSize does not match reel strip length");
        }
        int stop = 0;
        if (!g_forcedStops.empty()) {
            stop = g_forcedStops.front();
            g_forcedStops.erase(g_forcedStops.begin());
            stop %= ReelSize[reel];
            if (stop < 0) stop += ReelSize[reel];
        } else {
            std::uniform_int_distribution<int> dist(0, ReelSize[reel] - 1);
            stop = dist(rng);
        }
        for (int row = 0; row < NO_OF_ROWS; ++row) {
            pay_window[row][reel] = ReelSetArg[reel][(stop + row) % ReelSize[reel]];
        }
    }
    trace.output("pay_window", pay_window);
    return pay_window;
}

inline PayWindow additionalCOR(PayWindow pay_window, std::mt19937& rng) {
    Trace trace(__func__);
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("probTable.corTrigger", probTable.corTrigger);
    trace.GlobalandStruct("probTable.numCorWeight", probTable.numCorWeight);
    trace.GlobalandStruct("probTable.baseCORValueWeights.values", probTable.baseCORValueWeights.values);
    trace.GlobalandStruct("probTable.baseCORValueWeights.weights", probTable.baseCORValueWeights.weights);
    validateWindow(pay_window);

    int trigger = sampleWeighted(probTable.corTrigger, rng);
    if (trigger == 0) {
        int count = sampleWeighted(probTable.numCorWeight, rng) + 1;
        std::vector<Cell> eligible;
        for (int row = 0; row < NO_OF_ROWS; ++row) {
            for (int reel = 0; reel < NO_OF_REELS; ++reel) {
                Symbol s = pay_window[row][reel];
                if (s != COLLECT && s != WILD && s != SCAT) eligible.push_back({row, reel});
            }
        }
        std::shuffle(eligible.begin(), eligible.end(), rng);
        int placements = std::min(count, static_cast<int>(eligible.size()));
        for (int i = 0; i < placements; ++i) {
            Cell cell = eligible[i];
            pay_window[cell.first][cell.second] = COR;
            int value_index = sampleWeighted(probTable.baseCORValueWeights.weights, rng);
            g_corValues[cell] = probTable.baseCORValueWeights.values[value_index];
        }
    }

    trace.output("pay_window", pay_window);
    return pay_window;
}

inline int countScatterAnywhere(const PayWindow& pay_window) {
    int count = 0;
    for (const auto& row : pay_window) for (Symbol s : row) if (s == SCAT) ++count;
    return count;
}

inline int countScatterLeftToRight(const PayWindow& pay_window) {
    int count = 0;
    for (int reel = 0; reel < NO_OF_REELS; ++reel) {
        int reel_count = 0;
        for (int row = 0; row < NO_OF_ROWS; ++row) if (pay_window[row][reel] == SCAT) ++reel_count;
        if (reel_count == 0) break;
        count += reel_count;
    }
    return std::min(count, NO_OF_REELS);
}

inline WinBreakdown calculateWaysNoTrace(const PayWindow& pay_window) {
    WinBreakdown result;
    for (Symbol sym : StandardSymbols) {
        int symbol_count = 0;
        int ways = 1;
        for (int reel = 0; reel < NO_OF_REELS; ++reel) {
            int matches = 0;
            for (int row = 0; row < NO_OF_ROWS; ++row) {
                Symbol cell = pay_window[row][reel];
                if (cell == sym || cell == WILD) ++matches;
            }
            if (matches == 0) break;
            ways *= matches;
            ++symbol_count;
        }
        result.symbol_hits[sym] = symbol_count;
        int pay = PayTable[paytableRow(sym)][symbol_count];
        if (pay > 0) result.ways_win += pay * ways;
    }
    result.scatter_count = countScatterAnywhere(pay_window);
    int capped_scatter = std::min(result.scatter_count, NO_OF_REELS);
    result.scatter_win = PayTable[paytableRow(SCAT)][capped_scatter];
    return result;
}

inline WinBreakdown waysWinCalculation(const PayWindow& pay_window) {
    Trace trace(__func__);
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("PayTable", PayTable);
    trace.GlobalandStruct("STANDARD_SYMBOL_COUNT", STANDARD_SYMBOL_COUNT);
    validateWindow(pay_window);

    WinBreakdown result = calculateWaysNoTrace(pay_window);
    int win = result.ways_win + result.scatter_win;
    trace.output("win", win);
    return result;
}

inline WinBreakdown waysWinCalculationWithMultiplier(const PayWindow& pay_window, const std::map<Cell, int>& wild_multipliers = {}) {
    Trace trace(__func__);
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("probTable.wildMult", probTable.wildMult);
    trace.GlobalandStruct("PayTable", PayTable);
    validateWindow(pay_window);

    std::map<Cell, int> active_multipliers = wild_multipliers.empty() ? g_wildMultipliers : wild_multipliers;
    static std::mt19937 multiplier_rng(5489u);
    for (int row = 0; row < NO_OF_ROWS; ++row) {
        for (int reel = 0; reel < NO_OF_REELS; ++reel) {
            if (pay_window[row][reel] == WILD && !active_multipliers.count({row, reel})) {
                int idx = sampleWeighted(probTable.wildMult, multiplier_rng);
                active_multipliers[{row, reel}] = idx == 0 ? 2 : idx == 1 ? 3 : 5;
            }
        }
    }

    WinBreakdown result = calculateWaysNoTrace(pay_window);
    int product = 1;
    for (const auto& kv : active_multipliers) {
        int row = kv.first.first;
        int reel = kv.first.second;
        if (row >= 0 && row < NO_OF_ROWS && reel >= 0 && reel < NO_OF_REELS && pay_window[row][reel] == WILD) {
            product *= kv.second;
        }
    }
    result.ways_win *= product;
    if (freeSpinsScatterEvaluationRule == ScatterEvaluationRule::LEFT_TO_RIGHT) {
        result.scatter_count = countScatterLeftToRight(pay_window);
        result.scatter_win = PayTable[paytableRow(SCAT)][std::min(result.scatter_count, NO_OF_REELS)];
    }
    int session_win = result.ways_win + result.scatter_win;
    trace.output("session_win", session_win);
    return result;
}

inline CollectResult resolveCollect(const PayWindow& pay_window, int bet_fixed_coins) {
    Trace trace(__func__);
    trace.input("pay_window", pay_window);
    trace.input("bet_fixed_coins", bet_fixed_coins);
    trace.GlobalandStruct("probTable.CORValues", probTable.CORValues);
    validateWindow(pay_window);

    CollectResult result;
    for (int row = 0; row < NO_OF_ROWS; ++row) {
        if (pay_window[row][0] == COLLECT) ++result.collect_count;
        if (pay_window[row][NO_OF_REELS - 1] == COLLECT) ++result.collect_count;
    }
    for (int row = 0; row < NO_OF_ROWS; ++row) {
        for (int reel = 0; reel < NO_OF_REELS; ++reel) {
            if (pay_window[row][reel] == COR) {
                auto it = g_corValues.find({row, reel});
                if (it != g_corValues.end()) result.cor_value_x_bet_sum += it->second;
            }
        }
    }
    result.collect_win = static_cast<int>(result.collect_count * result.cor_value_x_bet_sum * bet_fixed_coins + 0.00001);
    trace.output("CollectResult", result);
    return result;
}

inline int freeSpinAward(int scatters) {
    if (scatters == 3) return 8;
    if (scatters == 4) return 12;
    if (scatters == 5) return 20;
    if (scatters >= 6) return 30;
    return 0;
}

inline int blitzSpinAward(int scatters) {
    if (scatters == 3) return 3;
    if (scatters == 4) return 4;
    if (scatters == 5) return 6;
    if (scatters >= 6) return 8;
    return 0;
}

inline int freeSpinRetriggerAward(int scatters) {
    if (scatters == 2) return 5;
    return freeSpinAward(scatters);
}

inline FeatureResult FreeGameFeature_1(int num_of_free_spins, std::mt19937& rng) {
    Trace trace(__func__);
    trace.input("num_of_free_spins", num_of_free_spins);
    FeatureResult result;
    int remaining = num_of_free_spins;
    while (remaining > 0) {
        PayWindow pay_window = generatePayWindow(FG1_Reels, FG1_ReelSize, rng);
        WinBreakdown breakdown = waysWinCalculationWithMultiplier(pay_window);
        result.session_win += breakdown.ways_win + breakdown.scatter_win;
        int award = freeSpinRetriggerAward(breakdown.scatter_count);
        if (award > 0) {
            remaining += award;
            result.retrigger_count++;
        }
        remaining--;
        result.spins_played++;
    }
    trace.output("FeatureResult.session_win", result.session_win);
    return result;
}

inline FeatureResult FreeGameFeature_2(int num_of_blitz_spins, std::mt19937& rng) {
    Trace trace(__func__);
    trace.input("num_of_blitz_spins", num_of_blitz_spins);
    trace.GlobalandStruct("probTable.freeGamenumCollect", probTable.freeGamenumCollect);
    trace.GlobalandStruct("probTable.freeGameCOROccur", probTable.freeGameCOROccur);
    trace.GlobalandStruct("probTable.goodCORTable", probTable.goodCORTable);
    trace.GlobalandStruct("probTable.badCORTable", probTable.badCORTable);
    trace.GlobalandStruct("probTable.CORValues", probTable.CORValues);

    FeatureResult result;
    for (int spin = 0; spin < num_of_blitz_spins; ++spin) {
        PayWindow window(NO_OF_ROWS, std::vector<Symbol>(NO_OF_REELS, COR));
        g_corValues.clear();
        int collect_count = sampleWeighted(probTable.freeGamenumCollect, rng) + 1;
        window[0][0] = COLLECT;
        if (collect_count == 2) window[0][NO_OF_REELS - 1] = COLLECT;
        std::vector<Cell> cor_cells;
        for (int row = 0; row < NO_OF_ROWS; ++row) {
            for (int reel = 0; reel < NO_OF_REELS; ++reel) {
                if (window[row][reel] != COLLECT) cor_cells.push_back({row, reel});
            }
        }
        int good_pos = 0;
        for (int i = 0; i < static_cast<int>(cor_cells.size()); ++i) {
            int idx = sampleWeighted(i == good_pos ? probTable.goodCORTable : probTable.badCORTable, rng);
            g_corValues[cor_cells[i]] = probTable.CORValues[idx];
        }
        CollectResult collect = resolveCollect(window, BASE_BET_FIXED_COINS);
        result.session_win += collect.collect_win;
        result.spins_played++;
    }
    trace.output("FeatureResult.session_win", result.session_win);
    return result;
}

inline SpinResult runOneSpin(int bet_fixed_coins, FeatureChoice feature_choice, std::mt19937& rng) {
    Trace trace(__func__);
    trace.input("bet_fixed_coins", bet_fixed_coins);
    trace.input("feature_choice", feature_choice);
    trace.GlobalandStruct("probTable.ReelsetWeights", probTable.ReelsetWeights);

    int reelset = sampleWeighted(probTable.ReelsetWeights, rng);
    PayWindow pay_window = reelset == 0
        ? generatePayWindow(BG1_Reels, BG1_ReelSize, rng)
        : generatePayWindow(BG2_Reels, BG2_ReelSize, rng);
    pay_window = additionalCOR(pay_window, rng);
    WinBreakdown base = waysWinCalculation(pay_window);
    CollectResult collect = resolveCollect(pay_window, bet_fixed_coins);

    SpinResult result;
    result.base_win = base.ways_win;
    result.scatter_win = base.scatter_win;
    result.collect_win = collect.collect_win;
    result.scatter_count = base.scatter_count;
    if (base.scatter_count >= 3) {
        if (feature_choice == FeatureChoice::FREE_SPINS) {
            result.feature_win = FreeGameFeature_1(freeSpinAward(base.scatter_count), rng).session_win;
        } else if (feature_choice == FeatureChoice::BLITZ_SPINS) {
            result.feature_win = FreeGameFeature_2(blitzSpinAward(base.scatter_count), rng).session_win;
        }
    }
    result.total_win = result.base_win + result.scatter_win + result.collect_win + result.feature_win;
    trace.output("SpinResult.total_win", result.total_win);
    return result;
}
