//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Contact: Jan Ciesko (jciesko@sandia.gov)
//
//@HEADER

#ifndef KOKKOS_REMOTESPACES_VIEWMAPPING_HELPER_HPP
#define KOKKOS_REMOTESPACES_VIEWMAPPING_HELPER_HPP

#include <type_traits>

namespace Kokkos {
namespace Experimental {
namespace Impl {

template <class T>
struct Is_Partitioned_Layout {
  enum : bool {
    value = std::is_base_of<Kokkos::PartitionedLayout,
                            typename T::array_layout>::value
  };
};

template <class T>
struct Is_View_Of_Type_RemoteSpaces {
  enum : bool {
    value = std::is_same<typename T::traits::specialize,
                         Kokkos::Experimental::RemoteSpaceSpecializeTag>::value
  };
};

template <class T>
struct Is_Dim0_Used_As_PE {
  enum : bool {
    value = RemoteSpaces_MemoryTraits<typename T::memory_traits>::dim0_is_pe
  };
};
/*
template <class view_type>
bool is_local_view(
    view_type v,
    std::enable_if_t<Is_View_Of_Type_RemoteSpaces<view_type>::value &&
Is_Partitioned_Layout<view_type>::value> * = nullptr) { return
(v.impl_map().get_owning_pe.dim0_idxOffset == v.impl_map().get_my_PE());
}*/

template <class view_type>
bool is_local_view(
    view_type v,
    std::enable_if_t<Is_View_Of_Type_RemoteSpaces<
        view_type>::value /* && !Is_Partitioned_Layout<view_type>::value*/> * =
        nullptr) {
  return (v.impl_map().get_owning_pe() == v.impl_map().get_my_PE());
}

template <class view_type>
bool is_local_view(
    view_type v,
    std::enable_if_t<!Is_View_Of_Type_RemoteSpaces<view_type>::value> * =
        nullptr) {
  return true;
}

template <class T>
struct RemoteSpaces_View_Properties {
  /* Is the first index denoting PE*/
  bool dim0_isPE;
  /* Is this view a subview of another view*/
  bool is_subview;
  /* Index offset in dim0 */
  T dim0_idxOffset;
  /* Num local elems in dim0  */
  T dim0_localSize;
  /* Com size and rank*/
  int num_PEs;
  int my_PE;

  KOKKOS_FUNCTION
  RemoteSpaces_View_Properties() {
    dim0_isPE      = true; /* default is true */
    is_subview     = false;
    dim0_idxOffset = 0;
    dim0_localSize = 0;
    num_PEs        = Kokkos::Experimental::get_num_pes();
    my_PE          = Kokkos::Experimental::get_my_pe();
  }

  KOKKOS_FUNCTION
  RemoteSpaces_View_Properties(const RemoteSpaces_View_Properties &rhs) {
    dim0_isPE      = rhs.dim0_isPE;
    is_subview     = rhs.is_subview;
    dim0_idxOffset = rhs.dim0_idxOffset;
    dim0_localSize = rhs.dim0_localSize;
    num_PEs        = rhs.num_PEs;
    my_PE          = rhs.my_PE;
  }

  KOKKOS_FUNCTION RemoteSpaces_View_Properties &operator=(
      const RemoteSpaces_View_Properties &rhs) {
    dim0_isPE      = rhs.dim0_isPE;
    is_subview     = rhs.is_subview;
    dim0_idxOffset = rhs.dim0_idxOffset;
    dim0_localSize = rhs.dim0_localSize;
    num_PEs        = rhs.num_PEs;
    my_PE          = rhs.my_PE;
    return *this;
  }
};

}  // namespace Impl

template <typename T>
KOKKOS_INLINE_FUNCTION auto get_indexing_block_size(T size) {
  auto num_pes = Kokkos::Experimental::get_num_pes();
  auto block   = (size + static_cast<T>(num_pes) - 1) / num_pes;
  return block;
}

template <typename T>
KOKKOS_INLINE_FUNCTION Kokkos::pair<T, T> getRange(T size, int pe) {
  auto block = get_indexing_block_size(size);
  auto start = static_cast<T>(pe) * block;
  auto end   = (static_cast<T>(pe) + 1) * block;

  auto num_pes = Kokkos::Experimental::get_num_pes();
  if (size < num_pes) {
    T diff = (num_pes * block) - size;
    if (pe > num_pes - 1 - diff) end--;
  } else {
    if (pe == num_pes - 1) {
      size_t diff = size - (num_pes - 1) * block;
      end         = start + diff;
    }
  }
  return Kokkos::pair<T, T>(start, end);
}

template <typename T>
KOKKOS_INLINE_FUNCTION Kokkos::pair<T, T> get_range(T size, int pe) {
  return getRange(size, pe);
}

template <typename T>
KOKKOS_INLINE_FUNCTION Kokkos::pair<T, T> get_local_range(T size) {
  auto pe = Kokkos::Experimental::get_my_pe();
  return getRange(size, pe);
}

}  // namespace Experimental
}  // namespace Kokkos

#endif  // KOKKOS_REMOTESPACES_VIEWMAPPING_HELPER_HPP
