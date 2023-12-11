/**
 * @file test_spmv_dia.cpp
 * @author Shengle Lin (lsl036@hnu.edu.cn)
 * @brief   Test routine for spmv_dia.cpp
 * @version 0.1
 * @date 2023-11-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include"../include/LeSpMV.h"
#include<iostream>

template <typename IndexType, typename ValueType>
int test_dia_matrix_kernels(const CSR_Matrix<IndexType,ValueType> &csr_ref, int kernel_tag)
{
    std::cout << "=====  Testing ELL Kernels  =====" << std::endl;

    DIA_Matrix<IndexType,ValueType> dia;

    IndexType max_diags = MAX_DIAG_NUM;
    IndexType alignment = 16;
    FILE* save_features = fopen(MAT_FEATURES,"w");

    dia = csr_to_dia(csr_ref, max_diags, save_features, alignment);

    fclose(save_features);
    // 测试这个routine 要我们测的 kernel_tag
    dia.kernel_flag = kernel_tag;

    if(0 == dia.kernel_flag){
        std::cout << "\n===  Compared DIA serial with csr default  ===" << std::endl;
        // test correctness
        test_spmv_kernel(csr_ref, LeSpMV_csr<IndexType, ValueType>,
                         dia, LeSpMV_dia<IndexType, ValueType>,
                         "dia_serial_simple");

        std::cout << "\n===  Performance of DIA serial simple  ===" << std::endl;
        // count performance of Gflops and Gbytes
        benchmark_spmv_on_host(dia, LeSpMV_dia<IndexType, ValueType>,"dia_serial_simple");
    }
    else if (1 == dia.kernel_flag)
    {
        std::cout << "\n===  Compared DIA omp with csr default  ===" << std::endl;
        // test correctness
        test_spmv_kernel(csr_ref, LeSpMV_csr<IndexType, ValueType>,
                         dia, LeSpMV_dia<IndexType, ValueType>,
                         "dia_omp_simple");

        std::cout << "\n===  Performance of DIA omp simple  ===" << std::endl;
        // count performance of Gflops and Gbytes
        benchmark_spmv_on_host(dia, LeSpMV_dia<IndexType, ValueType>,"dia_omp_simple");
          
    }

    delete_dia_matrix(dia);
    return 0;
}

template int test_dia_matrix_kernels<int,float>(const CSR_Matrix<int,float> &csr_ref, int kernel_tag);

template int test_dia_matrix_kernels<int,double>(const CSR_Matrix<int,double> &csr_ref, int kernel_tag);