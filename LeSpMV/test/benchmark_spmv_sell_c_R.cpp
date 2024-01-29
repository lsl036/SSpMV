/**
 * @file benchmark_spmv_sell_c_R.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include<iostream>
#include<cstdio>
#include"../include/LeSpMV.h"
#include"../include/cmdline.h"

void usage(int argc, char** argv)
{
    std::cout << "Usage:\n";
    std::cout << "\t" << argv[0] << " with following parameters:\n";
    std::cout << "\t" << " my_matrix.mtx\n";
    std::cout << "\t" << " --precision = 32(or 64)\n";
    std::cout << "\t" << " --ld        = is only supported Row-major format\n";
    std::cout << "\t" << " --sche      = chosing the schedule strategy\n";
    std::cout << "\t" << "               0: static | 1: static, CHUNK_SIZE | 2: dynamic | 3: guided\n";
    std::cout << "\t" << " --threads   = define the num of omp threads\n";
    std::cout << "Note: my_matrix.mtx must be real-valued sparse matrix in the MatrixMarket file format.\n"; 
}

template <typename IndexType, typename ValueType>
void run_sell_c_R_kernels(int argc, char **argv)
{
    char * mm_filename = NULL;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] != '-'){
            mm_filename = argv[i];
            break;
        }
    }

    if(mm_filename == NULL)
    {
        printf("You need to input a matrix file!\n");
        return;
    }

    // reference CSR kernel for ell test
    CSR_Matrix<IndexType, ValueType> csr;
    csr = read_csr_matrix<IndexType, ValueType> (mm_filename);
    printf("Using %d-by-%d matrix with %d nonzero values", csr.num_rows, csr.num_cols, csr.num_nnzs); 

    fflush(stdout);

    int sche_mode = SCHE_MODE;

    // 此时 0 == 1 都是 StCont 方式，因为按照本身的chunk划分
    char * schedule_str = get_argval(argc, argv, "sche");
    if(schedule_str != NULL)
    {
        sche_mode = atoi(schedule_str);
        if ( sche_mode!=1 && sche_mode!=2 && sche_mode!=3)
        {
            std::cout << "sche must be [1,2,3]. '--help see more details'" << std::endl;
            return ;
        }
    }

    std::cout << " , SELL-c-R matrix only support store in *RowMajor*" << std::endl;

    for(IndexType methods = 0; methods < 2; ++methods){
        test_sell_c_R_matrix_kernels(csr, methods, sche_mode);
        fflush(stdout);
    }

    delete_csr_matrix(csr);
}

int main(int argc, char** argv)
{
    if (get_arg(argc, argv, "help") != NULL){
        usage(argc, argv);
        return EXIT_SUCCESS;
    }

    int precision = 32;
    char * precision_str = get_argval(argc, argv, "precision");
    if(precision_str != NULL)
        precision = atoi(precision_str);

    // 包括超线程
    Le_set_thread_num(CPU_SOCKET * CPU_CORES_PER_SOC * CPU_HYPER_THREAD);

    char * threads_str = get_argval(argc, argv, "threads");
    if(threads_str != NULL)
        Le_set_thread_num(atoi(threads_str));

    printf("\nUsing %d-bit floating point precision, threads = %d\n\n", precision, Le_get_thread_num());

    if(precision ==  32){
        // run_all_kernels<int, float>(argc,argv);
        run_sell_c_R_kernels<int, float>(argc,argv);
    }
    else if(precision == 64){
        // run_all_kernels<int, double>(argc,argv);
        run_sell_c_R_kernels<int, double>(argc,argv);
    }
    else{
        usage(argc, argv);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
