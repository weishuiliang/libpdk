// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2017/12/07.

#ifndef PDK_M_BASE_DS_INTERNAL_ARRAYDATA_OPERATOR_H
#define PDK_M_BASE_DS_INTERNAL_ARRAYDATA_OPERATOR_H

#include "ArrayData.h"
#include "pdk/global/TypeInfo.h"

#include <new>
#include <string.h>

namespace pdk {
namespace ds {
namespace internal {

template <typename T>
struct PodArrayOperator : TypedArrayData<T>
{
   void appendInitialize(size_t newSize)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(newSize > static_cast<uint>(this->m_size));
      PDK_ASSERT(newSize <= static_cast<uint>(this->m_alloc));
      std::memset(this->end(), 0, (newSize - this->m_size));
      this->m_size = static_cast<int>(newSize);
   }
   
   void copyAppend(const T *begin, const T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(begin < end);
      PDK_ASSERT(static_cast<size_t>(end - begin) <= this->m_alloc - static_cast<uint>(this->m_size));
      std::memcpy(this->end(), begin, (end - begin) * sizeof(T));
      this->m_size += end - begin;
   }
   
   void copyAppend(size_t n, const T &target)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(static_cast<size_t>(end - begin) <= this->m_alloc - static_cast<uint>(this->m_size));
      T *iterator = this->end();
      const T *const end = iterator + n;
      for (; iterator != end; ++iterator) {
         std::memcpy(iterator, &target, sizeof(T));
      }
      this->m_size += static_cast<int>(n);
   }
   
   void truncate(size_t newSize)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(newSize < static_cast<size_t>(this->m_size));
      this->m_size = static_cast<int>(newSize);
   }
   
   void destroyAll()
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(this->m_ref.m_atomic.load() == 0);
      // As this is to be called only from destructor, it doesn't need to be
      // exception safe; size not updated.
   }
   
   void insert(T *where, const T *begin, const T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(where >= this->begin() && where < this->end());
      PDK_ASSERT(begin < end);
      PDK_ASSERT(where >= end || begin > this->end());
      PDK_ASSERT(static_cast<size_t>(end - begin) <= this->m_alloc - static_cast<uint>(this->m_size));
      std::memmove(where + (end - begin), where, (static_cast<const T*>(this->end()) - where) * sizeof(T));
      std::memcpy(where, begin, (end - begin) * sizeof(T));
      this->m_size += (end - begin);
   }
   
   void erase(T *begin, T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(begin < end);
      PDK_ASSERT(begin >= this->begin() && begin < this->end());
      PDK_ASSERT(end > this->begin() && end < this->end());
      std::memmove(begin, end, (static_cast<T *>(this->end()) - end) * sizeof(T));
      this->m_size -= (end - begin);
   }
};

