#ifndef LABELLEDSET_H_
#define LABELLEDSET_H_

#include <Arduino.h>

#include <utils/bst.h>
#include <utils/hashtable.h>

// THE ONLY THING THAT IS STORED ON THE HEAP IS THE VALUES OF INTEREST THEMSELVES

// N-dimensional variable
template <typename K, typename V> class SubSet {
  public:
    K key;      // Assigned on construction
    V* values;  // Array of UNIQUE values; new on construction and re-new/delete on set
    uint8_t numvalues;  // Size of values[]; updated

    SubSet(K key);
    SubSet(K key, V* values, uint8_t numvalues);
    ~SubSet();

    // void set(V value);
    void set(V* values, uint8_t numvalues);
    void set(K key, V* values, uint8_t numvalues);

    V& add(V value);
    void addGroup(V* values, uint8_t numvalues);
    void remove(V value);
    bool contains(V value);

    // Essentially a true ovewrite
    SubSet<K, V> operator=(const SubSet<K, V>& other);
};

/**
 * A "labelled set" is an Ordered Set of unique values (type V; numerical comparison >/</=) grouped by key (pointer-type K; byte-wise comparison aka strcmp). 
 * These values are stored in SubSet arrays located in the entries of a hash table (again, by key). Essentially a list of `{ "key" : values[] }` pairs where keys label values[] SubSets, each values[i] is unique in the broader Set.
 **/
template <typename K, typename V> class LabelledSet {
  private:    
    // Tables/trees are allocated STATICALLY, their entries are dynamic

    /**
     * A hashtable stores the keys and facilitates comparison/sorting/searching by key.
     * The hashtable is also responsible for the creation of subsets
     * Once a subset pointer has been inserted into the hash table, it cannot be REMOVED or MODIFIED (further emphasizing the link between subsets and their const-static/PROGMEM nature), only DESTROYED.
    */
    typedef HashTable<K, SubSet<K&, V>> SubSetTable;

    /**
     * A binary search tree is also implemented "in tandem". The BST stores REFERENCES to values (reusing memory space) and a reference to the subset 
     **/
    typedef BST<V&, SubSet<K&, V>*> OrderedSet;

    OrderedSet set = OrderedSet();
    SubSetTable subsets = SubSetTable();

    SubSet<K&, V>* addEmptySubSet(K key, bool overwrite = false);
  public:
    /**
     * Allocates a slot for a new value and inserts it into the table.
     * @param fqa
     * @param id
     * @param overwrite
     * @return Pointer to the state group
     */
    SubSet<K&, V>* add(K key, V value, bool overwrite = true);

    SubSet<K&, V>* addSubSet(K key, V* values, uint8_t numvalues, bool overwrite = true);
    
    /**
     * Finds a device and removes it from the table.
     * @param fqa
     * @param id
     * @param overwrite
     * @return Boolean: was the element found?
     */
    bool remove(V value);

    /**
     * Find a device group by ID.
     * @param id
     * @returns Pointer to the device group (`nullptr` if none)
     */
    SubSet<K&, V>* operator[](K key);

    /**
     * Find a device ID by FQA.
     * @param fqa
     * @returns Pointer to the device ID (`nullptr` if none)
     */
    SubSet<K&, V>* operator[](V value);

    // OrderedSet getSet(void);

    // SubSetTable getSubSets(void);
};

#include <LabelledSet.tpp>

#endif