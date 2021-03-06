#pragma once
#ifndef __ICHOL_LEFT_BY_BLOCKS_HPP__
#define __ICHOL_LEFT_BY_BLOCKS_HPP__

/// \file ichol_left_by_blocks.hpp
/// \brief Sparse incomplete Cholesky factorization by blocks.
/// \author Kyungjoo Kim (kyukim@sandia.gov)

namespace Example { 

  using namespace std;

  template<typename CrsTaskViewType>
  KOKKOS_INLINE_FUNCTION
  static int genGemmTasks_LowerLeftByBlocks(typename CrsTaskViewType::policy_type &policy,
                                            const CrsTaskViewType A,
                                            const CrsTaskViewType X,
                                            const CrsTaskViewType Y);
  
  template<typename CrsTaskViewType>
  KOKKOS_INLINE_FUNCTION
  static int genScalarTask_LowerLeftByBlocks(typename CrsTaskViewType::policy_type &policy,
                                             const CrsTaskViewType A);

  template<typename CrsTaskViewType>
  KOKKOS_INLINE_FUNCTION
  static int genTrsmTasks_LowerLeftByBlocks(typename CrsTaskViewType::policy_type &policy,
                                            const CrsTaskViewType A,
                                            const CrsTaskViewType B);

  template<>
  template<typename CrsTaskViewType>
  KOKKOS_INLINE_FUNCTION
  int 
  IChol<Uplo::Lower,AlgoIChol::LeftByBlocks>
  ::invoke(const CrsTaskViewType A) {
    typename CrsTaskViewType::policy_type policy;

    CrsTaskViewType ATL, ATR,      A00, A01, A02,
      /**/          ABL, ABR,      A10, A11, A12,
      /**/                         A20, A21, A22;
    
    Part_2x2(A,  ATL, ATR,
             /**/ABL, ABR, 
             0, 0, Partition::TopLeft);
    
    while (ATL.NumRows() < A.NumRows()) {
      Part_2x2_to_3x3(ATL, ATR, /**/  A00, A01, A02,
                      /*******/ /**/  A10, A11, A12,
                      ABL, ABR, /**/  A20, A21, A22,  
                      1, 1, Partition::BottomRight);
      // -----------------------------------------------------
      CrsTaskViewType AB0, AB1;
      
      Merge_2x1(A11,
                A21, AB1);
      
      Merge_2x1(A10,
                A20, AB0);

      // A11 = A11 - A10 * A10'
      // A21 = A21 - A20 * A10'
      genGemmTasks_LowerLeftByBlocks(policy, AB0, A10, AB1);

      // A11 = chol(A11)
      genScalarTask_LowerLeftByBlocks(policy, A11);

      // A21 = A21 * inv(tril(A11)')
      genTrsmTasks_LowerLeftByBlocks(policy, A11, A21);

      // -----------------------------------------------------
      Merge_3x3_to_2x2(A00, A01, A02, /**/ ATL, ATR,
                       A10, A11, A12, /**/ /******/
                       A20, A21, A22, /**/ ABL, ABR,
                       Partition::TopLeft);
    }

    return 0;
  }

}

#include "ichol_left_by_blocks_var1.hpp"

#endif
