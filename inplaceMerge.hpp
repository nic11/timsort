template<class RandomAccessIterator, class Compare>
size_t advanceGallop(
    RandomAccessIterator first, RandomAccessIterator last,
    const typename std::iterator_traits<RandomAccessIterator>::value_type &x,
    Compare cmp
) {
    size_t r = 1;
    while (cmp(*(first + r), x)) {
        r <<= 1;
        if (first + r >= last) {
            r = last - first;
            break;
        }
    }
    size_t l = 0;
    while (r - l > 1) {
        size_t m = l + ((r - l) >> 1);
        if (cmp(*(first + m), x)) {
            l = m;
        } else {
            r = m;
        }
    }
    return l;
}

template<class RandomAccessIterator, class Compare>
void mergeBlocks(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator buffer, Compare cmp, size_t gallop) {
    RandomAccessIterator last = middle + (middle - first);
    RandomAccessIterator bufLast = buffer + (middle - first);
    std::swap_ranges(first, middle, buffer);
    size_t c1 = 0, c2 = 0;
    RandomAccessIterator it1 = buffer, to = first, it2 = middle;

    while (it1 != bufLast && it2 != last) {
        if (c1 >= gallop) {
            c1 = 0;

            size_t g = advanceGallop(it1, bufLast, *it2, cmp);
            for (size_t j = 0; j < g; ++j) {
                if (!cmp(*it1, *it2) || it1 == bufLast) {
                    return;
                }
                std::swap(*to, *it1);
                ++it1;
                ++to;
            }
        }

        if (c2 >= gallop) {
            c2 = 0;

            size_t g = advanceGallop(it2, last, *it1, cmp);
            for (size_t j = 0; j < g; ++j) {
                if (cmp(*it1, *it2) || it2 == last) {
                    return;
                }
                std::swap(*to, *it2);
                ++it2;
                ++to;
            }
        }

        if (cmp(*it1, *it2)) {
            std::swap(*it1, *to);
            ++it1;
            ++c1;
            c2 = 0;
        } else {
            std::swap(*it2, *to);
            ++it2;
            ++c2;
            c1 = 0;
        }
        ++to;
    }
    while (it1 != bufLast) {
        std::swap(*it1, *to);
        ++it1;
        ++to;
    }
}

template<class RandomAccessIterator, class Compare>
void inplaceMerge(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, Compare cmp, size_t gallop) {
    size_t n = last - first;
    if (n < 20) {
        insertionSort(first, first, last, cmp);
        return;
    }

    size_t k = (int)sqrt(n);
    size_t parts = n / k - 1;
    size_t rem = n % k + k;

    size_t d = std::min(size_t(middle - first), k * parts);

    size_t yellow = d / k;
    std::swap_ranges(first + k * yellow, first + k * (yellow + 1), first + k * parts);

    for (size_t i = 0; i < parts; ++i) {
        size_t best = i;
        for (size_t j = i + 1; j < parts; ++j) {
            typename std::iterator_traits<RandomAccessIterator>::value_type
                a0 = *(first + k * best), a1 = *(first + k * (best + 1) - 1),
                b0 = *(first + k * j), b1 = *(first + k * (j + 1) - 1);

            if (cmp(b0, a0) || (/*!cmp(b0, a0) && */!cmp(a0, b0) && cmp(b1, a1))) {
                best = j;
            }
        }
        if (best != i) {
            std::swap_ranges(first + k * i, first + k * (i + 1), first + k * best);
        }
    }


    for (size_t i = 1; i < parts; ++i) {
        mergeBlocks(first + k * (i - 1), first + k * i, first + k * parts, cmp, gallop);
    }

    insertionSort(last - 2 * rem, last - 2 * rem, last, cmp);

    if (n >= 2 * rem) {
        for (size_t i = n - 2 * rem; i >= rem; i -= rem) {
            mergeBlocks(first + i - rem, first + i, last - rem, cmp, gallop);

            // avoiding unsigned overflow in i >= rem
            if (i < 2 * rem) {
                break;
            }
        }
    }

    insertionSort(first, first, first + 2 * rem, cmp);
    insertionSort(last - rem, last - rem, last, cmp);
}
