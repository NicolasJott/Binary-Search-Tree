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

    explicit BSTDictionary(uint32_t cap = DEFAULT_INITIAL_CAPACITY) {
        if (nTrees == 0) {
            counts =  new uint32_t[cap];
            heights = new uint32_t[cap];
            left = new uint32_t[cap];
            right = new uint32_t[cap];
            keys = new KeyType[cap];
            values = new ValueType[cap];

            capacity = cap;

            for (size_t i = 0; i < cap - 1; i++) {
                left[i] = i + 1;
            }

            left[capacity-1] = NULL_INDEX;

            freeListHead = 0;

        }
        nTrees++;
        root = NULL_INDEX;

    }

    ~BSTDictionary() {
        nTrees--;
        if (nTrees == 0) {
            delete[] counts;
            delete[] heights;
            delete[] left;
            delete[] right;
            delete[] keys;
            delete[] values;
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

    ValueType & search(const KeyType& key) {
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
        uint32_t ntbd = search(key);
        root = prvRemove(root, ntbd, key);

        if (ntbd != NULL_INDEX)
            prvFree(ntbd);
    }

private:

    uint32_t root;                  // tree root

    static uint32_t                 // this is the shared data
    *counts,                    // counts for each node
    *heights,                   // heights for each node
    *left,                      // left node indexes
    *right,                     // right node indexes
    nTrees,                     // number of BSTs with this key and value type
    capacity,                   // size of the arrays
    freeListHead;               // the head of the unused node list (the free list)

    static
    KeyType *keys;           // pool of keys
    static
    ValueType *values;       // pool of values

    uint32_t prvAllocate() {
        if (freeListHead == NULL_INDEX) {
            const uint32_t newCapacity = 2 * capacity;
            auto tmpCounts = new uint32_t[newCapacity];
            auto tmpHeights = new uint32_t[newCapacity];
            auto tmpLeft = new uint32_t[newCapacity];
            auto tmpRight = new uint32_t[newCapacity];
            auto tmpKeys = new KeyType[newCapacity];
            auto tmpValues = new ValueType[newCapacity];


            for (uint32_t i = 0; i < capacity; i++) {
                tmpCounts[i] = counts[i];
                tmpHeights[i] = heights[i];
                tmpLeft[i] = left[i];
                tmpRight[i] = right[i];
                tmpKeys[i] = keys[i];
                tmpValues[i] = values[i];
            }

            delete[] counts;
            delete[] heights;
            delete[] left;
            delete[] right;
            delete[] keys;
            delete[] values;

            // point shared pointers to temp arrays
                counts = tmpCounts;
                heights = tmpHeights;
                left = tmpLeft;
                right = tmpRight;
                keys = tmpKeys;
                values = tmpValues;


            for (size_t i = capacity; i < newCapacity - 1; i++) {
                left[i] = i + 1;
            }
            left[newCapacity-1] = NULL_INDEX;

            freeListHead = capacity;

            capacity = newCapacity;
        }

        uint32_t tmp = freeListHead;
        freeListHead = left[freeListHead];

        left[tmp] = NULL_INDEX;
        right[tmp] = NULL_INDEX;
        counts[tmp] = 1;
        heights[tmp] = 1;

        return tmp;
    }

    void prvFree(uint32_t n) {                        // deallocating node
        left[n] = freeListHead;
        freeListHead = n;
    }


    void prvClear(uint32_t r) {
        if (r != NULL_INDEX) {
            prvClear(left[r]);
            prvClear(right[r]);
            prvFree(r);
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

                        KeyType temp;
                        temp = keys[tmp];
                        keys[tmp] = keys[r];
                        keys[r] = temp;

                        ValueType tmpValues;
                        tmpValues = values[tmp];
                        values[tmp] = values[r];
                        values[r] = tmpValues;

                        right[r] = prvRemove(right[r], ntbd, key);
                    }
                    else {
                        uint32_t tmp = left[r];

                        while (right[tmp] != NULL_INDEX) {
                            tmp = right[tmp];
                        }

                        KeyType temp;
                        temp = keys[tmp];
                        keys[tmp] = keys[r];
                        keys[r] = temp;

                        ValueType tmpValues;
                        tmpValues = values[tmp];
                        values[tmp] = values[r];
                        values[r] = tmpValues;

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
template <typename KeyType, typename ValueType>
KeyType *BSTDictionary<KeyType, ValueType>::keys = nullptr;
template <typename KeyType, typename ValueType>
ValueType *BSTDictionary<KeyType, ValueType>::values = nullptr;