#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../baseline/core.cpp"

struct CoutCapture {
    std::streambuf* old = nullptr;
    std::ostringstream ss;
    CoutCapture() : old(std::cout.rdbuf(ss.rdbuf())) {}
    ~CoutCapture() { restore(); }
    void restore() {
        if (old != nullptr) {
            std::cout.rdbuf(old);
            old = nullptr;
        }
    }
    std::string str() const { return ss.str(); }
};

static const char* STACK_TRACE_PATH = "generation/tests/stack_traces.txt";

static void emitCapturedTrace(const std::string& label, CoutCapture& cap) {
    const std::string trace = cap.str();
    cap.restore();
    if (trace.empty()) return;

    std::ostringstream section;
    section << "===== " << label << " =====\n" << trace;
    if (!trace.empty() && trace.back() != '\n') section << "\n";
    section << "\n";

    std::cerr << section.str();
    std::ofstream out(STACK_TRACE_PATH, std::ios::app);
    out << section.str();
}

struct CaseResult {
    std::string id;
    std::string category;
    std::string status = "EXECUTED";
    std::string observed_json;
    std::vector<std::string> errors;
};

static std::string jsonEscape(const std::string& in) {
    std::ostringstream out;
    for (char ch : in) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch; break;
        }
    }
    return out.str();
}

static std::string symbolName(Symbol s) {
    switch (s) {
        case HV1: return "HV1";
        case HV2: return "HV2";
        case HV3: return "HV3";
        case HV4: return "HV4";
        case HV5: return "HV5";
        case LV1: return "LV1";
        case LV2: return "LV2";
        case LV3: return "LV3";
        case LV4: return "LV4";
        case LV5: return "LV5";
        case LV6: return "LV6";
        case WILD: return "WILD";
        case SCAT: return "SCAT";
        case COR: return "COR";
        case COLLECT: return "COLLECT";
    }
    return "UNKNOWN";
}

static PayWindow window(std::initializer_list<std::initializer_list<Symbol>> rows) {
    PayWindow w;
    for (auto row : rows) w.emplace_back(row);
    validateWindow(w);
    return w;
}

static std::string windowJson(const PayWindow& w) {
    std::ostringstream out;
    out << "[";
    for (size_t r = 0; r < w.size(); ++r) {
        if (r) out << ",";
        out << "[";
        for (size_t c = 0; c < w[r].size(); ++c) {
            if (c) out << ",";
            out << "\"" << symbolName(w[r][c]) << "\"";
        }
        out << "]";
    }
    out << "]";
    return out.str();
}

static bool hasShape4x6(const PayWindow& w) {
    if (w.size() != NO_OF_ROWS) return false;
    for (const auto& row : w) {
        if (row.size() != NO_OF_REELS) return false;
    }
    return true;
}

static int countSymbol(const PayWindow& w, Symbol s) {
    int count = 0;
    for (const auto& row : w) {
        for (Symbol cell : row) {
            if (cell == s) ++count;
        }
    }
    return count;
}

static bool noNegativeWinBreakdown(const WinBreakdown& b) {
    return b.ways_win >= 0 && b.scatter_win >= 0 && b.scatter_count >= 0;
}

static bool noNegativeCollect(const CollectResult& c) {
    return c.collect_count >= 0 && c.cor_value_x_bet_sum >= 0.0 && c.collect_win >= 0;
}

static bool noNegativeFeature(const FeatureResult& f) {
    return f.session_win >= 0 && f.spins_played >= 0 && f.retrigger_count >= 0;
}

static CaseResult runCase(const std::string& id, const std::string& category, const std::function<std::string()>& fn) {
    CaseResult result;
    result.id = id;
    result.category = category;
    CoutCapture capture;
    try {
        result.observed_json = fn();
    } catch (const std::exception& ex) {
        result.status = "ERROR";
        result.observed_json = "{}";
        result.errors.push_back(ex.what());
    } catch (...) {
        result.status = "ERROR";
        result.observed_json = "{}";
        result.errors.push_back("unknown exception");
    }
    emitCapturedTrace("forced_window " + id, capture);
    return result;
}

static std::string winBreakdownJson(const WinBreakdown& b) {
    std::ostringstream out;
    out << "\"ways_win\":" << b.ways_win
        << ",\"scatter_win\":" << b.scatter_win
        << ",\"scatter_count\":" << b.scatter_count;
    return out.str();
}

static std::string collectJson(const CollectResult& c) {
    std::ostringstream out;
    out << "\"collect_count\":" << c.collect_count
        << ",\"cor_value_x_bet_sum\":" << c.cor_value_x_bet_sum
        << ",\"collect_win\":" << c.collect_win;
    return out.str();
}

static std::string featureJson(const FeatureResult& f) {
    std::ostringstream out;
    out << "\"session_win\":" << f.session_win
        << ",\"spins_played\":" << f.spins_played
        << ",\"retrigger_count\":" << f.retrigger_count;
    return out.str();
}

