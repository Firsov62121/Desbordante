#pragma once

#include "structures/position_list_index.h"

class OrderedPartition {
private:
    double distinctiveness_;
    unsigned int column_index_;
public:
    OrderedPartition() = delete;
    OrderedPartition(OrderedPartition const& other) = delete;
    OrderedPartition(OrderedPartition&& other) = delete;
    OrderedPartition& operator=(OrderedPartition const& other) = delete;
    OrderedPartition& operator=(OrderedPartition&& other) = delete;

    OrderedPartition(structures::PositionListIndex const* const, unsigned int number_of_rows,
                     unsigned int column_index);

    bool operator<(OrderedPartition const& other) const;
    unsigned int GetColumnIndex() const { return this->column_index_; }
};