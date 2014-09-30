/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

#ifndef _ARRAY_H_
#define _ARRAY_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ======================================================================
// A resizing array.  but remove is slow & changes the element order.

template <class ARRAY_ELEMENT>
class Array {

public:

  Array(int s) {
    assert (s > 0);
    size = s;
    count = 0;
    data = new ARRAY_ELEMENT [size]; }
  virtual ~Array() { 
    delete [] data; 
    data = NULL; }

  // =========
  // ACCESSORS
  int Count() const { return count; }  
  ARRAY_ELEMENT operator [] (int i) const {
    assert (i >= 0 && i < count);
    return data[i]; }
  
  // =========
  // MODIFIERS
  void Add(const ARRAY_ELEMENT elem) {
    if (count == size) {
      // double the space & copy 
      int new_size = size*2;
   //   printf ("need to resize array from %d to %d\n", size, new_size);
      ARRAY_ELEMENT * new_data = new ARRAY_ELEMENT [new_size];
      for (int i = 0; i < size; i++) {
	new_data[i] = data[i];
      }
      delete [] data;
      data = new_data;
      size = new_size;
    }
    assert (count < size);
    data[count] = elem;
    count++;
  }

  void AddNoDuplicates(const ARRAY_ELEMENT e) { if (!Member(e)) Add(e); }

  ARRAY_ELEMENT Replace(int i, const ARRAY_ELEMENT elem) {
    assert(i >= 0 && i < count);
    ARRAY_ELEMENT answer = data[i];
    data[i] = elem;
    return answer;
  }

  void Remove(const ARRAY_ELEMENT elem) {
    int x = -1;
    for (int i = 0; i < count; i++) {
      if (data[i] == elem) {
	x = i;
	break;
      }
    }
    assert (x >= 0);
    data[x] = data[count-1];
    count--;    
  }

  int Member(const ARRAY_ELEMENT elem) const {
    int x = -1;
    for (int i = 0; i < count; i++) {
      if (data[i] == elem) {
	x = i;
	break;
      }
    }
    if (x >= 0) return 1;
    return 0;
  }

  void Clear() {
    count = 0;
  }

  void DeleteAllElements() {
    for (int i = 0; i < count; i++) {
      delete data[i];
    }
    count = 0;
  }

private:

  // ==============
  // REPRESENTATION
  int count;
  int size;
  ARRAY_ELEMENT * data;

};


#endif
