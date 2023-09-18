#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include "attribute_set.h"
#include "canonical_od.h"
#include "attribute_pair.h"
#include "timer.h"
#include "stripped_partition_cache.h"

namespace algos::fastod {

class Fastod {
private:
    const long time_limit_;
    bool is_complete_ = true;
    size_t level_;
    size_t od_count_ = 0;
    size_t fd_count_ = 0;
    size_t ocd_count_ = 0;
    std::vector<CanonicalOD<true>> result_asc_;
    std::vector<CanonicalOD<false>> result_desc_;
    std::vector<SimpleCanonicalOD> result_simple_;
    std::vector<std::unordered_set<AttributeSet>> context_in_each_level_;
    std::unordered_map<AttributeSet, AttributeSet> cc_;
    std::unordered_map<AttributeSet, std::unordered_set<AttributePair>> cs_asc_;
    std::unordered_map<AttributeSet, std::unordered_set<AttributePair>> cs_desc_;
    StrippedPartitionCache partition_cache_;

    AttributeSet schema_;
    DataFrame data_;

    Timer timer_;

    template <bool ascending>
    void addToRes(CanonicalOD<ascending>&& od) {
        if constexpr (ascending)
            result_asc_.emplace_back(std::move(od));
        else
            result_desc_.emplace_back(std::move(od));
    }

    void addToRes(SimpleCanonicalOD&& od) {
        result_simple_.emplace_back(std::move(od));
    }

    bool IsTimeUp() const;
    
    void CCPut(AttributeSet const& key, AttributeSet attribute_set);
    AttributeSet const& CCGet(AttributeSet const& key);
    
    template <bool ascending>
    void CSPut(AttributeSet const& key, const AttributePair& value) {
        if constexpr (ascending)
            cs_asc_[key].emplace(value);
        else
            cs_desc_[key].emplace(value);
    }

    template <bool ascending>
    void CSPut(AttributeSet const& key, AttributePair&& value) {
        if constexpr (ascending)
            cs_asc_[key].emplace(std::move(value));
        else
            cs_desc_[key].emplace(std::move(value));
    }

    template <bool ascending>
    std::unordered_set<AttributePair>& CSGet(AttributeSet const& key) {
        if constexpr (ascending)
            return cs_asc_[key];
        else
            return cs_desc_[key];
    }

    void Initialize();

    void CalculateAllLevels();
    void ComputeODs(std::size_t level_from, std::size_t level_to);
    void ComputeODsThread();
    // void Compute1();
    // void Compute2();

    void ComputeODs(std::size_t level);
    void PruneLevels(std::size_t level);
    void CalculateNextLevel(std::size_t current_level);

    template <bool ascending>
    void AddCandidates(AttributeSet const& context,
                       std::vector<AttributeSet> const& delAttrs, std::size_t current_level) {
        if (current_level == 2) {
            for (AttributeSet::size_type i = 0; i < data_.GetColumnCount(); i++) {
                for (AttributeSet::size_type j = 0; j < data_.GetColumnCount(); j++) {
                    if (i == j)
                        continue;
                    CSPut<ascending>(attributeSet({i, j}, data_.GetColumnCount()), AttributePair(i, j));
                }
            }
        } else if (current_level > 2) {
            std::unordered_set<AttributePair> candidates;
            for (AttributeSet::size_type attr = context.find_first(); attr != AttributeSet::npos;
                 attr = context.find_next(attr)) {
                const auto& tmp = CSGet<ascending>(delAttrs[attr]);
                candidates.insert(tmp.cbegin(), tmp.cend());
            }
            for (AttributePair const& attribute_pair : candidates) {
                AttributeSet context_delete_ab = deleteAttribute(
                    delAttrs[attribute_pair.left], attribute_pair.right);

                bool add_context = true;
                for (AttributeSet::size_type attr = context_delete_ab.find_first();
                    attr != AttributeSet::npos; attr = context_delete_ab.find_next(attr)) {
                    if (CSGet<ascending>(delAttrs[attr]).count(attribute_pair) == 0) {
                        add_context = false;
                        break;
                    }
                }

                if (add_context) {
                    CSPut<ascending>(context, attribute_pair);
                }
            }
        }
    }

    template <bool ascending>
    void CalcODs(AttributeSet const& context, std::vector<AttributeSet> const& delAttrs) {
        auto& cs_for_con = CSGet<ascending>(context);
        for (auto it = cs_for_con.begin(); it != cs_for_con.end();) {
            short a = it->left;
            short b = it->right;

            if (containsAttribute(CCGet(delAttrs[b]), a) &&
                 containsAttribute(CCGet(delAttrs[a]), b)) {
                CanonicalOD<ascending> od(deleteAttribute(delAttrs[a], b), a, b);
                if (od.IsValid(data_, partition_cache_)) {
                    ++ocd_count_;
                    addToRes(std::move(od));
                    cs_for_con.erase(it++);
                } else {
                    ++it;
                }
            } else {
                cs_for_con.erase(it++);
            }
        }
    }

public:
    Fastod(DataFrame data, long time_limit);

    void PrintStatistics() const;
    bool IsComplete() const;
    std::tuple<std::vector<CanonicalOD<true>>,
               std::vector<CanonicalOD<false>>,
               std::vector<SimpleCanonicalOD>> Discover();

    std::vector<std::string> DiscoverAsStrings();
};

} // namespace algos::fatod
