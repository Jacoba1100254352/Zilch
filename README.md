# Zilch
This branch is the SFML/OpenGL visual interface version of Zilch.

The audited CLI version is preserved on the `zilch-cli` branch.

The current implementation keeps the SFML surface while aligning its rule and
option layer with the other Zilch projects: `Checker` is an adapter over
`RuleManager`, `IRule`, `RuleContext`, and `GameOption`.

Prerequisites on macOS:
- `brew install sfml@2 libomp`

Build and run:
- `cmake -S . -B build`
- `cmake --build build`
- `./build/Zilch`

Or use:
- `make`

## Shared gameplay baseline

- Two players and a winning score of 5000.
- Opening Score of 1000.
- Straight, Three Pairs, Multiples and extensions, and Singles enabled.
- One 50-point First-Roll Bust recovery enabled.
- Final Chase and Allow Ties enabled.
- Stealing disabled.

At least one scoring rule must remain enabled before a game can start. The
legacy Bank Threshold API remains available as a compatibility alias for
Opening Score. Opening Score is always capped at the configured Winning Score.

## Intentional C++ differences

This edition remains the local human pass-and-play, SFML/OpenGL presentation of
Zilch. It supports one to six named players, animated visual dice, and an
animation toggle. Computer strategies and large simulation runs belong to the
computer-focused project rather than this GUI.

## Architecture

```mermaid
stateDiagram-v2
    [*] --> Intro
    Intro --> Rules: Rules button
    Rules --> Intro: Back
    Intro --> Setup: Setup button
    Setup --> Intro: Back
    Intro --> PlayerTurn: Start

    state PlayerTurn {
        [*] --> TurnStart
        TurnStart --> StealDecision: prior player banked with 1-5 dice
        TurnStart --> WaitingForRoll: no steal offer
        StealDecision --> Rolled: steal carried score and roll remaining dice
        StealDecision --> Rolled: decline and roll six fresh dice
        WaitingForRoll --> Rolled: Roll Dice
        Rolled --> AwaitingSelection: scoring options exist
        Rolled --> FirstRollBustBonus: no options on first roll
        FirstRollBustBonus --> WaitingForRoll: add 50, reset dice, retry
        Rolled --> BustPending: no options otherwise
        AwaitingSelection --> WaitingForRoll: score option and dice remain
        AwaitingSelection --> WaitingForRoll: hot dice after all dice score
        AwaitingSelection --> CanBank: Opening Score met
        CanBank --> WaitingForRoll: Roll Again
        CanBank --> Banked: Bank Score
        BustPending --> Banked: Pass Dice
        Banked --> [*]
    }

    note right of PlayerTurn
        Rolling, Resolving, and Switching are declared states.
        The current UI dispatch tracks those phases inside PlayerTurn.
    end note

    PlayerTurn --> PlayerTurn: next player
    PlayerTurn --> GameOver: limit reached and final chase off
    PlayerTurn --> PlayerTurn: final chase active and players remain
    PlayerTurn --> GameOver: final chase returns to starter
    GameOver --> [*]
```

```mermaid
classDiagram
    class GameUI {
        +run()
        +renderIntro()
        +renderSetup()
        +renderRules()
        +renderTurn()
        +renderGameOver()
    }
    class GameManager {
        -vector~Player~ players
        -Player* currentPlayer
        -RuleConfig ruleConfig
        -GameStateMachine stateMachine
        +startTurn()
        +registerRoll()
        +endTurnAndBank()
        +acceptStealOffer()
        +declineStealOffer()
        +startFinalRound()
    }
    class GameStateMachine {
        -State current
        +getState()
        +setState()
        +describe()
    }
    class RuleConfig {
        +getOpeningScoreLimit()
        +hasScoringRuleEnabled()
        +firstRollBustBonusEnabled()
        +toggleFirstRollBustBonus()
        +finalChaseEnabled()
        +tiesAllowed()
        +stealingEnabled()
    }
    class Checker {
        -GameManager& game
        +getAvailableOptions()
        +handleBusts()
    }
    class RuleManager {
        -vector~IRule~ rules
        +evaluate()
        +apply()
        +hasOption()
    }
    class IRule {
        +type()
        +evaluate()
    }
    class RuleContext {
        +game
        +player
        +dice
        +score
        +config
    }
    class GameOption {
        +ruleType
        +label
        +selectedValue
        +pointsAwarded
        +consumedDice
    }
    class StraightRule
    class ThreePairsRule
    class MultipleRule
    class AddMultipleRule
    class SingleRule
    class Player {
        -Dice dice
        -Score score
        -string name
    }
    class Dice {
        +diceSetMap
        +rollDice()
        +calculateMultipleAvailability()
    }
    class Score {
        +getPermanentScore()
        +getRoundScore()
        +getScoreFromMultiples()
    }
    class DiceRenderer {
        +startRoll()
        +updateValues()
        +draw()
    }
    class ResourcePath {
        +resolveResourcePath()
    }
    class ObjLoader {
        +loadOBJ()
    }

    GameUI --> GameManager : drives state
    GameUI --> Checker : displays options
    GameUI --> DiceRenderer : renders dice
    GameUI ..> ResourcePath : loads font
    GameManager *-- RuleConfig
    GameManager *-- GameStateMachine
    GameManager "1" *-- "1..6" Player : players
    Player *-- Dice
    Player *-- Score
    Checker --> GameManager : current turn adapter
    Checker --> RuleManager : evaluates/applies options
    RuleManager --> RuleContext : builds
    RuleManager --> GameOption : returns/applies
    RuleManager *-- IRule
    IRule <|.. StraightRule
    IRule <|.. ThreePairsRule
    IRule <|.. MultipleRule
    IRule <|.. AddMultipleRule
    IRule <|.. SingleRule
    RuleContext --> GameManager
    RuleContext --> Player
    RuleContext --> Dice
    RuleContext --> Score
    DiceRenderer ..> ResourcePath : loads model
    DiceRenderer ..> ObjLoader : loads dice.obj
```

First-roll bust behavior:
- When enabled, a first roll with no scoring options awards 50 round points, resets to all six dice, clears the rolled/selected flags, and lets the same player roll again.
- The bonus is tracked per turn and can only be used once; any later bust clears unbanked round points and requires passing dice.
- The behavior is covered by `ZilchRulesTests` through the `firstRollBustBonusIsOneTimeAndRestartsRoll`, `disabledBustBonusBustsImmediately`, and `nonBustHandsDoNotTriggerBustHandling` cases.

Stealing behavior:
- Stealing is an optional setup toggle and is disabled by default.
- When a player banks with one to five dice unrolled, the immediately following player may roll those dice and copy the completed round score as the start of a continuation. The prior player keeps the score they banked.
- The offer is available only to a player whose permanent score already meets the configured Opening Score, so stealing cannot put a player on the board.
- A successful continuation can be banked and offered again with its larger carried score. Choosing a fresh six-die roll, busting, or scoring all available dice ends the chain.
- A bust during a steal loses the carried score and is not rescued by the optional first-roll bust bonus.

Run tests:
- `ctest --test-dir build --output-on-failure`
- `make test`

The `ZilchRulesTests` target covers core dice invariants, structured rule options, scoring combinations, rule toggles, bust handling, banking and stealing gates, steal chaining and termination, final-round and winner state, state-machine descriptions, and resource path resolution.
