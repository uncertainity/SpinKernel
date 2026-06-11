# Vegas Blitz Game Flow Plan

## Game Overview

Vegas Blitz is a 6 reel x 4 row video slot with 4096 ways in the base game and Free Spins. Each base game costs 20 fixed coins. The documented long-term expected payback is 94.11% in the rules document; the math workbook lists game RTP rows of 94.10764112986% for the Blitz-feature path and 94.10689660827% for the Free-Spins path.

Major systems:

- Base 4096-ways wins, paid leftmost reel to right in consecutive order.
- Scatter-triggered feature selection at 3 or more SCAT in the base game.
- Free Spins with WILD multipliers and retriggers.
- Blitz Spins with guaranteed COLLECT and COR symbols.
- Collect feature in base game and Blitz Spins only.
- Jackpot values carried by COR symbols and collected only through the Collect feature.

## Symbol Inventory

Normalized symbols are used downstream:

| Normalized | Source symbol |
|---|---|
| HV1 | PIC1 |
| HV2 | PIC2 |
| HV3 | PIC3 |
| HV4 | PIC4 |
| HV5 | PIC5 |
| LV1 | ACE |
| LV2 | KING |
| LV3 | QUEEN |
| LV4 | JACK |
| LV5 | TEN |
| LV6 | NINE |
| WILD | WILD |
| SCAT | SCAT |
| COR | COIN |
| COLLECT | COLLECT |

WILD substitutes for standard symbols only. It does not substitute for SCAT, COR, or COLLECT. WILD appears only on reels 2-6 in the base game and reels 2-4 in Free Spins. In Free Spins, each WILD carries a 2x, 3x, or 5x multiplier.

SCAT triggers the feature selection screen in the base game at 3 or more symbols. SCAT also has a paytable entry. The source documents conflict on whether Free Spins SCAT is evaluated anywhere or left-to-right; see Ambiguities.

COR carries a cash value or jackpot value. COR values are multiplied by total bet when collected. COLLECT appears only on reels 1 and/or 6 in the base game and Blitz Spins. Each COLLECT independently collects all visible COR and jackpot values.

Jackpots are fixed xBet values: Mini 10x, Minor 25x, Major 250x, Grand 2000x.

## Bet Setup

- Base game cost: 20 fixed coins.
- Pay mechanism: 4096 ways.
- Coin denomination: missing.
- Configurable bet ladder beyond the 20 coin cost: missing.
- Feature buy is present in the math workbook: 4 SCAT costs 80x game bet, 5 SCAT costs 95x, 6 SCAT costs 110x. Player-facing availability and jurisdictional enablement are not specified.

## Paytable

Values are expressed in the source paytable units.

| Symbol | 2 | 3 | 4 | 5 | 6 |
|---|---:|---:|---:|---:|---:|
| HV1 | 20 | 30 | 40 | 50 | 100 |
| HV2 | 10 | 20 | 30 | 40 | 50 |
| HV3 | 10 | 20 | 30 | 40 | 50 |
| HV4 | 4 | 16 | 24 | 30 | 40 |
| HV5 | 4 | 16 | 24 | 30 | 40 |
| LV1 |  | 10 | 20 | 24 | 30 |
| LV2 |  | 10 | 20 | 24 | 30 |
| LV3 |  | 8 | 16 | 20 | 24 |
| LV4 |  | 8 | 16 | 20 | 24 |
| LV5 |  | 6 | 10 | 16 | 20 |
| LV6 |  | 6 | 10 | 16 | 20 |
| SCAT |  | 20 | 40 | 60 | 100 |

## Base Game Flow

1. Player places the fixed 20 coin wager.
2. Select base reelset B1 or B2 using weights B1=58 and B2=42.
3. Generate a 6x4 visible window.
4. Resolve additional COR trigger with Yes=1 and No=9 weights.
5. If triggered, add 1 to 5 COR symbols using weights 240, 200, 150, 100, 80. Additional COR may be placed anywhere except on COLLECT, WILD, or SCAT.
6. Evaluate left-to-right 4096 ways wins and SCAT wins.
7. Resolve Collect: if one or more COLLECT symbols appear on reel 1 and/or reel 6, each COLLECT independently collects all visible COR and jackpot values.
8. Multiply collected COR and jackpot xBet values by total bet.
9. If 3 or more SCAT appear, show feature selection.
10. Accumulate/display line wins, scatter wins, and COR wins.

## Feature Flow

### Free Spins

Base-game SCAT awards:

| SCAT count | Free Spins |
|---:|---:|
| 3 | 8 |
| 4 | 12 |
| 5 | 20 |
| 6 | 30 |

