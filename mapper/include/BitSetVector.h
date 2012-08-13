//===-- llvm/ADT/BitVectorSet.h - A bit-vector rep. of sets -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is an implementation of the bit-vector representation of sets.  Unlike
// vector<bool>, this allows much more efficient parallel set operations on
// bits, by using the bitset template.  The bitset template unfortunately can
// only represent sets with a size chosen at compile-time.  We therefore use a
// vector of bitsets.  The maxmimum size of our sets (i.e., the size of the
// universal set) can be chosen at creation time.
//
// External functions:
//
// bool Disjoint(const BitSetVector& set1, const BitSetVector& set2):
//    Tests if two sets have an empty intersection.
//    This is more efficient than !(set1 & set2).any().
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_BITSETVECTOR_H
#define LLVM_ADT_BITSETVECTOR_H

#include "llvm/Support/raw_ostream.h"
#include <bitset>
#include <vector>
#include <iostream>
#include <functional>

namespace llvm {

class BitSetVector {
  enum { BITSET_WORDSIZE = sizeof(long)*8 };

  // Types used internal to the representation
  typedef std::bitset<BITSET_WORDSIZE> bitword;
  typedef bitword::reference reference;

  // Data used in the representation
  std::vector<bitword> bitsetVec;
  unsigned maxSize;

private:
  // Utility functions for the representation
  static unsigned NumWords(unsigned Size) {
    return (Size+BITSET_WORDSIZE-1)/BITSET_WORDSIZE;
  }
  static unsigned LastWordSize(unsigned Size) { return Size % BITSET_WORDSIZE; }

  // Clear the unused bits in the last word.
  // The unused bits are the high (BITSET_WORDSIZE - LastWordSize()) bits
  void ClearUnusedBits() {
    unsigned long usedBits = (1U << LastWordSize(size())) - 1;
    bitsetVec.back() &= bitword(usedBits);
  }

  const bitword& getWord(unsigned i) const { return bitsetVec[i]; }
        bitword& getWord(unsigned i)       { return bitsetVec[i]; }

  friend bool Disjoint(const BitSetVector& set1,
                       const BitSetVector& set2);

  BitSetVector();                       // do not implement!

public:
  class iterator;
  ///
  /// Constructor: create a set of the maximum size maxSetSize.
  /// The set is initialized to empty.
  ///
  BitSetVector(unsigned maxSetSize)
    : bitsetVec(NumWords(maxSetSize)), maxSize(maxSetSize) { }

  /// size - Return the number of bits tracked by this bit vector...
  unsigned size() const { return maxSize; }

  ///
  ///  Modifier methods: reset, set for entire set, operator[] for one element.
  ///
  void reset() {
    for (unsigned i=0, N = bitsetVec.size(); i < N; ++i)
      bitsetVec[i].reset();
  }
  void set() {
    for (unsigned i=0, N = bitsetVec.size(); i < N; ++i) // skip last word
      bitsetVec[i].set();
    ClearUnusedBits();
  }
  reference operator[](unsigned n) {
    assert(n  < size() && "BitSetVector: Bit number out of range");
    unsigned ndiv = n / BITSET_WORDSIZE, nmod = n % BITSET_WORDSIZE;
    return bitsetVec[ndiv][nmod];
  }
  iterator begin() { return iterator::begin(*this); }
  iterator end()   { return iterator::end(*this);   }

  ///
  ///  Comparison operations: equal, not equal
  ///
  bool operator == (const BitSetVector& set2) const {
    assert(maxSize == set2.maxSize && "Illegal == comparison");
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      if (getWord(i) != set2.getWord(i))
        return false;
    return true;
  }
  bool operator != (const BitSetVector& set2) const {
    return ! (*this == set2);
  }

  ///
  ///  Set membership operations: single element, any, none, count
  ///
  bool test(unsigned n) const {
    assert(n  < size() && "BitSetVector: Bit number out of range");
    unsigned ndiv = n / BITSET_WORDSIZE, nmod = n % BITSET_WORDSIZE;
    return bitsetVec[ndiv].test(nmod);
  }
  bool any() const {
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      if (bitsetVec[i].any())
        return true;
    return false;
  }
  bool none() const {
    return ! any();
  }
  unsigned count() const {
    unsigned n = 0;
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      n += bitsetVec[i].count();
    return n;
  }
  bool all() const {
    return (count() == size());
  }

