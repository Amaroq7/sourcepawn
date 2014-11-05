/* vim: set ts=2 sw=2 tw=99 et:
 *
 * Copyright (C) 2012-2014 AlliedModders LLC, David Anderson
 *
 * This file is part of SourcePawn.
 *
 * SourcePawn is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * SourcePawn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SourcePawn. If not, see http://www.gnu.org/licenses/.
 */
#ifndef _include_jitcraft_string_pool_h_
#define _include_jitcraft_string_pool_h_

#include "pool-allocator.h"
#include <am-hashtable.h>
#include <am-hashset.h>
#include <am-string.h>
#include <string.h>

namespace ke {

// Wrapper around AString, since we might want to remove charfs() in a GC-safe
// implementation.
class Atom
{
  friend class StringPool;

 private:
  Atom(const char *str, size_t len)
   : str_(str, len)
  {
  }

 public:
  size_t length() const {
    return str_.length();
  }
  const char *chars() const {
    return str_.chars();
  }

 private:
  AString str_;
};

class CharsAndLength
{
  public:
    CharsAndLength()
      : str_(nullptr),
        length_(0)
    {
    }

    CharsAndLength(const char *str, size_t length)
      : str_(str),
        length_(length)
    {
    }

    const char *str() const {
        return str_;
    }
    size_t length() const {
        return length_;
    }

  private:
    const char *str_;
    size_t length_;
};

class StringPool
{
  public:
    StringPool()
      : table_(SystemAllocatorPolicy())
    {
    }

    ~StringPool()
    {
      if (!table_.elements())
        return;
      for (Table::iterator i(&table_); !i.empty(); i.next())
        delete *i;
    }

    bool init() {
      return table_.init(256);
    }

    Atom *add(const char *str, size_t length) {
      CharsAndLength chars(str, length);
      Table::Insert p = table_.findForAdd(chars);
      if (!p.found() && !table_.add(p, new Atom(str, length)))
        return nullptr;
      return *p;
    }

    Atom *add(const char *str) {
      return add(str, strlen(str));
    }

  private:
    struct Policy {
      typedef Atom *Payload;

      static uint32_t hash(const char *key) {
        return HashCharSequence(key, strlen(key));
      }

      static uint32_t hash(const CharsAndLength &key) {
        return HashCharSequence(key.str(), key.length());
      }

      static bool matches(const CharsAndLength &key, const Payload &e) {
        if (key.length() != e->length())
          return false;
        return strcmp(key.str(), e->chars()) == 0;
      }
    };

    typedef HashTable<Policy> Table;

  private:
    Table table_;
};

struct AtomHashPolicy {
  static uint32_t hash(Atom *p) {
    return HashPointer(p);
  }
  static bool matches(Atom *a, Atom *b) {
    return a == b;
  }
};

class AtomSet : public HashSet<Atom *, AtomHashPolicy, SystemAllocatorPolicy>
{
 public:
  AtomSet() {
    init(16);
  }
};

} // namespace ke

#endif // _include_jitcraft_string_pool_h_


