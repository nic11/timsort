enum EWhatMerge {
    WM_NoMerge,
    WM_MergeXY,
    WM_MergeYZ
};

struct ITimSortParams {
    virtual size_t minRun(size_t n) const = 0;
    virtual bool needMerge(size_t lenX, size_t lenY) const = 0;
    virtual EWhatMerge whatMerge(size_t lenX, size_t lenY, size_t lenZ) const = 0;
    virtual size_t getGallop() const = 0;
};

struct DefaultTimSortParams : public ITimSortParams {
    size_t minRun(size_t n) const {
        bool flag = false;
        while (n >= 64) {
            flag |= n & 1;
            n >>= 1;
        }
        return n + flag;
    }

    bool needMerge(size_t lenX, size_t lenY) const {
        return lenX <= lenY;
    }

    EWhatMerge whatMerge(size_t lenX, size_t lenY, size_t lenZ) const {
        if (lenY > lenZ && lenX > lenY + lenZ) {
            return WM_NoMerge;
        }

        return lenX < lenZ? WM_MergeXY : WM_MergeYZ;
    }

    size_t getGallop() const {
        return 7;
    }
} defaultTimSortParams;
