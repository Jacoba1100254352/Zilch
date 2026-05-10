# Zilch
This branch is the SFML/OpenGL visual interface version of Zilch.

The audited CLI version is preserved on the `zilch-cli` branch.

Prerequisites on macOS:
- `brew install sfml@2 libomp`

Build and run:
- `cmake -S . -B build`
- `cmake --build build`
- `./build/Zilch`

Or use:
- `make`

Architecture:

```mermaid
stateDiagram-v2
    [*] --> Intro
    Intro --> Rules: Rules button
    Rules --> Intro: Back
    Intro --> Setup: Setup button
    Setup --> Intro: Back
    Intro --> PlayerTurn: Start

    state PlayerTurn {
        [*] --> WaitingForRoll
        WaitingForRoll --> Rolled: Roll Dice
        Rolled --> AwaitingSelection: scoring options exist
        Rolled --> FirstRollBustBonus: no options on first roll
        FirstRollBustBonus --> WaitingForRoll: add 50, reset dice, retry
        Rolled --> BustPending: no options otherwise
        AwaitingSelection --> WaitingForRoll: score option and dice remain
        AwaitingSelection --> WaitingForRoll: hot dice after all dice score
        AwaitingSelection --> CanBank: threshold met
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
        +startFinalRound()
    }
    class GameStateMachine {
        -State current
        +getState()
        +setState()
        +describe()
    }
    class RuleConfig {
        +firstRollBustBonusEnabled()
        +toggleFirstRollBustBonus()
        +finalChaseEnabled()
        +tiesAllowed()
    }
    class Checker {
        -GameManager& game
        +getAvailableOptions()
        +handleBusts()
    }
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
    Checker --> GameManager : evaluates current turn
    Checker ..> Dice : reads dice map
    Checker ..> Score : applies scoring
    DiceRenderer ..> ResourcePath : loads model
    DiceRenderer ..> ObjLoader : loads dice.obj
```

First-roll bust behavior:
- When enabled, a first roll with no scoring options awards 50 round points, resets to all six dice, clears the rolled/selected flags, and lets the same player roll again.
- The bonus is tracked per turn and can only be used once; any later bust clears unbanked round points and requires passing dice.
- The behavior is covered by `ZilchRulesTests` through the `firstRollBustBonusIsOneTimeAndRestartsRoll`, `disabledBustBonusBustsImmediately`, and `nonBustHandsDoNotTriggerBustHandling` cases.

Run tests:
- `ctest --test-dir build --output-on-failure`
- `make test`

The `ZilchRulesTests` target covers core dice invariants, scoring combinations, rule toggles, bust handling, banking gates, final-round and winner state, state-machine descriptions, and resource path resolution.
