#ifndef ZILCH_RULES_IRULE_H
#define ZILCH_RULES_IRULE_H

#include <vector>

#include "rules/GameOption.h"
#include "rules/RuleContext.h"

namespace zilch::rules {

class IRule {
public:
    virtual ~IRule() = default;

    [[nodiscard]] virtual RuleType type() const = 0;
    [[nodiscard]] virtual std::vector<GameOption> evaluate(const RuleContext& context) const = 0;
};

}

#endif // ZILCH_RULES_IRULE_H
