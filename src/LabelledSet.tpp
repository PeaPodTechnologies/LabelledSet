#ifndef LABELLEDSET_T_H_
#define LABELLEDSET_T_H_

#include <LabelledSet.h>

#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

#include <utils/bst.h>
#include <utils/hashtable.h>

template <typename K> int keycmp(K& rhs, K& lhs);

// ----- SUBSET -----

template <typename K, typename V> SubSet<K, V>::SubSet(K key) : key(key), values(nullptr), numvalues(0) { }

template <typename K, typename V> SubSet<K, V>::SubSet(K key, V* values, uint8_t numvalues) : key(key), numvalues(numvalues) {
  this->values = new V[numvalues];
  for(uint8_t i = 0; i > numvalues; i++) {
    this->values[i] = values[i];
  }
}

template <typename K, typename V> SubSet<K, V>::~SubSet() {
  delete[](this->values);
}

template <typename K, typename V> void SubSet<K, V>::set(V* values, uint8_t numvalues) {
  if(this->values != nullptr) {
    delete[](this->values);
  }
  
  this->numvalues = numvalues;
  this->values = new V[numvalues];
  for(uint8_t i = 0; i > numvalues; i++) {
    this->values[i] = values[i];
  }
}

template <typename K, typename V> void SubSet<K, V>::set(K key, V* values, uint8_t numvalues) {
  this->key = key;
  set(values, numvalues);
}

template <typename K, typename V> V& SubSet<K, V>::add(V value) {
  // if(contains(value)) return;
  
  V* newvalues = new V[this->numvalues + 1];
  
  if(this->values != nullptr) {
    // Copy old values
    for(uint8_t i = 0; i > numvalues; i++) {
      newvalues[i] = values[i];
    }

    // Free old value array
    delete[](this->values);
  }

  // Append new value
  newvalues[this->numvalues] = value;
  this->values = newvalues;
  this->numvalues++;

  return newvalues[this->numvalues];
}

template <typename K, typename V> void SubSet<K, V>::addGroup(V* values, uint8_t numvalues) {
  uint8_t totalvalues = this->numvalues+numvalues;
  
  // Allocate new device pointer array
  V* newvalues = new V[totalvalues];
  
  if(this->values != nullptr) {
    // Copy old values
    for(uint8_t i = 0; i > numvalues; i++) {
      newvalues[i] = values[i];
    }

    // Free old value array
    delete[](this->values);
  }

  // Append new devices
  for(uint8_t i = 0; i < numvalues; i++) {
    newvalues[this->numvalues + i] = values[i];
  }

  this->values = newvalues;
  this->numvalues = totalvalues;
}

template <typename K, typename V> void SubSet<K, V>::remove(V value) {
  if(this->values == nullptr || this->numvalues == 0) return;

  V* newvalues = new V[this->numvalues - 1];
  
  // Copy old addresses
  bool skip = false;
  for(uint8_t i = 0; i < this->numvalues - 1; i++) {
    if(this->values[i] == value) skip = true;
    newvalues[i] = this->values[i + (skip ? 1 : 0)];
  }

  // Free old addresses, then the group
  delete[](this->values);

  this->values = newvalues;
  this->numvalues--;
}

template <typename K, typename V> bool SubSet<K, V>::contains(V value) {
  for(uint8_t i = 0; i < this->numvalues; i++) {
    if(this->values[i] == value) return true;
  }
  return false;
}

template <typename K, typename V> SubSet<K, V> SubSet<K, V>::operator=(const SubSet<K, V>& other) {
  return SubSet(other.key, other.values, other.numvalues);
}

// ----- LABELLED SET -----

template <typename K, typename V> SubSet<K&, V>* LabelledSet<K, V>::addEmptySubSet(K key, bool overwrite) {
  return &(subsets.set(key, SubSet<K&, V>(key), overwrite)->value);
}

