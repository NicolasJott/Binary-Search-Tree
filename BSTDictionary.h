#pragma once

#include <cstdint>
#include <stdexcept>

#define GET_COUNT(n) (((n) == NULL_INDEX) ? 0 : counts[n])
#define GET_HEIGHT(n) (((n) == NULL_INDEX) ? 0 : heights[n])

static const uint32_t
        NULL_INDEX = 0xffffffff,
        DEFAULT_INITIAL_CAPACITY = 16;

template <typename KeyType, typename ValueType>
class BSTDictionary {
public:

    void GenerateFreeList(uint32_t start, uint32_t end) {
        for (uint32_t i = start; i <= end; i++) {
            left[i] = (i + 1);
        }
        left[end] = NULL_INDEX;

        freeListHead = start;
    }

    explicit BSTDictionary(uint32_t _cap = DEFAULT_INITIAL_CAPACITY) {
        if (nTrees == 0) {
            counts =  new uint32_t[_cap];
            heights = new uint32_t[_cap];
            left = new uint32_t[_cap];
            right = new uint32_t[_cap];
            keys = new KeyType[_cap];
            values = new ValueType[_cap];


            capacity = _cap;

            GenerateFreeList(root, capacity);

        }
        nTrees++;
        root = NULL_INDEX;

    }

    ~BSTDictionary() {
        nTrees--;
        if (nTrees == 1) {
            delete[] counts, heights, left, right, keys, values;
        }
        else {
            prvClear(root);
        }
    }

    void clear() { prvClear(root); root = NULL_INDEX; }

    uint32_t size() {
        if (root == NULL_INDEX)
            return 0;

        return counts[root];
    }

    uint32_t height() {
        if (root == NULL_INDEX)
            return NULL_INDEX;

        return heights[root];
    }

    bool isEmpty() { return root == NULL_INDEX; }

    ValueType& search(const KeyType& key) {
        uint32_t n = root;
        while (n != NULL_INDEX) {
            if (key == keys[n]) {
                return values[n];
            }
            else if (key < keys[n]) {
                n = left[n];
            }
            else {
                n = right[n];
            }
        }

        throw std::domain_error("Search: Key not found");

    }

    ValueType& operator[](const KeyType& key) {
        uint32_t tmp = prvAllocate();
        uint32_t n = tmp;

        root = prvInsert(root, n, key);

        if (n != tmp) {
            prvFree(tmp);
        }

        return values[n];

    }

    void remove(const KeyType& key) {


    }

private:

    uint32_t root;                  // tree root

    static uint32_t                 // this is the shared data
    * counts,                    // counts for each node
    * heights,                   // heights for each node
    * left,                      // left node indexes
    * right,                     // right node indexes
    nTrees,                     // number of BSTs with this key and value type
    capacity,                   // size of the arrays
    freeListHead;               // the head of the unused node list (the free list)

    KeyType *keys;           // pool of keys
    ValueType *values;       // pool of values

    uint32_t prvAllocate() {
        if (freeListHead == NULL_INDEX) {
            const uint32_t tmpCapacity = 2 * capacity;
            uint32_t *tmpCounts = new uint32_t[tmpCapacity];
            uint32_t *tmpHeights = new uint32_t[tmpCapacity];
            uint32_t *tmpLeft = new uint32_t[tmpCapacity];
            uint32_t *tmpRight = new uint32_t[tmpCapacity];
            KeyType *tmpKeys = new KeyType[tmpCapacity];
            ValueType *tmpValues = new ValueType[tmpCapacity];


            for (uint32_t i = 0; i <= capacity; i++) {
                tmpCounts[i] = counts[i];
                tmpHeights[i] = heights[i];
                tmpLeft[i] = left[i];
                tmpRight[i] = right[i];
                tmpKeys[i] = keys[i];
                tmpValues[i] = values[i];
            }

            delete[] counts, heights, left, right, keys, values;

            // point shared pointers to temp arrays

            GenerateFreeList(root, tmpCapacity);
        }

        uint32_t tmp = freeListHead;
        freeListHead = left[freeListHead];

        left[tmp] = NULL_INDEX;
        right[tmp] = NULL_INDEX;
        counts[tmp] = 1;
        heights[tmp] = 1;

        return tmp;
    }

    void prvFree(uint32_t r) {                        // deallocating node
        left[r] = freeListHead;

        freeListHead = r;
    }


    void prvClear(uint32_t r) {
        if (r != NULL_INDEX) {
            prvClear(left[r]);
            prvClear(right[r]);
        }
    }

    void prvAdjust(uint32_t r) {
        counts[r] = GET_COUNT(left[r]) + GET_COUNT(right[r]) + 1;

        heights[r] = std::max(GET_HEIGHT(left[r]), GET_HEIGHT(right[r])) + 1;
    }

    uint32_t prvInsert(uint32_t r, uint32_t& n, const KeyType& key) {
        if (r == NULL_INDEX) {
            keys[n] = key;

            return n;
        }

        if (key == keys[r]) {
            n = r;
        }
        else if (key < keys[r]) {
            left[r] = prvInsert(left[r], n, key);
        }
        else {
            right[r] = prvInsert(right[r], n, key);
        }

        prvAdjust(r);

        return r;
    }


    uint32_t prvRemove(uint32_t r, uint32_t& ntbd, const KeyType& key) {
        if (r == NULL_INDEX) {
            throw std::domain_error("Remove: Key not found.");
        }


        if (key < keys[r]) {
            left[r] = prvRemove(left[r], ntbd, key);
        }
        else if (key > keys[r]) {
            right[r] = prvRemove(right[r], ntbd, key);
        }
        else {
            ntbd = r;

            if (left[r] == NULL_INDEX) {
                if (right[r] == NULL_INDEX) {
                    r = NULL_INDEX;
                }
                else {
                    r = right[r];
                }
            }
            else {
                if (right[r] == NULL_INDEX) {
                    r = left[r];
                }
                else {
                    if (GET_HEIGHT(right[r]) > GET_HEIGHT(left[r])) {
                        uint32_t tmp = right[r];

                        while (left[tmp] != NULL_INDEX) {
                            tmp = left[tmp];
                        }

                        keys[r] = keys[tmp];
                        values[r] = values[tmp];

                        right[r] = prvRemove(right[r], ntbd, key);
                    }
                    else {
                        uint32_t tmp = left[r];

                        while (right[tmp] != NULL_INDEX) {
                            tmp = right[tmp];
                        }

                        keys[r] = keys[tmp];
                        values[r] = values[tmp];

                        left[r] = prvRemove(left[r], ntbd, key);
                    }
                }
            }
        }

        if (r != NULL_INDEX) {
            prvAdjust(r);
        }

        return r;
    }


};

template <typename KeyType, typename ValueType>
uint32_t* BSTDictionary<KeyType, ValueType>::counts = nullptr;
template <typename KeyType, typename ValueType>
uint32_t* BSTDictionary<KeyType, ValueType>::heights = nullptr;
template <typename KeyType, typename ValueType>
uint32_t* BSTDictionary<KeyType, ValueType>::left = nullptr;
template <typename KeyType, typename ValueType>
uint32_t* BSTDictionary<KeyType, ValueType>::right = nullptr;
template <typename KeyType, typename ValueType>
uint32_t BSTDictionary<KeyType, ValueType>::nTrees = 0;
template <typename KeyType, typename ValueType>
uint32_t BSTDictionary<KeyType, ValueType>::capacity = 0;
template <typename KeyType, typename ValueType>
uint32_t BSTDictionary<KeyType, ValueType>::freeListHead = 0;