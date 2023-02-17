#ifndef LABELLEDSET_HASHTABLE_H_
#define LABELLEDSET_HASHTABLE_H_

// Parameters
#define HASHTABLE_OFFSET 31
#define HASHTABLE_SLOTS  16

template <typename K, typename V> class HashTableEntry {
  public:
    HashTableEntry(K key, V value, HashTableEntry<K, V>* last = nullptr);
    ~HashTableEntry();

    K key;
    V value;
    HashTableEntry<K, V>* next;
};

template <typename K, typename V> class HashTable {
  public:
    // Slots are static, entries are dynamic
    HashTableEntry<K, V>* hashtable[HASHTABLE_SLOTS] = { nullptr }; // FIXED POINTER to the entries - de/allocated on de/construction

    HashTable();
    ~HashTable();

    /**
     * Put {key: value} in the hash table.
     * @param key String key
     * @param value Pointer to value
     * @param overwrite Overwrite existing value if found? Default: `true`
     * @return Pointer to the new entry
     */
    HashTableEntry<K, V>* set(K key, V value, bool overwrite = true);

    /**
     * Look at the index for that key, down the chain until either key=key or next = nullptr
     * @param key to look for
     * @return Pointer if found, nullptr otherwise
     */
    HashTableEntry<K, V>* get(K key);

    /**
     * Index operator by key.
     * @param key
     * @return Pointer to entry
     */
    V* operator[](K key);

    bool remove(K key);
};

#endif

#include <utils/hashtable.tpp>