#include "table.hpp"

bool Table::get(ObjString* key, Value* value) {
  if (count == 0) return false;

  auto entry = findEntry(entries, key);
  if (entry->key == NULL) return false;

  *value = entry->value;
  return true;
}

bool Table::set(ObjString* key, Value value) {
  if (count + 1 > entries->capacity() * TABLE_MAX_LOAD) {
    adjustCapacity(entries->capacity() * 2);
  }
  auto entry = findEntry(entries, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NIL(entry->value)) count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool Table::deleteKey(ObjString* key) {
  if (count == 0) return false;

  auto entry = findEntry(entries, key);
  if (entry->key == NULL) return false;

  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  return true;
}

Entry* findEntry(std::vector<Entry>* entries, ObjString* key) {
  uint32_t index = key->hash % entries->capacity();
  Entry* tombstone = NULL;

  while (true) {
    Entry* entry = &((*entries)[index]);
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
      return entry;
    }
    index = (index + 1) % entries->capacity();
  }
  return nullptr;
}

ObjString* Table::findString(ObjString* target) {
  if (count == 0) return nullptr;

  uint32_t index = target->hash % entries->capacity();
  while (true) {
    Entry* entry = &(*entries)[index];

    if (entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value)) return NULL;
    } else if (entry->key->str.length() == target->str.length() &&
               entry->key->hash == target->hash &&
               entry->key->str == target->str) {
      return entry->key;
    }

    index = (index + 1) % entries->capacity();
  }
}

void Table::adjustCapacity(int cap) {
  auto old = entries;
  entries = new std::vector<Entry>{};
  entries->reserve(cap);

  initializeEntries();

  count = 0;
  for (size_t i = 0; i < old->capacity(); i++) {
    Entry* src = &((*old)[i]);
    if (src->key == NULL) continue;

    Entry* dst = findEntry(entries, src->key);
    dst->key = src->key;
    dst->value = src->value;
    count++;
  }

  delete old;
};

void Table::addAll(Table* src) {
  for (size_t i = 0; i < src->entries->capacity(); i++) {
    Entry* entry = &(*src->entries)[i];
    if (entry->key != NULL) {
      set(entry->key, entry->value);
    }
  }
}