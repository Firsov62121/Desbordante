#include <stdexcept>
#include <sstream>
#include <utility>

#include "attribute_pair.h"
#include "single_attribute_predicate.h"

using namespace algos::fastod;

AttributePair::AttributePair(const SingleAttributePredicate& left, size_t right) noexcept : pair_(std::move(std::make_pair(left, right))) {
    if (left.GetAttribute() == right) {
        throw std::invalid_argument("Two attributes cannot be the same");
    }
}

const SingleAttributePredicate& AttributePair::GetLeft() const noexcept {
    return pair_.first;
}

size_t AttributePair::GetRight() const noexcept {
    return pair_.second;
}

std::string AttributePair::ToString() const {
    std::stringstream ss;
    ss << "{ " << pair_.first.ToString() << ", " << pair_.second + 1 << " }";
    return ss.str();
}

namespace algos::fastod {

bool operator==(const AttributePair& x, const AttributePair& y) {
    return x.pair_ == y.pair_;
}

} // namespace algos::fastod
