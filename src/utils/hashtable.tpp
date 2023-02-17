#ifndef LABELLEDSET_HASHTABLE_T_H_
#define LABELLEDSET_HASHTABLE_T_H_

#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

inline static uint8_t hash(const char* s) {
  unsigned index;
  for (index = 0; *s != '\0'; s++) {
    index = *s + HASHTABLE_OFFSET * index;
  }
  return index % HASHTABLE_SLOTS;
}

// HASH TABLE ENTRY

template <typename K, typename V> HashTableEntry<K, V>::HashTableEntry(K key, V value, HashTableEntry<K, V>* last) : key(strdup(key)), value(value), next(last) { }

template <typename K, typename V> HashTableEntry<K, V>::~HashTableEntry() {
  // Free our key then trigger the next entry
  free((void*)this->key);
  delete(this->next);
}

// HASH TABLE

template <typename K, typename V> HashTable<K, V>::HashTable() { 
  for(int i = 0; i < HASHTABLE_SLOTS; i++) {
    hashtable[i] = nullptr;
  }
}

template <typename K, typename V> HashTable<K, V>::~HashTable() {
  // Free all allocated entries (and their keys) recursively; slots are static
  for (uint8_t i = 0; i < HASHTABLE_SLOTS; i++) {
    delete(this->hashtable[i]);
  }
}

template <typename K, typename V> V* HashTable<K, V>::operator[](K key) {
  HashTableEntry<K, V>* ptr = get(key);
  return ptr == nullptr ? nullptr : &(ptr->value);
}

// Public methods

template <typename K, typename V> HashTableEntry<K, V>* HashTable<K, V>::set(K key, V value, bool overwrite) {
  HashTableEntry<K, V>* head = get(key);

  // Match found?
  if (head != nullptr) {
    if (overwrite) head->value = value;
    return head;
  }
  
  // No match, allocate new; point "next" to the top entry
  uint8_t index = hash(key);
  head = new HashTableEntry<K, V>(key, value, hashtable[index]);

  // If allocation was successful:
  if (head != nullptr && head->key != nullptr) {
    // Point the top of the hashtable to the new entry
    hashtable[index] = head;
  }

  return head;
}

template <typename K, typename V> HashTableEntry<K, V>* HashTable<K, V>::get(K key) {
  HashTableEntry<K, V>* np;
  for (np = hashtable[hash(key)]; np != nullptr; np = np->next) {
    if (strcmp(key, np->key) == 0) {
      return np; /* found */
    }
  }
  return nullptr; /* not found */
}

template <typename K, typename V> bool HashTable<K, V>::remove(K key) {
  // Find ptr.next.key == key
  uint8_t index = hash(key);
  HashTableEntry<K, V>* ptr = hashtable[index];

  // Check top-level
  if (strcmp(key, ptr->key) == 0) {
    // Found; point the slot to next
    hashtable[index] = ptr->next;
    ptr->next = nullptr;
    delete(ptr);
    return true;
  }

  for (; ptr->next != nullptr; ptr = ptr->next) {
    if (strcmp(key, ptr->next->key) == 0) {
      // Found; point the preceding entry to the subsequent one
      HashTableEntry<K, V>* entry = ptr->next;
      ptr->next = ptr->next->next;
      entry->next = nullptr;
      delete(entry);
      return true;
    }
  }
  // Not found
  return false;
}

#endif