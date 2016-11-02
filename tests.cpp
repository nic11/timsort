#include "timsort.cpp"

#include <iostream>
#include <algorithm>
#include <cassert>
#include <deque>
#include <vector>
#include <random>
#include <limits>
#include <functional>

using std::is_sorted;
using std::vector;
using std::deque;

template<class BaseComparator>
class ComparisonCounter {
private:
    const BaseComparator f;
    unsigned long long mCalls;
public:
    ComparisonCounter(): f(), mCalls(0) {}

    template<class... Args>
    bool operator()(Args... args) {
        ++mCalls;
        return f(args...);
    }

    unsigned long long count() const {
        return mCalls;
    }
};

void testWithArray() {
    int a[] = {1, 3, 2, 2, 6, -1, 17, 1521, 512, 12, 0, 0, 0, 0, 0};
    timSort(a, a + sizeof(a) / sizeof(int));
    assert(is_sorted(a, a + sizeof(a) / sizeof(int)));
}

void testWithDeque() {
    deque<int> a = {1, 3, 2, 2, 6, -1, 17, 1521, 512, 12, 0, 0, 0, 0, 0};
    timSort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
}

void testOnPermutations(size_t n) {
    std::cout << "Testing on permutations..." << std::endl;
    vector<size_t> p(n);
    for (size_t i = 0; i < n; ++i) {
        p[i] = i;
    }

    do {
        vector<size_t> p1(p);
        timSort(p1.begin(), p1.end());
        assert(is_sorted(p1.begin(), p1.end()));
    } while (std::next_permutation(p.begin(), p.end()));
}

void testOnRandomArrays(size_t n, size_t runs) {
    std::mt19937 rng(5219);
    std::uniform_int_distribution<int> dist(
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max()
    );

    std::cout << "Testing on random arrays\n";

    for (size_t run = 0; run < runs; ++run) {
        vector<int> a0(n);
        for (size_t i = 0; i < n; ++i) {
            a0[i] = dist(rng);
        }

        std::cout << "Run #" << run << "...\n";

        {
            vector<int> a(a0);
            ComparisonCounter<std::less<int>> counter;
            clock_t start = clock();
            timSort(a.begin(), a.end(), std::ref(counter));
            clock_t finish = clock();
            assert(is_sorted(a.begin(), a.end()));

            std::cout << "timSort finished in " << (finish - start) * 1000 / CLOCKS_PER_SEC << " ms (" << counter.count() << " comparisons)\n";
        }

        {
            vector<int> a(a0);
            ComparisonCounter<std::less<int>> counter;
            clock_t start = clock();
            std::sort(a.begin(), a.end(), std::ref(counter));
            clock_t finish = clock();

            std::cout << "std::sort finished in " << (finish - start) * 1000 / CLOCKS_PER_SEC << " ms (" << counter.count() << " comparisons)\n";
        }
    }
}

void testOnPartitiallySortedArrays(size_t m, size_t c) {
    std::mt19937 rng(5219);
    std::uniform_int_distribution<int> dist(
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max()
    );

    std::cout << "Testing on " << c << " sorted arrays, " << m << " elements in each\n";

    std::cout << "generating... " << std::flush;
    vector<int> a0(m * c);
    for (size_t i = 0; i < m * c; ++i) {
        a0[i] = dist(rng);
    }
    for (size_t i = 0; i < c; ++i) {
        std::sort(a0.begin() + i * m, a0.begin() + (i + 1) * m);
        if (rng() & 1) {
            std::reverse(a0.begin() + i * m, a0.begin() + (i + 1) * m);
        }
    }
    std::cout << "done!" << std::endl;

    {
        vector<int> a(a0);
        ComparisonCounter<std::less<int>> counter;
        clock_t start = clock();
        timSort(a.begin(), a.end(), std::ref(counter));
        clock_t finish = clock();
        assert(is_sorted(a.begin(), a.end()));

        std::cout << "timSort finished in " << (finish - start) * 1000 / CLOCKS_PER_SEC << " ms (" << counter.count() << " comparisons)\n";
    }

    {
        vector<int> a(a0);
        ComparisonCounter<std::less<int>> counter;
        clock_t start = clock();
        std::sort(a.begin(), a.end(), std::ref(counter));
        clock_t finish = clock();

        std::cout << "std::sort finished in " << (finish - start) * 1000 / CLOCKS_PER_SEC << " ms (" << counter.count() << " comparisons)\n";
    }
}

