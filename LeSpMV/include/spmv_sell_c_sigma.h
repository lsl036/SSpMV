#ifndef SPMV_SELL_C_SIGMA_H
#define SPMV_SELL_C_SIGMA_H

#include "sparse_format.h"

template <typename IndexType, typename ValueType>
void __spmv_sell_cs_serial_simple( const IndexType * Reorder,
                                   const IndexType num_rows,
                                   const IndexType chunk_rowNum,
                                   const IndexType total_chunk_num,
                                   const ValueType alpha,
                                   const IndexType *max_row_width,
                                   const IndexType * const *col_index,
                                   const ValueType * const *values,
                                   const ValueType * x, 
                                   const ValueType beta, 
                                   ValueType * y);

template <typename IndexType, typename ValueType>
void __spmv_sell_cs_omp_simple( const IndexType * Reorder,
                                const IndexType num_rows,
                                const IndexType chunk_rowNum,
                                const IndexType total_chunk_num,
                                const ValueType alpha,
                                const IndexType *max_row_width,
                                const IndexType * const *col_index,
                                const ValueType * const *values,
                                const ValueType * x, 
                                const ValueType beta, 
                                ValueType * y);

/**
 * @brief Compute y += alpha * A * x + beta * y for a sparse matrix
 *        Matrix Format: SELL-c-sigma
 *        Inside call : __spmv_sell_c_sigma_omp_simple() to calculation
 * 
 * @tparam IndexType 
 * @tparam ValueType 
 * @param alpha 
 * @param sell_c_sigma 
 * @param x 
 * @param beta 
 * @param y 
 */
template <typename IndexType, typename ValueType>
void LeSpMV_sell_c_sigma(const ValueType alpha, const SELL_C_Sigma_Matrix<IndexType, ValueType>& sell_c_sigma, const ValueType *x, const ValueType beta, ValueType *y);

#endif /* SPMV_SELL_C_SIGMA_H */
