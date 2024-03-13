# Smart SpMV (SSpMV)
**Smart SpMV** is a library for SpMV implemented by different sparse matrix format.
It can adaptively select the optimal performance format and algorithm to compute SpMV operation.
Right now, the most of kernel code can deploy to different CPU platforms (have tested successfully on both Intel and ArmV8).
The parallelism of these SpMV algorithms are achieved by OpenMP, the SIMD optimizations rely on `#pragma omp simd` and Intel compiler's option (see *CMakeLists.txt*).

## Repository Structure


### LeSPMV
This library is designed for different platforms. It can auto-detect the hardware information and write the configurations into `./include/plat_config.h`. The code structure is:

`include`: Contains the head files for LeSpMV library. 

`src`: Contains different SpMV kernels implementations.

`utils`: Contains some auxiliary function routines including memory IO, sparse matrix reading and test helper.

`test`: Contains some test routines that can check the results' correctness and performance for different sparse matrix format.

`features`: Prepared for storing the matrix features extracted by `Get_features`.

## Compilation and Installation
Typically, SSpMV library is easy to install. Please follow the steps:
```
cd LeSpMV
mkdir build && cd build
cmake ..
make
```
Then you will get some test routines of different SpMV algorithms. If some compiling errors occur, please see *CMakeLists.txt* for compiling details.

## Supported Matrix Format
- **COO** : The COO is also known as the transactional format. In this format, the matrix is represented as a set of triples , where x is an entry in the matrix and i and j denote its row and column indices, respectively.
- **CSR** : The most common format for CSRs is the [PKCS #10](https://en.wikipedia.org/wiki/Certificate_signing_request) specification; others include the more capable Certificate Request Message Format (CRMF) and the SPKAC (Signed Public Key and Challenge) format generated by some web browsers.
- **BSR** : The Block Sparse Row (BSR) format is very similar to the Compressed Sparse Row (CSR) format, which is mainly used in intel MKL solver. A non-zero block is the block that contains at least one non-zero element. Please click [here](http://z-s.xyz/2019/04/20/Block-Compressed-Sparse-Row-BSR-Matrix-Format/) for more details about this format.
- **CSR5** : An complicated version of CSR format to exploit SIMD acceleration on AVX-512. This format only support the Intel CPU. Please click [here](https://arxiv.org/abs/1503.05032) for the whole paper.
- **DIA** : Diagonal Storage (DIA) format. Please click [here](https://phys.libretexts.org/Bookshelves/Mathematical_Physics_and_Pedagogy/Computational_Physics_(Chong)/08%3A_Sparse_Matrices/8.02%3A_Sparse_Matrix_Formats) for a quick introduction.
- **ELL** : The traditional ELL format compresses the matrix data into a rectangular dense matrix, and adds zeros to force each column in the matrix to have the same number of elements. The matrix data is then stored column by column, followed by their column indices in the original sparse matrix.
- **S-ELL** : Sliced ELL format. Multiple rows are packed into a row block for the ELL storage. Please click [here](https://library.eecs.utk.edu/storage/files/ut-eecs-14-727.pdf) for more detailed implementation. **Parameters: chunk width C.**
- SELL-c- $\sigma$ : Sliced ELL format with $\sigma$ slice for reordering. **Parameters: chunk width C, slice width $\sigma$**.
- SELL-c-R : Here the $\sigma=R$, reorder the whole matrix rows without sliced tiles.

## Matrix Features
To adaptive select the optimal algorithm for different sparse matrices. We need to extract some representative features for our deep learning model. Here we list these features as a reference.
### Basic matrix features
1. The number of rows (M);
2. The number of columns (N);
3. The number of none zeros (nnzs);
4. The ratio of nnzs ($nnzs/M*N$);
5. Whether the matrix is symmetric;
6. The ratio of pattern symmetric ( a[i][j]!=0 && a[j][i]!=0 &check; );
7. The ratio of numerical symmetric ( a[i][j] == a[j][i] &check; ).

### Rows/Cols statistic features
1. The ratio of no empty rows/cols; (number of no empty rows/M or cols/N);
2. The minimum number of nnzs in each rows/cols;
3. The maximum number of nnzs in each rows/cols;
4. The average number of nnzs in rows/cols;
5. The variance of nnzs in rows/cols;
6. The standard deviation of nnzs in rows/cols;
7. P-ratio of nnzs in rows/cols; (p fraction of rows/cols have (1-p) fraction of nnzs in the matrix $[0, 0.5]$ -> [imbalanced ~ balanced] )
8. Gini coefficient of nnzs in rows/cols; ($[0, 1]$ -> [balanced ~ imbalanced]).

The number 7 and 8 features is learned from this [paper](https://dl.acm.org/doi/abs/10.1145/3572848.3577506).

### RowBlock(RB)/ColBlock(CB)/Tiles(T) statistic features
Now the default TILEs number is 2048 * 2048, which means `RB num = 2048`, `CB num = 2048`, `TILE num = 2048*2048`.
1. The ratio of none empty RB/CB/T; (number of no empty RB/Block_num, no empty CB/Block_num, no empty Tiles/Tiles_num);
2. The minimum number of nnzs in each RB/CB/T;
3. The maximum number of nnzs in each RB/CB/T;
4. The average number of nnzs in RB/CB/T;
5. The variance of nnzs in RB/CB/T;
6. The standard deviation of nnzs in RB/CB/T;
7. P-ratio of nnzs in RB/CB/T; 
8. Gini coefficient of nnzs in RB/CB/T;

#### Some extra features from Tiles:
- uniqR and uniqC : Count the sum of none zero Rows/Cols inside each tile, and then the sum is divided by total_nnz.
![uniqR and uniqC]()

### Numerical stability features
1. The max values on the matrix diagonal;
2. The max values on the off-diagonal;
3. The diagonal dominance ratio;
4. The variability of rows (MAX $log_{10}\frac{max_j|a_{i,j}|}{min_j|a_{i,j}|}$);
5. The variability of columns (MAX $log_{10}\frac{max_i|a_{i,j}|}{min_i|a_{i,j}|}$).