#include <Arduino.h>
#include <unity.h>

#include <utils/bst.h>
#include <utils/hashtable.h>
#include <LabelledSet.h>

LabelledSet<const char*, int> labelledset = LabelledSet<const char*, int>();

const char* key1 = "test";

void test_labelledset_empty(void) {
  SubSet<const char*&, int>* subset = labelledset["null"];
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, subset, "Hashtable[\"null\"] -> nullptr");
  subset = labelledset[0];
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, subset, "BST[0] -> nullptr");
}

void test_labelledset_add(void) {
  SubSet<const char*&, int>* subset = labelledset.add(key1, 1);
  TEST_ASSERT_FALSE_MESSAGE(subset == nullptr, "New Entry: Added");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(key1, subset->key, "New Entry: Key Match");
  TEST_ASSERT_TRUE_MESSAGE(subset->contains(1), "New Entry: Value Match");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, subset->numvalues, "SubSet: Value Count");

  // BST fetch
  subset = labelledset[1];
  TEST_ASSERT_FALSE_MESSAGE(subset == nullptr, "New Entry: Found in BST");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(key1, subset->key, "New Entry (BST): Key Match");
  TEST_ASSERT_TRUE_MESSAGE(subset->contains(1), "New Entry (BST): Value Match");

  // Hashtable fetch
  subset = labelledset[key1];
  TEST_ASSERT_FALSE_MESSAGE(subset == nullptr, "New Entry: Found in HT");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(key1, subset->key, "New Entry (HT): Key Match");
  TEST_ASSERT_TRUE_MESSAGE(subset->contains(1), "New Entry (HT): Value Match");
}

void test_labelledset_overwrite(void) {
  SubSet<const char*&, int>* existing = labelledset[1];
  SubSet<const char*&, int>* subset = labelledset.add("test1", 1, false);
  // TEST_ASSERT_TRUE_MESSAGE(subset == existing, "NO Overwrite: Return Existing Device");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(key1, subset->key, "NO Overwrite: Old Key Remains");

  subset = labelledset.add("test1", 1);
  TEST_ASSERT_EQUAL_STRING_MESSAGE("test1", subset->key, "Overwrite: New Key Set");
}

void test_labelledset_addgroup(void) {
  int test[3] = {1, 2, 3};
  // Add without overwriting {1:"test1"} from the previous test
  SubSet<const char*&, int>* subset = labelledset.addSubSet("test2", test, 3, false);
  TEST_ASSERT_FALSE_MESSAGE(subset == nullptr, "Add Group");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(2, subset->numvalues, "Add Group: Device count match");
  // for (int i = 0; i < subset->numvalues; i++) {
    // if(subset->values[i] == 1) {
    //   TEST_ASSERT_EQUAL_STRING_MESSAGE("test1", subset->values[i], "Add Group: Overwrote");
    // } else {
      // TEST_ASSERT_EQUAL_STRING_MESSAGE("test2", subset->values[i], "Added entry ID match");
    // }
  // }
}

void test_labelledset_remove(void) {
  TEST_ASSERT_FALSE_MESSAGE(labelledset.remove(0), "Remove: Nonexistent entry");
  TEST_ASSERT_TRUE_MESSAGE(labelledset.remove(1), "Found entry and attempted to remove!");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, labelledset[1], "Entry removed successfully!");
  uint8_t n = labelledset["test1"]->numvalues;
  TEST_ASSERT_EQUAL_UINT_MESSAGE(0, n, "Entry removed successfully!");
  TEST_ASSERT_TRUE_MESSAGE(labelledset.remove(2), "Removed remaining group entries! (1/2)");
  TEST_ASSERT_TRUE_MESSAGE(labelledset.remove(3), "Removed remaining group entries! (2/2)");
  n = labelledset["test2"]->numvalues;
  TEST_ASSERT_EQUAL_UINT_MESSAGE(0, n, "Device group removed successfully!");
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_labelledset_empty);
  RUN_TEST(test_labelledset_add);
  RUN_TEST(test_labelledset_overwrite);
  RUN_TEST(test_labelledset_addgroup);
  RUN_TEST(test_labelledset_remove);

  UNITY_END();
}

void loop() {

}