template <typename T>
struct GenericArrayOperator : TypedArrayData<T>
{
   void appendInitialize(size_t newSize)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(newSize > static_cast<uint>(this->m_size));
      PDK_ASSERT(newSize <= this->m_alloc);
      T *const begin = this->begin();
      do {
         new (begin + this->m_size) T();
      } while (static_cast<size_t>(++this->m_size) != newSize);
   }
   
   void copyAppend(const T *begin, const T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(begin < end);
      PDK_ASSERT(static_cast<size_t>(end - begin) <= this->m_alloc - static_cast<size_t>(this->m_size));
      T *iterator = this->end();
      for (; begin != end; ++iterator, ++begin) {
         new (iterator) T(*begin);
         ++this->m_size;
      }
   }
   
   void copyAppend(size_t n, const T &data)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(n <= this->m_alloc - static_cast<uint>(this->m_size));
      T *iterator = this->end();
      const T *const end = iterator + n;
      for (; iterator != end; ++iterator) {
         new (iterator) T(data);
         ++this->m_size;
      }
   }
   
   void truncate(size_t newSize)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(newSize < static_cast<size_t>(this->m_size));
      const T * const begin = this->begin();
      do {
         static_cast<T *>((begin + (--this->m_size)))->~T();
      } while (static_cast<size_t>(this->m_size) != newSize);
   }
   
   void destroyAll()
   {
      PDK_ASSERT(this->isMutable());
      // As this is to be called only from destructor, it doesn't need to be
      // exception safe; size not updated.
      PDK_ASSERT(this->m_ref.m_atomic.load() == 0);
      const T *const begin = this->begin();
      const T *iterator = this->end();
      while (iterator != begin) {
         static_cast<T *>(--iterator)->~T();
      }
   }
   
   void insert(T *where, const T *begin, const T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(!this->m_ref.isShared());
      PDK_ASSERT(where >= this->begin() && where < this->end());
      PDK_ASSERT(static_cast<size_t>(end - begin));
      PDK_ASSERT(begin < end);
      PDK_ASSERT(end <= where || begin > this->end());
      // Array may be truncated at where in case of exceptions
      T *const selfEnd = this->end();
      const T *readIterator = selfEnd;
      T *writerIterator = selfEnd + (end - begin);
      const T *const step1End = where + std::max(end - begin, selfEnd  - where);
      struct Destructor
      {
         Destructor(T *&iterator)
            : m_iterator(&iterator),
              m_end(iterator)
         {}
         
         void commit()
         {
            m_iterator = &m_end;
         }
         
         ~Destructor()
         {
            for (; *m_iterator != m_end; --*m_iterator) {
               static_cast<T *>(*m_iterator)->~T();
            }
         }
         
         T **m_iterator;
         T *m_end;
      } destroyer(writeIter);
      
      // Construct new elements in array
      do {
         --readIterator;
         --writerIterator;
         new (writerIterator) T(*readIterator);
      } while (writerIterator != step1End);
      
      while (writerIterator != selfEnd) {
         --end;
         --writerIterator;
         new (writerIterator) T(*end);
      }
      
      destroyer.commit();
      this->m_size = destroyer.m_end - selfEnd;
      while (writerIterator != selfEnd) {
         --readIterator;
         --writerIterator;
         *writerIterator = *readIterator;
      }
      
      while (writerIterator != where) {
         --end;
         --writerIterator;
         *writerIterator = *end;
      }
   }
   
   void erase(T *begin, T *end)
   {
      PDK_ASSERT(this->isMutable());
      PDK_ASSERT(begin < end);
      PDK_ASSERT(begin >= this->begin() && begin < this->end());
      PDK_ASSERT(end > this->begin() && end < this->end());
      const T *const selfEnd = this->end();
      do {
         *begin = *end;
         ++begin;
         ++end;
      } while (end != selfEnd);
      
      do {
         static_cast<T *>(--end)->~T();
         --this->m_size;
      } while (end != begin);
   }
};

template <typename T>
struct MovableArrayOperator : TypedArrayData<T>
{
   void appendInitialize(size_t newSize)
   {
      
   }
   
   void copyAppend(const T *begin, const T *end)
   {
      
   }
   
   void copyAppend(size_t n, const T &target)
   {
      
   }
   
   void truncate(size_t newSize)
   {
      
   }
   
   void destroyAll()
   {
      
   }
   
   void insert(T *where, const T *begin, const T *end)
   {
      
   }
   
   void erase(T *begin, T *end)
   {
      
   }
};

template <typename T, typename = void>
struct ArrayOperatorSelector
{
   using Type = GenericArrayOperator<T>;
};

template <typename T>
struct ArrayOperatorSelector<T, 
      typename std::enable_if<!pdk::TypeInfo<T>::isComplex && !pdk::TypeInfo<T>::isStatic>::type>
{
   using Type = PodArrayOperator;
};

template <typename T>
struct ArrayOperatorSelector<T, 
      typename std::enable_if<pdk::TypeInfo<T>::isComplex && !pdk::TypeInfo<T>::isStatic>::type>
{
   using Type = MovableArrayOperator;
};

template <typename T>
struct ArrayDataOperator : ArrayOperatorSelector<T>::Type
{};

} // internal
} // ds
} // pdk

#endif // PDK_M_BASE_DS_INTERNAL_ARRAYDATA_OPERATOR_H