struct My {
    int key, payload;
};

bool cmpMy(const My &a, const My &b) {
    return a.key < b.key;
}

bool operator==(const My &a, const My &b) {
    return a.key == b.key && a.payload == b.payload;
}

void testStablity(size_t m, size_t c) {
    std::mt19937 rng(5219);

    vector<My> a0(m * c);
    for (size_t i = 0; i < m * c; ++i) {
        a0[i].key = i / c;
        a0[i].payload = i % m;
    }
    vector<My> a(a0);
    shuffle(a.begin(), a.end(), rng);

    vector<size_t> kc(m);
    for (size_t i = 0; i < m * c; ++i) {
        a[i].payload = kc[a[i].key]++;
    }

    timSort(a.begin(), a.end(), cmpMy);

    assert(a == a0);
}

void testInplaceMerge(size_t n, size_t m, size_t runs) {
    std::mt19937 rng(5219);
    std::uniform_int_distribution<int> dist(1, 10);

    for (size_t ii = 0; ii < runs; ++ii) {
        vector<int> a(n + m);
        for (size_t i = 0; i < n + m; ++i) {
            a[i] = dist(rng);
        }
        sort(a.begin(), a.begin() + n);
        sort(a.end() - m, a.end());

        vector<int> b(a), a0(a);
        inplaceMerge(a.begin(), a.begin() + n, a.end(), std::less<int>(), 7);
        std::sort(b.begin(), b.end());
        if (a != b) {
            for (size_t i = 0; i < n + m; ++i) {
                std::cout << a0[i] << " ";
            }
            std::cout << "\n";
            std::cerr << "testInplaceMerge failed (" << n << " " << m << ")\n";
            exit(1);
        }
    }
}

struct Point3D {
    int x, y, z;
};

bool pointCmpFunction(const Point3D &a, const Point3D &b) {
    if (a.x == b.x) {
        if (a.y == b.y) {
            return a.z < b.z;
        }
        return a.y < b.y;
    }
    return a.x < b.x;
}

struct Point3DCmpFunctor {
    bool operator()(const Point3D &a, const Point3D &b) const {
        if (a.x == b.x) {
            if (a.y == b.y) {
                return a.z < b.z;
            }
            return a.y < b.y;
        }
        return a.x < b.x;
    }
};

void testWithPoints(size_t n) {
    std::mt19937 rng(5219);
    std::uniform_int_distribution<int> dist(-100, 100);

    vector<Point3D> a0(n);
    for (size_t i = 0; i < n; ++i) {
        a0[i].x = dist(rng);
        a0[i].y = dist(rng);
        a0[i].z = dist(rng);
    }

    vector<Point3D> a(a0);
    timSort(a.begin(), a.end(), pointCmpFunction);
    assert(is_sorted(a.begin(), a.end(), pointCmpFunction));

    a = a0;
    timSort(a.begin(), a.end(), Point3DCmpFunctor());
    assert(is_sorted(a.begin(), a.end(), Point3DCmpFunctor()));
}

int main() {
    testInplaceMerge(5, 3, 1000);
    testInplaceMerge(1, 3, 1000);
    testInplaceMerge(1, 4, 1000);
    testInplaceMerge(15, 15, 1000);
    testInplaceMerge(1, 21, 1000);

    testWithArray();
    testWithDeque();
    testOnRandomArrays(100000, 10); // generally fails on this one, so moved a little bit up
    testOnPermutations(10);
    testOnRandomArrays(50, 10);

    vector<size_t> lengths = {20, 40, 80, 128, 1024};
    vector<size_t> counts = {2, 4, 10, 100, 1000, 10000};
    for (size_t m : lengths) {
        for (size_t c : counts) {
            testOnPartitiallySortedArrays(m, c);
        }
    }

    testOnPartitiallySortedArrays(100000, 10);

    testWithPoints(100000);

    std::cout << "Tests have finished successfully.\n";
}
