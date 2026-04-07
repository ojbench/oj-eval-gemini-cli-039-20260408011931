#ifndef CSR_MATRIX_HPP
#define CSR_MATRIX_HPP

#include <vector>
#include <exception>

namespace sjtu {

class size_mismatch : public std::exception {
public:
    const char *what() const noexcept override {
        return "Size mismatch";
    }
};

class invalid_index : public std::exception {
public:
    const char *what() const noexcept override {
        return "Index out of range";
    }
};

// TODO: Implement a CSR matrix class
// You only need to implement the TODOs in this file
// DO NOT modify other parts of this file
// DO NOT include any additional headers
// DO NOT use STL other than std::vector

template <typename T>
class CSRMatrix {

private:
    // TODO: add your private member variables here
    size_t n_rows;
    size_t n_cols;
    size_t nnz;
    std::vector<size_t> indptr_arr;
    std::vector<size_t> indices_arr;
    std::vector<T> data_arr;

public:
    // Assignment operators are deleted
    CSRMatrix &operator=(const CSRMatrix &other) = delete;
    CSRMatrix &operator=(CSRMatrix &&other) = delete;

    // Constructor for empty matrix with dimensions
    // TODO: Initialize an empty CSR matrix with n rows and m columns
    CSRMatrix(size_t n, size_t m) : n_rows(n), n_cols(m), nnz(0) {
        indptr_arr.assign(n + 1, 0);
    }

    // Constructor with pre-built CSR components
    // TODO: Initialize CSR matrix from existing CSR format data, validate sizes
    CSRMatrix(size_t n, size_t m, size_t count,
        const std::vector<size_t> &indptr,
        const std::vector<size_t> &indices,
        const std::vector<T> &data) : n_rows(n), n_cols(m), nnz(count), indptr_arr(indptr), indices_arr(indices), data_arr(data) {
        if (indptr.size() != n + 1) throw size_mismatch();
        if (indices.size() != count) throw size_mismatch();
        if (data.size() != count) throw size_mismatch();
        if (indptr[0] != 0) throw invalid_index();
        if (indptr[n] != count) throw invalid_index();
        for (size_t i = 0; i < count; ++i) {
            if (indices[i] >= m) throw invalid_index();
        }
        for (size_t i = 0; i < n; ++i) {
            if (indptr[i] > indptr[i+1]) throw invalid_index();
            for (size_t j = indptr[i]; j + 1 < indptr[i+1] && j + 1 < count; ++j) {
                if (indices[j] >= indices[j+1]) throw invalid_index();
            }
        }
    }

    // Copy constructor
    CSRMatrix(const CSRMatrix &other) = default;

    // Move constructor
    CSRMatrix(CSRMatrix &&other) = default;

    // Constructor from dense matrix format (given as vector of vectors)
    // TODO: Convert dense matrix representation to CSR format
    CSRMatrix(size_t n, size_t m, const std::vector<std::vector<T>> &data) : n_rows(n), n_cols(m), nnz(0) {
        if (data.size() != n) throw size_mismatch();
        indptr_arr.push_back(0);
        for (size_t i = 0; i < n; ++i) {
            if (data[i].size() != m) throw size_mismatch();
            for (size_t j = 0; j < m; ++j) {
                // We need to check if data[i][j] is non-zero.
                // But wait, the problem says "if (i,j) is '0' (not recorded in data)".
                // For dense matrix, we should probably check if it's equal to default constructed T.
                if (data[i][j] != T{}) {
                    indices_arr.push_back(j);
                    data_arr.push_back(data[i][j]);
                    nnz++;
                }
            }
            indptr_arr.push_back(nnz);
        }
    }

    // Destructor
    ~CSRMatrix() = default;

    // Get dimensions and non-zero count
    // TODO: Return the number of rows
    size_t getRowSize() const { return n_rows; }

    // TODO: Return the number of columns
    size_t getColSize() const { return n_cols; }

