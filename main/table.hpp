#ifndef cpplox_table_h
#define cpplox_table_h

#include "common.hpp"
#include "object.hpp"
#include "value.hpp"

#define TABLE_INITIAL_CAPACITY 8
#define TABLE_MAX_LOAD 0.75

class Entry {
 public:
  ObjString* key;
  Value value;
};

class Table {
 public:
  int count;
  std::vector<Entry>* entries;
  Table() : count(0), entries(new std::vector<Entry>()) {
    entries->reserve(TABLE_INITIAL_CAPACITY);
    initializeEntries();
  };

  void initializeEntries() {
    for (size_t i = 0; i < entries->capacity(); i++) {
      (*entries)[i].key = NULL;
      (*entries)[i].value = NIL_VAL;
    }
  }

  bool get(ObjString* key, Value* value);
  bool set(ObjString* key, Value value);
  bool deleteKey(ObjString* key);
  void adjustCapacity(int cap);
  ObjString* findString(ObjString* target);
  void addAll(Table* src);
};

Entry* findEntry(std::vector<Entry>* entries, ObjString* key);

#endif