Retriggers in Free Spins:

| SCAT count | Additional Free Spins |
|---:|---:|
| 2 | 5 |
| 3 | 8 |
| 4 | 12 |
| 5 | 20 |
| 6 | 30 |

Natural Free Spins use reelset F1. Feature-buy Free Spins use BB F2, BB F3, or BB F4 depending on 4, 5, or 6 SCAT. COR and COLLECT do not appear. WILD multipliers use weights 2x=480, 3x=545, 5x=360. All WILD multipliers on the window multiply together and apply to ways wins, regardless of whether each WILD participates. WILD multipliers do not apply to SCAT wins.

### Blitz Spins

Base-game SCAT awards:

| SCAT count | Blitz Spins |
|---:|---:|
| 3 | 3 |
| 4 | 4 |
| 5 | 6 |
| 6 | 8 |

Only COR, jackpot-valued COR, and COLLECT symbols appear. Each Blitz Spin guarantees at least one COLLECT and at least one COR. The number of COLLECT symbols uses weights 1=75 and 2=25. Each eligible non-COLLECT position checks COR occurrence using Yes=26 and No=74 weights. If no COR appears, one COR is forced. Normal Blitz Spins use exactly one Good-table COR per spin and Bad-table weights for all other CORs. There are no retriggers.

### Collect

Collect is active only in the base game and Blitz Spins. Each COLLECT independently collects all visible COR and jackpot values. Jackpot can be won only through Collect.

## State Transitions

```text
Idle
-> BetPlaced
-> BaseWindowGenerated
-> AdditionalCORResolved
-> BaseWinsEvaluated
-> CollectResolved
-> FeatureSelection, if 3+ base SCAT
-> FreeSpinsActive or BlitzSpinsActive, based on player choice
-> FeatureResolved
-> PayoutComplete
-> Idle
```

If fewer than 3 base SCAT land, `CollectResolved` transitions directly to `PayoutComplete`.

## Win Evaluation Logic

- Standard symbols pay left-to-right on consecutive reels under 4096 ways.
- WILD substitutes for standard symbols only.
- SCAT has a paytable and triggers features at 3 or more in the base game.
- Base implementation order is ways/scatter evaluation, then Collect resolution, then feature trigger check.
- Free Spins apply the product of visible WILD multipliers to ways wins only.
- Collect wins are accumulated separately from line/ways and SCAT wins.

## Mathematical Dependencies

- Paytable values.
- Base reel strips B1 and B2 and their selection weights.
- Free Spins reel strips F1 and buy-bonus Free Spins reel strips BB F2/F3/F4.
- Additional COR trigger, count, placement restrictions, and value weights.
- COLLECT reel restrictions and independent collection behavior.
- Blitz Spins COLLECT count weights, COR occurrence weights, forcing condition, and Good/Bad value tables.
- Feature selection behavior after 3+ base SCAT.
- Free Spins retrigger rules.
- Feature-buy costs and special reel/value tables if feature buy is implemented.

Exact normalized reel-strip arrays are included in `game_flow.json` under `reel_strips` and `buy_bonus_reel_strips`.

## Assumptions

- ASSUMPTION A1: 4096 ways means 4 visible positions on each of 6 reels, yielding 4^6 ways. Reason: the rule sheets describe a 6x4 window and 4096 ways.
- ASSUMPTION A2: Normal base-game feature trigger gives the player a choice between Free Spins and Blitz Spins. Reason: the rule document states a feature selection screen with two options.
- ASSUMPTION A3: Feature-buy rows are math-model-supported but player-facing availability is not confirmed. Reason: Buy Bonus sheets and costs exist; rules document does not specify jurisdictional availability.

## Ambiguities

- [AMBIGUOUS] Scatter evaluation conflicts. Game rules say SCAT pays anywhere; the Implementation sheet says Free Spins calculate left-to-right Scatter wins.
- [AMBIGUOUS] The phrase "Each symbol can be used once per winning combination" is not formalized for duplicate counting in 4096 ways.
- [AMBIGUOUS] Buy-bonus Free Spins sheet says ways wins are multiplied by all WILDs "till that reel," while general Free Spins rules say all window WILD multipliers apply regardless of participation.

## Missing Information

- [MISSING] Coin denomination and configurable bet ladder beyond fixed 20 coin game cost.
- [MISSING] Player-facing feature-buy availability and jurisdictional enablement rules.
- [MISSING] Formal duplicate-counting rule for 4096 ways phrase: each symbol can be used once per winning combination.
