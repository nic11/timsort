#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>

#include "params.hpp"
#include "runs.hpp"
#include "inplaceMerge.hpp"

template<class RandomAccessIterator, class Compare>
const Run mergeRuns(RandomAccessIterator first, Compare cmp, Run a, Run b, size_t gallop) {
    assert(a.start < b.start);
    assert(a.start + a.len == b.start);
    inplaceMerge(first + a.start, first + b.start, first + b.start + b.len, cmp, gallop);
    return Run(a.start, a.len + b.len);
}

template<class RandomAccessIterator, class Compare>
void timSort(RandomAccessIterator first, RandomAccessIterator last, Compare cmp, const ITimSortParams &params) {
    size_t minRun = params.minRun(last - first);
    size_t gallop = params.getGallop();
    std::vector<Run> runs = selectAndSortRuns(first, last, cmp, minRun);

    std::vector<Run> s;

    for (size_t i = 0; i < runs.size(); ++i) {
        s.push_back(runs[i]);
        bool canBeBad = true;
        while (canBeBad) {
            if (s.size() >= 3) {
                const Run
                    &x1 = s[s.size() - 3],
                    &y1 = s[s.size() - 2],
                    &z1 = s[s.size() - 1];
                switch (params.whatMerge(x1.len, y1.len, z1.len)) {
                case WM_NoMerge: {
                    canBeBad = false;
                    break;
                }
                case WM_MergeXY: { // <- doesn't compile without them ¯\_(ツ)_/¯
                    Run xy = mergeRuns(first, cmp, x1, y1, gallop), z = z1;
                    s.erase(s.end() - 3, s.end());
                    s.push_back(xy);
                    s.push_back(z);
                    break;
                }
                case WM_MergeYZ: {
                    Run yz = mergeRuns(first, cmp, y1, z1, gallop);
                    s.erase(s.end() - 2, s.end());
                    s.push_back(yz);
                    break;
                }
                }
            } else if (s.size() == 2 && params.needMerge(s[0].len, s[1].len)) {
                Run r = mergeRuns(first, cmp, s[0], s[1], gallop);
                s.clear();
                s.push_back(r);
            } else {
                canBeBad = false;
            }
        }
    }

    while (s.size() > 1) {
        Run r = mergeRuns(first, cmp, s[s.size() - 2], s[s.size() - 1], gallop);
        s.erase(s.end() - 2, s.end());
        s.push_back(r);
    }
}

template<class RandomAccessIterator, class Compare>
void timSort(RandomAccessIterator first, RandomAccessIterator last, Compare cmp) {
    timSort(first, last, cmp, defaultTimSortParams);
}

template<class RandomAccessIterator>
void timSort(RandomAccessIterator first, RandomAccessIterator last) {
    timSort(first, last, std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>());
}