int main() {
    std::vector<CaseResult> results;

    results.push_back(runCase("CASE_EDGE_001", "BASE_SCATTER_BOUNDARY", [] {
        clearForcedState();
        PayWindow w = window({
            {SCAT, HV2, HV3, HV4, HV5, LV1},
            {LV2,  LV3, LV4, LV5, LV6, HV1},
            {HV2,  HV3, HV4, HV5, LV1, SCAT},
            {LV3,  LV4, LV5, LV6, HV1, HV2},
        });
        WinBreakdown b = waysWinCalculation(w);
        std::ostringstream out;
        out << "{" << winBreakdownJson(b)
            << ",\"free_spin_award\":" << freeSpinAward(b.scatter_count)
            << ",\"blitz_spin_award\":" << blitzSpinAward(b.scatter_count)
            << ",\"window_shape_valid\":" << (hasShape4x6(w) ? "true" : "false")
            << ",\"non_negative_fields\":" << (noNegativeWinBreakdown(b) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_002", "BASE_SCATTER_BOUNDARY", [] {
        clearForcedState();
        PayWindow w = window({
            {SCAT, HV2, HV3, HV4, HV5, LV1},
            {LV2,  SCAT, LV4, LV5, LV6, HV1},
            {HV2,  HV3, HV4, HV5, LV1, SCAT},
            {LV3,  LV4, LV5, SCAT, HV1, HV2},
        });
        WinBreakdown b = waysWinCalculation(w);
        std::ostringstream out;
        out << "{" << winBreakdownJson(b)
            << ",\"free_spin_award\":" << freeSpinAward(b.scatter_count)
            << ",\"blitz_spin_award\":" << blitzSpinAward(b.scatter_count)
            << ",\"window_shape_valid\":" << (hasShape4x6(w) ? "true" : "false")
            << ",\"non_negative_fields\":" << (noNegativeWinBreakdown(b) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_003", "FREE_SPINS_SCATTER_RULE_CONFIG", [] {
        clearForcedState();
        PayWindow w = window({
            {HV1, SCAT, HV3, HV4, HV5, LV1},
            {LV2, LV3,  LV4, LV5, LV6, HV1},
            {HV2, HV3,  HV4, HV5, LV1, SCAT},
            {LV3, LV4,  LV5, SCAT, HV1, HV2},
        });
        freeSpinsScatterEvaluationRule = ScatterEvaluationRule::ANYWHERE;
        WinBreakdown anywhere = waysWinCalculationWithMultiplier(w, {});
        freeSpinsScatterEvaluationRule = ScatterEvaluationRule::LEFT_TO_RIGHT;
        WinBreakdown ltr = waysWinCalculationWithMultiplier(w, {});
        freeSpinsScatterEvaluationRule = ScatterEvaluationRule::ANYWHERE;
        std::ostringstream out;
        out << "{\"anywhere\":{" << winBreakdownJson(anywhere)
            << "},\"left_to_right\":{" << winBreakdownJson(ltr)
            << "},\"window_shape_valid\":" << (hasShape4x6(w) ? "true" : "false")
            << ",\"non_negative_fields\":" << ((noNegativeWinBreakdown(anywhere) && noNegativeWinBreakdown(ltr)) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_004", "FREE_SPINS_MULTIPLIER_RESET", [] {
        clearForcedState();
        PayWindow prior = window({
            {HV1, WILD, LV1, LV2, LV3, LV4},
            {LV2, LV3,  LV4, LV5, LV6, HV2},
            {HV3, HV4,  HV5, LV1, LV2, LV3},
            {LV4, LV5,  LV6, HV2, HV3, HV4},
        });
        setForcedWildMultiplier(0, 1, 5);
        WinBreakdown with_forced = waysWinCalculationWithMultiplier(prior);
        clearForcedState();
        PayWindow after = window({
            {HV1, HV1, LV1, LV2, LV3, LV4},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {HV3, HV4, HV5, LV1, LV2, LV3},
            {LV4, LV5, LV6, HV2, HV3, HV4},
        });
        WinBreakdown reset = waysWinCalculationWithMultiplier(after);
        std::ostringstream out;
        out << "{\"prior_forced\":{" << winBreakdownJson(with_forced)
            << "},\"after_clear\":{" << winBreakdownJson(reset)
            << "},\"after_visible_wild_count\":" << countSymbol(after, WILD)
            << ",\"forced_multiplier_state_size\":" << g_wildMultipliers.size()
            << ",\"window_shape_valid\":" << (hasShape4x6(after) ? "true" : "false")
            << ",\"non_negative_fields\":" << ((noNegativeWinBreakdown(with_forced) && noNegativeWinBreakdown(reset)) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_005", "COLLECT_REEL_RESTRICTION", [] {
        clearForcedState();
        PayWindow w = window({
            {HV1, COLLECT, HV3, HV4, HV5, LV1},
            {LV2, COR,     LV4, LV5, LV6, HV1},
            {HV2, HV3,     HV4, COR, LV1, LV2},
            {LV3, LV4,     LV5, LV6, HV1, HV2},
        });
        setForcedCorValue(1, 1, 2);
        setForcedCorValue(2, 3, 10);
        CollectResult c = resolveCollect(w, BASE_BET_FIXED_COINS);
        std::ostringstream out;
        out << "{" << collectJson(c)
            << ",\"window_shape_valid\":" << (hasShape4x6(w) ? "true" : "false")
            << ",\"non_negative_fields\":" << (noNegativeCollect(c) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_006", "ADDITIONAL_COR_NO_ELIGIBLE_CELLS", [] {
        clearForcedState();
        PayWindow w = window({
            {COLLECT, WILD, SCAT, WILD, SCAT, COLLECT},
            {SCAT,    WILD, SCAT, WILD, SCAT, WILD},
            {WILD,    SCAT, WILD, SCAT, WILD, SCAT},
            {COLLECT, WILD, SCAT, WILD, SCAT, COLLECT},
        });
        setForcedWeightedIndexes({0, 4});
        std::mt19937 rng(19);
        PayWindow out_window = additionalCOR(w, rng);
        std::ostringstream out;
        out << "{\"input_cor_count\":" << countSymbol(w, COR)
            << ",\"output_cor_count\":" << countSymbol(out_window, COR)
            << ",\"forced_cor_value_count\":" << g_corValues.size()
            << ",\"window_shape_valid\":" << (hasShape4x6(out_window) ? "true" : "false")
            << ",\"output_window\":" << windowJson(out_window)
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_007", "GENERATE_PAY_WINDOW_FORCED_STOP_WRAP", [] {
        clearForcedState();
        std::mt19937 rng(23);
        setForcedStops({-1, 25, 26, 27, 54, -28});
        PayWindow w = generatePayWindow(BG2_Reels, BG2_ReelSize, rng);
        std::ostringstream out;
        out << "{\"window_shape_valid\":" << (hasShape4x6(w) ? "true" : "false")
            << ",\"generated_window\":" << windowJson(w)
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_008", "RUN_ONE_SPIN_NO_FEATURE_FORCED_PATH", [] {
        clearForcedState();
        std::mt19937 rng(31);
        setForcedWeightedIndexes({1, 1});
        setForcedStops({0, 0, 0, 0, 0, 0});
        SpinResult s = runOneSpin(BASE_BET_FIXED_COINS, FeatureChoice::NONE, rng);
        bool sum_ok = s.total_win == s.base_win + s.scatter_win + s.collect_win + s.feature_win;
        bool non_negative = s.base_win >= 0 && s.scatter_win >= 0 && s.collect_win >= 0 &&
            s.feature_win >= 0 && s.total_win >= 0 && s.scatter_count >= 0;
        std::ostringstream out;
        out << "{\"base_win\":" << s.base_win
            << ",\"scatter_win\":" << s.scatter_win
            << ",\"collect_win\":" << s.collect_win
            << ",\"feature_win\":" << s.feature_win
            << ",\"total_win\":" << s.total_win
            << ",\"scatter_count\":" << s.scatter_count
            << ",\"total_equals_components\":" << (sum_ok ? "true" : "false")
            << ",\"non_negative_fields\":" << (non_negative ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_009", "FREE_GAME_FEATURE_SMOKE", [] {
        clearForcedState();
        std::mt19937 rng(37);
        setForcedStops({0, 0, 0, 0, 0, 0});
        FeatureResult f = FreeGameFeature_1(1, rng);
        std::ostringstream out;
        out << "{" << featureJson(f)
            << ",\"non_negative_fields\":" << (noNegativeFeature(f) ? "true" : "false")
            << "}";
        return out.str();
    }));

    results.push_back(runCase("CASE_EDGE_010", "BLITZ_FEATURE_SMOKE", [] {
        clearForcedState();
        std::vector<int> forced;
        forced.push_back(1);
        forced.push_back(7);
        for (int i = 0; i < 21; ++i) forced.push_back(0);
        setForcedWeightedIndexes(forced);
        std::mt19937 rng(41);
        FeatureResult f = FreeGameFeature_2(1, rng);
        std::ostringstream out;
        out << "{" << featureJson(f)
            << ",\"remaining_forced_weighted_indexes\":" << g_forcedWeightedIndexes.size()
            << ",\"observed_cor_value_count\":" << g_corValues.size()
            << ",\"non_negative_fields\":" << (noNegativeFeature(f) ? "true" : "false")
            << "}";
        return out.str();
    }));

    std::cout << "{\n  \"cases\": [\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const CaseResult& r = results[i];
        if (i) std::cout << ",\n";
        std::cout << "    {\n"
                  << "      \"case_id\": \"" << r.id << "\",\n"
                  << "      \"category\": \"" << r.category << "\",\n"
                  << "      \"status\": \"" << r.status << "\",\n"
                  << "      \"observed_result\": " << r.observed_json << ",\n"
                  << "      \"errors\": [";
        for (size_t e = 0; e < r.errors.size(); ++e) {
            if (e) std::cout << ", ";
            std::cout << "\"" << jsonEscape(r.errors[e]) << "\"";
        }
        std::cout << "]\n    }";
    }
    std::cout << "\n  ]\n}\n";
    return 0;
}