  ///
  ///  Set operations: intersection, union, disjoint union, complement.
  ///
  BitSetVector operator& (const BitSetVector& set2) const {
    assert(maxSize == set2.maxSize && "Illegal intersection");
    BitSetVector result(maxSize);
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      result.getWord(i) = getWord(i) & set2.getWord(i);
    return result;
  }
  BitSetVector operator| (const BitSetVector& set2) const {
    assert(maxSize == set2.maxSize && "Illegal intersection");
    BitSetVector result(maxSize);
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      result.getWord(i) = getWord(i) | set2.getWord(i);
    return result;
  }
  BitSetVector operator^ (const BitSetVector& set2) const {
    assert(maxSize == set2.maxSize && "Illegal intersection");
    BitSetVector result(maxSize);
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      result.getWord(i) = getWord(i) ^ set2.getWord(i);
    return result;
  }
  BitSetVector operator~ () const {
    BitSetVector result(maxSize);
    for (unsigned i = 0; i < bitsetVec.size(); ++i)
      (result.getWord(i) = getWord(i)).flip();
    result.ClearUnusedBits();
    return result;
  }

  ///
  ///  Printing and debugging support
  ///
  void print(raw_ostream &O) const;
  void print(raw_ostream *O) const { if (O) print(*O); }
  void dump() const { print(errs()); }

public:
  //
  // An iterator to enumerate the bits in a BitSetVector.
  // Eventually, this needs to inherit from bidirectional_iterator.
  // But this iterator may not be as useful as I once thought and
  // may just go away.
  //
  class iterator {
    unsigned   currentBit;
    unsigned   currentWord;
    BitSetVector* bitvec;
    iterator(unsigned B, unsigned W, BitSetVector& _bitvec)
      : currentBit(B), currentWord(W), bitvec(&_bitvec) { }
  public:
    iterator(BitSetVector& _bitvec)
      : currentBit(0), currentWord(0), bitvec(&_bitvec) { }
    iterator(const iterator& I)
      : currentBit(I.currentBit),currentWord(I.currentWord),bitvec(I.bitvec) { }
    iterator& operator=(const iterator& I) {
      currentWord = I.currentWord;
      currentBit = I.currentBit;
      bitvec = I.bitvec;
      return *this;
    }

    // Increment and decrement operators (pre and post)
    iterator& operator++() {
      if (++currentBit == BITSET_WORDSIZE)
        { currentBit = 0; if (currentWord < bitvec->size()) ++currentWord; }
      return *this;
    }
    iterator& operator--() {
      if (currentBit == 0) {
        currentBit = BITSET_WORDSIZE-1;
        if (currentWord == 0)
            currentWord = bitvec->size();
        else
            --currentWord;
      }
      else
        --currentBit;
      return *this;
    }
    iterator operator++(int) { iterator copy(*this); ++*this; return copy; }
    iterator operator--(int) { iterator copy(*this); --*this; return copy; }

    // Dereferencing operators
    reference operator*() {
      assert(currentWord < bitvec->size() &&
             "Dereferencing iterator past the end of a BitSetVector");
      return bitvec->getWord(currentWord)[currentBit];
    }

    // Comparison operator
    bool operator==(const iterator& I) {
      return (I.bitvec == bitvec &&
              I.currentWord == currentWord && I.currentBit == currentBit);
    }
    bool operator!=(const iterator& I) {
      return !(*this == I);
    }

  protected:
    static iterator begin(BitSetVector& _bitvec) { return iterator(_bitvec); }
    static iterator end(BitSetVector& _bitvec)   { return iterator(0,
                                                    _bitvec.size(), _bitvec); }
    friend class BitSetVector;
  };
};

inline void BitSetVector::print(raw_ostream& O) const
{
  // for (std::vector<bitword>::const_iterator I=bitsetVec.begin(), E=bitsetVec.end(); I != E; ++I)
  //  O << "<" << (*I) << ">" << (I+1 == E? "\n" : ", ");
}

inline raw_ostream & operator<<(raw_ostream& O, const BitSetVector& bset) {
  bset.print(O);
  return O;
}

///
/// Optimized versions of fundamental comparison operations
///
inline bool Disjoint(const BitSetVector& set1,
                     const BitSetVector& set2)
{
  assert(set1.size() == set2.size() && "Illegal intersection");
  for (unsigned i = 0; i < set1.bitsetVec.size(); ++i)
    if ((set1.getWord(i) & set2.getWord(i)).any())
      return false;
  return true;
}

} // End llvm namespace
#endif
