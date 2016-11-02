struct Run {
    size_t start, len;

    Run() = default;
    Run(size_t s, size_t l): start(s), len(l) {}
};

template<class Less>
class GreaterByLess {
private:
    const Less &less;
public:
    GreaterByLess(const Less &l): less(l) {}

    template<class T>
    bool operator()(const T &a, const T &b) const {
        return less(b, a);
    }
};

template<class RandomAccessIterator, class Compare>
RandomAccessIterator findNonDecreasingSequence(RandomAccessIterator first, RandomAccessIterator last, Compare cmp) {
    RandomAccessIterator prev = first++;
    while (first != last && !cmp(*first, *prev)) {
        ++prev;
        ++first;
    }
    return first;
}

template<class RandomAccessIterator, class Compare>
RandomAccessIterator findNonIncreasingSequence(RandomAccessIterator first, RandomAccessIterator last, Compare cmp) {
    return findNonDecreasingSequence(first, last, GreaterByLess<Compare>(cmp));
}

template<class RandomAccessIterator, class Compare>
void insertionSort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, Compare cmp) {
    while (middle != last) {
        RandomAccessIterator p = middle, pp = p;
        --pp;
        while (p != first && cmp(*p, *pp)) {
            std::swap(*p, *pp);
            --p;
            --pp;
        }
        ++middle;
    }
}

template<class RandomAccessIterator, class Compare>
void selectionSort(RandomAccessIterator first, RandomAccessIterator last, Compare cmp) {
    while (first != last) {
        RandomAccessIterator best = first;
        RandomAccessIterator it = first;
        for (++it; it != last; ++it) {
            if (cmp(*it, *best)) {
                best = it;
            }
        }
        if (first != best) {
            std::swap(*first, *best);
        }
        ++first;
    }
}

template<class RandomAccessIterator, class Compare>
std::vector<Run> selectAndSortRuns(RandomAccessIterator first, RandomAccessIterator last, Compare cmp, size_t minRun) {
    RandomAccessIterator initialBegin = first;

    std::vector<Run> ans;
    while (first != last) {
        size_t nonDecreasingLength = findNonDecreasingSequence(first, last, cmp) - first;
        size_t nonIncreasingLength = findNonIncreasingSequence(first, last, cmp) - first;
        size_t currentLength = nonDecreasingLength;
        if (nonDecreasingLength < nonIncreasingLength) {
            currentLength = nonIncreasingLength;
            std::reverse(first, first + currentLength);
        }

        if (currentLength < minRun) {
            size_t upto = std::min(minRun, size_t(last - first));
            insertionSort(first, first + currentLength, first + upto, cmp);
            // selectionSort(first, first + upto, cmp);
            currentLength = upto;
        }

        // std::cerr << "emplacing " << first - initialBegin << " " << currentLength << "\n";
        ans.emplace_back(first - initialBegin, currentLength);
        first += currentLength;
    }
    return ans;
}
