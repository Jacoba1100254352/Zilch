#ifndef ZILCH_INPUT_CLOSED_H
#define ZILCH_INPUT_CLOSED_H

#include <stdexcept>

class InputClosed final : public std::runtime_error {
public:
    InputClosed() : std::runtime_error("Input stream closed") {}
};

#endif //ZILCH_INPUT_CLOSED_H
