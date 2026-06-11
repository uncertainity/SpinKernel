#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../baseline/core.cpp"

struct CoutCapture {
    std::streambuf* old = nullptr;
    std::ostringstream ss;
    CoutCapture() : old(std::cout.rdbuf(ss.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(old); }
    std::string str() const { return ss.str(); }
};

static int g_failures = 0;

static void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++g_failures;
    }
}

static PayWindow window(std::initializer_list<std::initializer_list<Symbol>> rows) {
    PayWindow w;
    for (auto row : rows) w.emplace_back(row);
    validateWindow(w);
    return w;
}

static void testNoWinBaseWindow() {
    clearForcedState();
    PayWindow w = window({
        {HV1, HV2, HV3, HV4, HV5, LV1},
        {LV2, LV3, LV4, LV5, LV6, HV1},
        {HV2, HV3, HV4, HV5, LV1, LV2},
        {LV3, LV4, LV5, LV6, HV1, HV2},
    });
    CoutCapture cap;
    WinBreakdown b = waysWinCalculation(w);
    CollectResult c = resolveCollect(w, BASE_BET_FIXED_COINS);
    require(b.ways_win == 0, "TC-BASE-001 ways_win");
    require(b.scatter_win == 0, "TC-BASE-001 scatter_win");
    require(c.collect_win == 0, "TC-BASE-001 collect_win");
}

static void testStandard2OakWaysWin() {
    clearForcedState();
    PayWindow w = window({
        {HV1, HV1, LV1, LV2, LV3, LV4},
        {LV2, LV3, LV4, LV5, LV6, HV2},
        {HV3, HV4, HV5, LV1, LV2, LV3},
        {LV4, LV5, LV6, HV2, HV3, HV4},
    });
    CoutCapture cap;
    WinBreakdown b = waysWinCalculation(w);
    require(b.symbol_hits[HV1] == 2, "TC-BASE-002 HV1 kind");
    require(b.ways_win == 20, "TC-BASE-002 ways_win");
}

static void testWildSubstitutionWaysWin() {
    clearForcedState();
    PayWindow w = window({
        {HV1, WILD, HV1, LV2, LV3, LV4},
        {LV2, HV1,  LV4, LV5, LV6, HV2},
        {HV3, HV4,  HV5, LV1, LV2, LV3},
        {LV4, LV5,  LV6, HV2, HV3, HV4},
    });
    CoutCapture cap;
    WinBreakdown b = waysWinCalculation(w);
    require(b.symbol_hits[HV1] == 3, "TC-BASE-003 HV1 kind");
    require(b.ways_win == 60, "TC-BASE-003 ways_win");
}

static void testBaseScatterAnywhereFeatureTrigger() {
    clearForcedState();
    PayWindow w = window({
        {SCAT, HV2, HV3, HV4, HV5, LV1},
        {LV2,  LV3, SCAT, LV5, LV6, HV1},
        {HV2,  HV3, HV4, HV5, LV1, SCAT},
        {LV3,  LV4, LV5, LV6, HV1, HV2},
    });
    CoutCapture cap;
    WinBreakdown b = waysWinCalculation(w);
    require(b.scatter_count == 3, "TC-BASE-004 scatter_count");
    require(b.scatter_win == 20, "TC-BASE-004 scatter_win");
    require(freeSpinAward(b.scatter_count) == 8, "TC-BASE-004 free spin award");
    require(blitzSpinAward(b.scatter_count) == 3, "TC-BASE-004 blitz spin award");
}

static void testSingleCollect() {
    clearForcedState();
    PayWindow w = window({
        {COLLECT, HV2, HV3, HV4, HV5, LV1},
        {LV2, COR, LV4, LV5, LV6, HV1},
        {HV2, HV3, HV4, COR, LV1, LV2},
        {LV3, LV4, LV5, LV6, HV1, HV2},
    });
    setForcedCorValue(1, 1, 2);
    setForcedCorValue(2, 3, 10);
    CoutCapture cap;
    CollectResult c = resolveCollect(w, BASE_BET_FIXED_COINS);
    require(c.collect_count == 1, "TC-BASE-005 collect_count");
    require(c.cor_value_x_bet_sum == 12, "TC-BASE-005 cor sum");
    require(c.collect_win == 240, "TC-BASE-005 collect_win");
}

static void testTwoCollects() {
    clearForcedState();
    PayWindow w = window({
        {COLLECT, HV2, HV3, HV4, HV5, LV1},
        {LV2, COR, LV4, LV5, LV6, HV1},
        {HV2, HV3, HV4, COR, LV1, COLLECT},
        {LV3, LV4, LV5, LV6, HV1, HV2},
    });
    setForcedCorValue(1, 1, 2);
    setForcedCorValue(2, 3, 5);
    CoutCapture cap;
    CollectResult c = resolveCollect(w, BASE_BET_FIXED_COINS);
    require(c.collect_count == 2, "TC-BASE-006 collect_count");
    require(c.collect_win == 280, "TC-BASE-006 collect_win");
}

static void testFreeSpinsWildMultiplierProduct() {
    clearForcedState();
    PayWindow w = window({
        {HV1, WILD, LV2, WILD, LV3, LV4},
        {LV2, HV2,  LV4, LV5,  LV6, HV2},
        {HV3, HV4,  HV5, LV1,  LV2, LV3},
        {LV4, LV5,  LV6, HV2,  HV3, HV4},
    });
    setForcedWildMultiplier(0, 1, 2);
    setForcedWildMultiplier(0, 3, 3);
    CoutCapture cap;
    WinBreakdown b = waysWinCalculationWithMultiplier(w);
    require(b.ways_win == 120, "TC-FS-001 multiplied ways_win");
    require(b.scatter_win == 0, "TC-FS-001 scatter unaffected");
}