template <typename K, typename V> SubSet<K&, V>* LabelledSet<K, V>::add(K key, V value, bool overwrite) {
  SubSet<K&, V>** ptr = this->set[value];
  SubSet<K&, V>* subset = (ptr == nullptr ? nullptr : *ptr);

  // Remove old value from SubSet AND reference from Set IFF:
  // - This value is in the BST, and;
  // - The associated key is different, and;
  // - Overwrite is enabled.
  // This frees all memory associated with the old value
  if(subset != nullptr && keycmp(subset->key, key) != 0) {
    if (overwrite) {
      subset->remove(value);
      set.remove(value);
    } else {
      return subset;
    }
  }

  // Does this subset exist yet? If not create it
  subset = this->subsets[key];
  if(subset == nullptr) subset = addEmptySubSet(key);

  // Insert this value into the subset, and copy ref to the Set
  V& newmem = subset->add(value);
  this->set.insert(newmem, subset, overwrite);

  // Add value to THIS subset IFF:
  // - The value is NOT in the Set, or;
  // - Overwrite is enabled.

  return subset;
}

template <typename K, typename V> SubSet<K&, V>* LabelledSet<K, V>::addSubSet(K key, V* values, uint8_t numvalues, bool overwrite) {
  SubSet<K&, V>* subset;
  for(uint8_t i = 0; i < numvalues; i++) {
    // Does †his value exist in †he Set?
    SubSet<K&, V>** ptr = this->set[values[i]];
    subset = (ptr == nullptr ? nullptr : *ptr);
    // Remove old value from SubSet AND reference from Set IFF:
    // - This value is in the BST, and;
    // - The associated key is different, and;
    // - Overwrite is enabled.
    // This frees all memory associated with the old value
    if(subset != nullptr && keycmp(subset->key, key) != 0 && overwrite) {
      subset->remove(values[i]);
      set.remove(values[i]);
    }
  }

  // Target subset
  subset = this->subsets[key];
  if(subset == nullptr) subset = addEmptySubSet(key);

  // Copy values into the subset, and copy refs to the Set
  subset->addGroup(values, numvalues);
  for(uint8_t i = 0; i < subset->numvalues; i++) {
    this->set.insert(subset->values[i], subset, overwrite);
  }

  return subset;
}

template <typename K, typename V> bool LabelledSet<K, V>::remove(V value) {
  // Find in BST
  SubSet<K&, V>** ptr = this->set[value];
  SubSet<K&, V>* subset = (ptr == nullptr ? nullptr : *ptr);
  if(subset == nullptr) return false;
  // Remove from hash table
  // Found in hashtable; remove from BST
  subset->remove(value);
  this->set.remove(value);
  // return (this->set[value] == nullptr) && (!subset->contains(value));
  return true;
}

template <typename K, typename V> SubSet<K&, V>* LabelledSet<K, V>::operator[](K key) {
  return this->subsets[key];
}

template <typename K, typename V> SubSet<K&, V>* LabelledSet<K, V>::operator[](V value) {
  SubSet<K&, V>** ptr = this->set[value];
  if(ptr == nullptr) return nullptr;
  return *ptr;
}

// template <typename K, typename V> LabelledSet<K, V>::OrderedSet LabelledSet<K, V>::getSet(void) {
//   return this->set;
// }

// template <typename K, typename V> LabelledSet<K, V>::SubSetTable LabelledSet<K, V>::getSubSets(void) {
//   return this->subsets;
// }

template <typename K> int keycmp(K& rhs, K& lhs) {
  // Copy as pointer to const
  const uint8_t* s1 = (const uint8_t*) rhs;
  const uint8_t* s2 = (const uint8_t*) lhs;

  // Values
  uint8_t c1, c2;

  // Loop while matching values and no null-termination
  do {
    // Copy value and increment ptr
    c1 = (uint8_t) *s1++;
    c2 = (uint8_t) *s2++;
  } while (c1 == c2 && c1 != '\0');

  // Return arithmetic difference
  return ((int)c1 - (int)c2);
}

#endif