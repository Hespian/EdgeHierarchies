/*******************************************************************************
 * lib/arraySet.h
 *
 * Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <limits>

using namespace std;

template <typename T>
class ArraySet {
public:
    ArraySet(T size) {
        lookup.resize(size, std::numeric_limits<T>::max());
        elements.resize(size);
        endPtr = 0;
    }

    bool contains(T element) {
        assert(element < lookup.size());
        return lookup[element] < endPtr;
    }

    void insert(T element) {
        assert(element < lookup.size());
        assert(!contains(element));
        lookup[element] = endPtr;
        elements[endPtr++] = element;
    }

    void remove(T element) {
        assert(element < lookup.size());
        assert(contains(element));
        elements[lookup[element]] = elements[--endPtr];
        lookup[elements[endPtr]] = lookup[element];
        lookup[element] = std::numeric_limits<T>::max();
    }

    T size() {
        return endPtr;
    }

    T capacity() {
        return lookup.size();
    }

    void resize(T newSize) {
        assert(endPtr < newSize);
        lookup.resize(newSize, std::numeric_limits<T>::max());
        elements.resize(newSize);
    }

    typename vector<T>::iterator begin() { return elements.begin();   }
    typename vector<T>::iterator end()   { return elements.begin() + endPtr; }

    typename vector<T>::const_iterator begin() const { return elements.begin();   }
    typename vector<T>::const_iterator end()   const { return elements.begin() + endPtr; }

protected:
    vector<T> lookup;
    vector<T> elements;
    T endPtr;

};
