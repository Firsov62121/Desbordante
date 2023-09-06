#include <cstdint>

#include "stripped_partition.h"
#include "cache_with_limit.h"

namespace algos::fastod {

StrippedPartition::StrippedPartition(const DataFrame& data) : data_(std::move(data)) {}

std::string StrippedPartition::ToString() const {
    std::stringstream ss;
    if (!pli_)
        return "[]";
    return pli_->ToString();
}

StrippedPartition& StrippedPartition::operator=(const StrippedPartition& other) {
    if (this == &other) {
        return *this;
    }
    this->pli_ = other.pli_;
    return *this;
}

void StrippedPartition::Product(size_t attribute) {
    if (!pli_) {
        pli_ = data_.GetColumnSP(attribute);
        return;
    }
    pli_ = pli_->Intersect(data_.GetColumnSP(attribute).get());
}

bool StrippedPartition::Split(size_t right) {
    if (!pli_) {
        int group_value = data_.GetValue(0, right);
        for (size_t i = 1; i < data_.GetTupleCount(); ++i) {
            if (data_.GetValue(i, right) != group_value)
                return true;
        }
        return false;
    }
    for (const auto& cluster : pli_->GetIndex()) {
        int group_value = data_.GetValue(cluster[0], right);
        for (size_t i = 1; i < cluster.size(); ++i) {
            if (data_.GetValue(cluster[i], right) != group_value)
                return true;
        }
    }

    return false;
}

bool StrippedPartition::Swap(const SingleAttributePredicate& left, size_t right) {
    auto checkValues = [&left, right](std::vector<std::pair<int, int>>& values) {
        if (left.ascending)
            std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                return p1.first < p2.first;
            });
        else
            std::sort(values.begin(), values.end(), [](const auto& p1, const auto& p2) {
                return p2.first < p1.first;
            });

        size_t prev_group_max_index = 0;
        size_t current_group_max_index = 0;
        bool is_first_group = true;
        
        
        for (size_t i = 0; i < values.size(); i++) {
            const auto& first = values[i].first;
            const auto& second = values[i].second;

            // values are sorted by "first"
            if (i != 0 && values[i - 1].first != first) {
                is_first_group = false;
                prev_group_max_index = current_group_max_index;
                current_group_max_index = i;
            } else if (values[current_group_max_index].second <= second) {
                current_group_max_index = i;
            }

            if (!is_first_group && values[prev_group_max_index].second > second) {
                return true;
            }
        }
        return false;
    };
    if (!pli_) {
        std::vector<std::pair<int, int>> values(data_.GetTupleCount());
        for (size_t i = 0; i < data_.GetTupleCount(); ++i) {
            values[i] = { data_.GetValue(i, left.attribute), 
                          data_.GetValue(i, right) };
        }
        return checkValues(values);
    }
    for (const auto& cluster : pli_->GetIndex()) {
        std::vector<std::pair<int, int>> values(cluster.size());
        for (size_t i = 0; i < cluster.size(); ++i) {
            values[i] = { data_.GetValue(cluster[i], left.attribute), 
                          data_.GetValue(cluster[i], right) };
        }

        if (checkValues(values))
            return true;
    }

    return false;
}

} // namespace algos::fastod
