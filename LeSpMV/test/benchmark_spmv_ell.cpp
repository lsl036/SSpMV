/**
 * @file benchmark_spmv_ell.cpp for running the ell test routine.
 * @author Shengle Lin (lsl036@hnu.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2023-11-17
 * 
 * @copyright Copyright (c) 2023
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
    std::cout << "\t" << " --Index     = 0 (int:default) or 1 (long long)\n";
    std::cout << "\t" << " --precision = 32(or 64)\n";
    std::cout << "\t" << " --ld        = row(or col) for leading dimension\n";
    std::cout << "\t" << " --sche      = chosing the schedule strategy\n";
    std::cout << "\t" << "               0: static | 1: static, CHUNK_SIZE | 2: dynamic | 3: guided\n";
    std::cout << "\t" << " --threads= define the num of omp threads\n";
    std::cout << "Note: my_matrix.mtx must be real-valued sparse matrix in the MatrixMarket file format.\n"; 
}

template <typename IndexType, typename ValueType>
void run_ell_kernels(int argc, char **argv)
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

    if constexpr(std::is_same<IndexType, int>::value) {
        printf("Using %d-by-%d matrix with %d nonzero values", csr.num_rows, csr.num_cols, csr.num_nnzs);
    }
    else if constexpr(std::is_same<IndexType, long long>::value) {
        printf("Using %lld-by-%lld matrix with %lld nonzero values", csr.num_rows, csr.num_cols, csr.num_nnzs);
    }


    fflush(stdout);

    LeadingDimension ld_test = RowMajor;
    char * ld_str = get_argval(argc, argv, "ld");
    if(ld_str != NULL)
    {
        if ( strcmp(ld_str, "row") == 0)
        {
            ld_test = RowMajor;
        }
        else if (strcmp(ld_str, "col")== 0)
        {
            ld_test = ColMajor;
        }
        else{
            std::cout << "Wrong input of Leading Dimension! Default:row" << std::endl;
        }
    }

    if(ld_test == RowMajor)
        std::cout << " , ELL matrix store in *RowMajor*" << std::endl;
    else
        std::cout << " , ELL matrix store in *ColMajor*" << std::endl;

    int sche_mode = 0;
    char * schedule_str = get_argval(argc, argv, "sche");
    if(schedule_str != NULL)
    {
        sche_mode = atoi(schedule_str);
        if (sche_mode!=0 && sche_mode!=1 && sche_mode!=2 && sche_mode!=3)
        {
            std::cout << "sche must be [0,1,2,3]. '--help see more details'" << std::endl;
            return ;
        }
    }
    
    for(IndexType methods = 0; methods <= 2; ++methods){
        test_ell_matrix_kernels(csr, methods, ld_test, sche_mode);
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

    int Index = 0;
    char * Index_str = get_argval(argc, argv, "Index");
    if(Index_str != NULL)
        Index = atoi(Index_str);

    printf("\nUsing %d-bit floating point precision, %d-bit Index, threads = %d\n\n", precision, (Index+1)*32 , Le_get_thread_num());

    if (Index == 0 && precision ==  32){
        run_ell_kernels<int, float>(argc,argv);
    }
    else if (Index == 0 && precision == 64){
        run_ell_kernels<int, double>(argc,argv);
    }
    else if (Index == 1 && precision ==  32){
        run_ell_kernels<long long, float>(argc,argv);
    }
    else if (Index == 1 && precision == 64){
        run_ell_kernels<long long, double>(argc,argv);
    }
    else{
        usage(argc, argv);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