    // TODO: Return the count of non-zero elements
    size_t getNonZeroCount() const { return nnz; }

    // Element access
    // TODO: Retrieve element at position (i,j)
    T get(size_t i, size_t j) const {
        if (i >= n_rows || j >= n_cols) throw invalid_index();
        size_t start = indptr_arr[i];
        size_t end = indptr_arr[i+1];
        size_t l = start, r = end;
        while (l < r) {
            size_t mid = l + (r - l) / 2;
            if (indices_arr[mid] == j) return data_arr[mid];
            if (indices_arr[mid] < j) l = mid + 1;
            else r = mid;
        }
        return T{};
    }

    // TODO: Set element at position (i,j), updating CSR structure as needed
    void set(size_t i, size_t j, const T &value) {
        if (i >= n_rows || j >= n_cols) throw invalid_index();
        size_t start = indptr_arr[i];
        size_t end = indptr_arr[i+1];
        size_t l = start, r = end;
        while (l < r) {
            size_t mid = l + (r - l) / 2;
            if (indices_arr[mid] == j) {
                data_arr[mid] = value;
                return;
            }
            if (indices_arr[mid] < j) l = mid + 1;
            else r = mid;
        }
        // Not found, insert at l
        indices_arr.insert(indices_arr.begin() + l, j);
        data_arr.insert(data_arr.begin() + l, value);
        for (size_t k = i + 1; k <= n_rows; ++k) {
            indptr_arr[k]++;
        }
        nnz++;
    }

    // Access CSR components
    // TODO: Return the row pointer array
    const std::vector<size_t> &getIndptr() const { return indptr_arr; }

    // TODO: Return the column indices array
    const std::vector<size_t> &getIndices() const { return indices_arr; }

    // TODO: Return the data values array
    const std::vector<T> &getData() const { return data_arr; }

    // Convert to dense matrix format
    // TODO: Convert CSR format to dense matrix representation
    std::vector<std::vector<T>> getMatrix() const {
        std::vector<std::vector<T>> mat(n_rows, std::vector<T>(n_cols, T{}));
        for (size_t i = 0; i < n_rows; ++i) {
            for (size_t k = indptr_arr[i]; k < indptr_arr[i+1]; ++k) {
                mat[i][indices_arr[k]] = data_arr[k];
            }
        }
        return mat;
    }

    // Matrix-vector multiplication
    // TODO: Implement multiplication of this matrix with vector vec
    std::vector<T> operator*(const std::vector<T> &vec) const {
        if (vec.size() != n_cols) throw size_mismatch();
        std::vector<T> res(n_rows, T{});
        for (size_t i = 0; i < n_rows; ++i) {
            T sum = T{};
            for (size_t k = indptr_arr[i]; k < indptr_arr[i+1]; ++k) {
                sum += data_arr[k] * vec[indices_arr[k]];
            }
            res[i] = sum;
        }
        return res;
    }

    // Row slicing
    // TODO: Extract submatrix containing rows [l,r)
    CSRMatrix getRowSlice(size_t l, size_t r) const {
        if (l > r || r > n_rows) throw invalid_index();
        size_t new_n = r - l;
        size_t new_nnz = indptr_arr[r] - indptr_arr[l];
        std::vector<size_t> new_indptr(new_n + 1, 0);
        std::vector<size_t> new_indices(new_nnz);
        std::vector<T> new_data(new_nnz);
        
        size_t offset = indptr_arr[l];
        for (size_t i = 0; i <= new_n; ++i) {
            new_indptr[i] = indptr_arr[l + i] - offset;
        }
        for (size_t i = 0; i < new_nnz; ++i) {
            new_indices[i] = indices_arr[offset + i];
            new_data[i] = data_arr[offset + i];
        }
        return CSRMatrix(new_n, n_cols, new_nnz, new_indptr, new_indices, new_data);
    }
};

}

#endif // CSR_MATRIX_HPP