static void testFreeSpinsRetriggerWith2Scat() {
    clearForcedState();
    PayWindow w = window({
        {SCAT, HV2, HV3, HV4, HV5, LV1},
        {LV2,  LV3, LV4, LV5, LV6, HV1},
        {HV2,  HV3, HV4, HV5, LV1, SCAT},
        {LV3,  LV4, LV5, LV6, HV1, HV2},
    });
    CoutCapture cap;
    WinBreakdown b = waysWinCalculationWithMultiplier(w);
    require(b.scatter_count == 2, "TC-FS-002 scatter_count");
    require(freeSpinRetriggerAward(b.scatter_count) == 5, "TC-FS-002 retrigger award");
    require(b.scatter_win == 0, "TC-FS-002 scatter_win");
}

static void testBlitzCollect() {
    clearForcedState();
    PayWindow w = window({
        {COLLECT, COR, COR, COR, COR, COLLECT},
        {COR,     COR, COR, COR, COR, COR},
        {COR,     COR, COR, COR, COR, COR},
        {COR,     COR, COR, COR, COR, COR},
    });
    const double vals[4][6] = {
        {0, 1, 4, 5, 10, 0},
        {2, 3, 1, 2, 1, 5},
        {1, 2, 3, 4, 5, 6},
        {2, 1, 2, 1, 2, 1},
    };
    for (int r = 0; r < NO_OF_ROWS; ++r) {
        for (int c = 0; c < NO_OF_REELS; ++c) {
            if (w[r][c] == COR) setForcedCorValue(r, c, vals[r][c]);
        }
    }
    CoutCapture cap;
    CollectResult c = resolveCollect(w, BASE_BET_FIXED_COINS);
    require(c.collect_count == 2, "TC-BLITZ-001 collect_count");
    require(c.cor_value_x_bet_sum == 64, "TC-BLITZ-001 cor total");
    require(c.collect_win == 2560, "TC-BLITZ-001 collect_win");
}

static void testAdditionalCorEligibility() {
    clearForcedState();
    PayWindow w = window({
        {COLLECT, WILD, SCAT, HV4, HV5, LV1},
        {LV2,     LV3,  LV4,  LV5, LV6, HV1},
        {HV2,     HV3,  HV4,  HV5, LV1, LV2},
        {LV3,     LV4,  LV5,  LV6, HV1, HV2},
    });
    std::mt19937 rng(7);
    setForcedWeightedIndexes({0, 4, 0, 0, 0, 0, 0});
    CoutCapture cap;
    PayWindow out = additionalCOR(w, rng);
    require(out[0][0] == COLLECT, "TC-BASE-007 preserve COLLECT");
    require(out[0][1] == WILD, "TC-BASE-007 preserve WILD");
    require(out[0][2] == SCAT, "TC-BASE-007 preserve SCAT");
    int added = 0;
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) if (out[r][c] == COR) ++added;
    require(added == 5, "TC-BASE-007 added COR count");
}

static void testGeneratePayWindowForcedStops() {
    clearForcedState();
    std::mt19937 rng(1);
    setForcedStops({0, 0, 0, 0, 0, 0});
    CoutCapture cap;
    PayWindow w = generatePayWindow(BG2_Reels, BG2_ReelSize, rng);
    require(w.size() == 4 && w[0].size() == 6, "generatePayWindow shape");
    require(w[0][0] == BG2_Reels[0][0], "generatePayWindow forced stop reel 1");
    require(w[3][5] == BG2_Reels[5][3], "generatePayWindow forced stop wrap/window");
}

static void testTraceShape() {
    clearForcedState();
    PayWindow w = window({
        {HV1, HV1, LV1, LV2, LV3, LV4},
        {LV2, LV3, LV4, LV5, LV6, HV2},
        {HV3, HV4, HV5, LV1, LV2, LV3},
        {LV4, LV5, LV6, HV2, HV3, HV4},
    });
    CoutCapture cap;
    (void)waysWinCalculation(w);
    std::string t = cap.str();
    require(t.find("->waysWinCalculation") != std::string::npos, "trace enter waysWinCalculation");
    require(t.find("input pay_window") != std::string::npos, "trace input pay_window");
    require(t.find("shape:[4x6]") != std::string::npos, "trace pay_window shape");
    require(t.find("Global/Struct PayTable") != std::string::npos, "trace PayTable global");
    require(t.find("output win") != std::string::npos, "trace output win");
    require(t.find("<- waysWinCalculation") != std::string::npos, "trace exit waysWinCalculation");
}

int main() {
    testNoWinBaseWindow();
    testStandard2OakWaysWin();
    testWildSubstitutionWaysWin();
    testBaseScatterAnywhereFeatureTrigger();
    testSingleCollect();
    testTwoCollects();
    testFreeSpinsWildMultiplierProduct();
    testFreeSpinsRetriggerWith2Scat();
    testBlitzCollect();
    testAdditionalCorEligibility();
    testGeneratePayWindowForcedStops();
    testTraceShape();
    if (g_failures != 0) {
        std::cerr << g_failures << " coder test assertion(s) failed\n";
        return 1;
    }
    std::cout << "All coder tests passed\n";
    return 0;
}
