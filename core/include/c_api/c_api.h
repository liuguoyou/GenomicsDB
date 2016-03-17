/**
 * @file   c_api.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2016 MIT and Intel Corp.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @section DESCRIPTION
 *
 * This file declares the C API for TileDB. 
 */

#ifndef __C_API_H__
#define __C_API_H__

#include "constants.h"
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#  define TILEDB_EXPORT __attribute__((visibility("default")))
#else
#  define TILEDB_EXPORT
#endif

/* ********************************* */
/*              CONTEXT              */
/* ********************************* */

/** The TileDB context, which maintains state for the TileDB modules. */
typedef struct TileDB_CTX TileDB_CTX; 

/** 
 * Initializes the TileDB context.  
 *
 * @param tiledb_ctx The TileDB context to be initialized.
 * @param config_filename The name of the configuration file. If it is NULL or
 *     not found, TileDB will use its default configuration parameters.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_ctx_init(
    TileDB_CTX** tiledb_ctx, 
    const char* config_filename);

/** 
 * Finalizes the TileDB context, properly freeing-up memory. 
 *
 * @param tiledb_ctx The TileDB context to be finalized.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_ctx_finalize(TileDB_CTX* tiledb_ctx);




/* ********************************* */
/*              WORKSPACE            */
/* ********************************* */

/**
 * Creates a new TileDB workspace.
 *
 * @param tiledb_ctx The TileDB context.
 * @param workspace The directory to the workspace to be created in the file 
 *     system. This directory should not be inside another TileDB workspace,
 *     group, array or metadata directory.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_workspace_create(
    const TileDB_CTX* tiledb_ctx,
    const char* workspace);

/**
 * Lists all TileDB workspaces, copying their directory names in the input
 * string buffers.
 *
 * @param tiledb_ctx The TileDB context.
 * @param workspaces An array of strings that will store the listed workspaces.
 *     Note that this should be pre-allocated by the user. If the size of
 *     each string is smaller than the corresponding workspace path name, the
 *     function will probably segfault. It is a good idea to allocate to each
 *     workspace string TILEDB_NAME_MAX_LEN characters. 
 * @param workspace_num The number of allocated elements of the *workspaces*
 *     string array. After the function execution, this will hold the actual
 *     number of workspaces written in the *workspaces* string array. If the
 *     number of allocated elements is smaller than the number of existing
 *     TileDB workspaces, the function will return an error.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_ls_workspaces(
    const TileDB_CTX* tiledb_ctx,
    char** workspaces,
    int* workspace_num);




/* ********************************* */
/*                GROUP              */
/* ********************************* */

/**
 * Creates a new TileDB group.
 *
 * @param tiledb_ctx The TileDB context.
 * @param group The directory of the group to be created in the file system. 
 *     This should be a directory whose parent is a TileDB workspace or another 
 *     TileDB group.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_group_create(
    const TileDB_CTX* tiledb_ctx,
    const char* group);




/* ********************************* */
/*               ARRAY               */
/* ********************************* */

/** A TileDB array object. */
typedef struct TileDB_Array TileDB_Array;

/** The array schema. */
typedef struct TileDB_ArraySchema {
  /** 
   * The array name. It is a directory, whose parent must be a TileDB workspace,
   * or group.
   */
  char* array_name_;
  /** The attribute names. */
  char** attributes_;
  /** The number of attributes. */
  int attribute_num_;
  /** 
   * The tile capacity for the case of sparse fragments. If it is <=0,
   * TileDB will use its default.
   */
  int64_t capacity_;
  /** 
   * The cell order. It can be one of the following:
   *    - TILEDB_ROW_MAJOR
   *    - TILEDB_COL_MAJOR
   *    - TILEDB_HILBERT. 
   */
  int cell_order_;
  /**
   * Specifies the number of values per attribute for a cell. If it is NULL,
   * then each attribute has a single value per cell. If for some attribute
   * the number of values is variable (e.g., in the case off strings), then
   * TILEDB_VAR_NUM must be used.
   */
  int* cell_val_num_;
  /** 
   * The compression type for each attribute (plus one extra at the end for the
   * coordinates. It can be one of the following: 
   *    - TILEDB_NO_COMPRESSION
   *    - TILEDB_GZIP. 
   */
  int* compression_;
  /** 
   * Specifies if the array is dense (1) or sparse (0). If the array is dense, 
   * then the user must specify tile extents (see below).
   */
  int dense_;
  /** The dimension names. */
  char** dimensions_;
  /** The number of dimensions. */
  int dim_num_;
  /**  
   * The array domain. It should contain one [lower, upper] pair per dimension. 
   * The type of the values stored in this buffer should match the coordinates
   * type.
   */
  void* domain_;
  /** 
   * The tile extents. There should be one value for each dimension. The type of
   * the values stored in this buffer should match the coordinates type. If it
   * is NULL (applicable only to sparse arrays), then it means that the
   * array has irregular tiles.
   */
  void* tile_extents_;
  /** 
   * The tile order. It can be one of the following:
   *    - TILEDB_ROW_MAJOR
   *    - TILEDB_COL_MAJOR. 
   */
  int tile_order_;
  /** 
   * The attribute types, plus an extra one in the end for the coordinates.
   * The attribute type can be one of the following: 
   *    - TILEDB_INT32
   *    - TILEDB_INT64
   *    - TILEDB_FLOAT32
   *    - TILEDB_FLOAT64
   *    - TILEDB_CHAR. 
   * The coordinate type can be one of the following: 
   *    - TILEDB_INT32
   *    - TILEDB_INT64
   *    - TILEDB_FLOAT32
   *    - TILEDB_FLOAT64
   */
  int* types_;
} TileDB_ArraySchema;

/**
 * Populates a TileDB array schema struct.
 *
 * @param array_name The array name.
 * @param attributes The attribute names.
 * @param attribute_num The number of attributes.
 * @param dimensions The dimension names.
 * @param dim_num The number of dimensions.
 * @param dense Specifies if the array is dense (1) or sparse (0).
 * @param domain The array domain.
 * @param domain_len The length of *domain* in bytes.
 * @param tile_extents The tile extents.
 * @param tile_extents_len The length of *tile_extents* in bytes.
 * @param types The attribute types (plus one in the end for the coordinates).
 * @param cell_val_num The number of values per attribute per cell.
 * @param cell_order The cell order.
 * @param tile_order The tile order.
 * @param capacity The tile capacity.
 * @param compression The compression type for each attribute (plus an extra one
 *     in the end for the coordinates).
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 * @see TileDB_ArraySchema
 */
TILEDB_EXPORT int tiledb_array_set_schema(
    TileDB_ArraySchema* tiledb_array_schema,
    const char* array_name,
    const char** attributes,
    int attribute_num,
    const char** dimensions,
    int dim_num,
    int dense,
    const void* domain,
    size_t domain_len,
    const void* tile_extents,
    size_t tile_extents_len,
    const int* types,
    const int* cell_val_num,
    int cell_order,
    int tile_order,
    int64_t capacity,
    const int* compression);

/**
 * Creates a new TileDB array.
 *
 * @param tiledb_ctx The TileDB context.
 * @param array_schema The array schema. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_create(
    const TileDB_CTX* tiledb_ctx,
    const TileDB_ArraySchema* array_schema);

/**
 * Initializes a TileDB array.
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_array The array object to be initialized. The function
 *     will allocate memory space for it.
 * @param array The directory of the array to be initialized.
 * @param mode The mode of the array. It must be one of the following:
 *    - TILEDB_ARRAY_WRITE 
 *    - TILEDB_ARRAY_WRITE_UNSORTED 
 *    - TILEDB_ARRAY_READ 
 * @param subarray The subarray in which the array read/write will be
 *     constrained on. If it is NULL, then the subarray is set to the entire
 *     array domain. For the case of writes, this is meaningful only for
 *     dense arrays, and specifically dense writes.
 * @param attributes A subset of the array attributes the read/write will be
 *     constrained on. A NULL value indicates **all** attributes (including
 *     the coordinates in the case of sparse arrays).
 * @param attribute_num The number of the input attributes. If *attributes* is
 *     NULL, then this should be set to 0.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_init(
    const TileDB_CTX* tiledb_ctx,
    TileDB_Array** tiledb_array,
    const char* array,
    int mode,
    const void* subarray,
    const char** attributes,
    int attribute_num);

/**
 * Resets the subarray used upon initialization of the array. This is useful
 * when the array is used for reading, and the user wishes to change the
 * query subarray without having to finalize and re-initialize the array
 * with a different subarray.
 *
 * @param tiledb_array The TileDB array.
 * @param subarray The new subarray. Note that the type of the values in
 *     *subarray* should match the coordinates type in the array schema.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_reset_subarray(
    const TileDB_Array* tiledb_array,
    const void* subarray);

/**
 * Resets the attributes used upon initialization of the array. 
 * @param tiledb_array The TileDB array.
 * @param attributes The new attributes to focus on. If it is NULL, then
 *     all the attributes are used (including the coordinates in the case of
 *     sparse arrays).
 * @param attribute_num The number of the attributes. If *attributes* is NULL,
 *     then this should be 0.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_reset_attributes(
    const TileDB_Array* tiledb_array,
    const char** attributes,
    int attribute_num);

/**
 * Retrieves the schema of an already initialized array.
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_array The TileDB array object (must already be initialized). 
 * @param tiledb_array_schema The array schema to be retrieved. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_get_schema(
    const TileDB_Array* tiledb_array,
    TileDB_ArraySchema* tiledb_array_schema);

/**
 * Retrieves the schema of an array from disk.
 *
 * @param tiledb_ctx The TileDB context.
 * @param array The directory of the array whose schema will be retrieved.
 * @param tiledb_array_schema The array schema to be retrieved. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_load_schema(
    const TileDB_CTX* tiledb_ctx,
    const char* array,
    TileDB_ArraySchema* tiledb_array_schema);

/**
 * Frees the input array schema struct, properly deallocating memory space.
 *
 * @param tiledb_array_schema The array schema to be freed.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_free_schema(
    TileDB_ArraySchema* tiledb_array_schema);

/**
 * Performs a write operation in an array. The cell values are provided
 * in a set of buffers (one per attribute specified upon initialization).
 * Note that there must be a one-to-one correspondance between the cell
 * values across the attribute buffers.
 *
 * The array must be initialized in one of the following write modes,
 * each of which having a different behaviour:
 *    - TILEDB_ARRAY_WRITE: \n
 *      In this mode, the cell values are provided in the buffers respecting
 *      the cell order on the disk. It is practically an **append** operation,
 *      where the provided cell values are simply written at the end of
 *      their corresponding attribute files. This mode leads to the best
 *      performance. The user may invoke this function an arbitrary number
 *      of times, and all the writes will occur in the same fragment. 
 *      Moreover, the buffers need not be synchronized, i.e., some buffers
 *      may have more cells than others when the function is invoked.
 *    - TILEDB_ARRAY_WRITE_UNSORTED: \n
 *      This mode is applicable to sparse arrays, or when writing sparse updates
 *      to a dense array. One of the buffers holds the coordinates. The cells
 *      in this mode are given in an arbitrary, unsorted order (i.e., without
 *      respecting how the cells must be stored on the disk according to the
 *      array schema definition). Each invocation of this function internally
 *      sorts the cells and writes them to the disk on the proper order. In
 *      addition, each invocation creates a **new** fragment. Finally, the
 *      buffers in each invocation must be synced, i.e., they must have the
 *      same number of cell values across all attributes.
 * 
 * @param tiledb_array The TileDB array.
 * @param buffers An array of buffers, one for each attribute. These must be
 *     provided in the same order as the attributes specified in
 *     tiledb_array_init() or tiledb_array_reset_attributes(). The case of
 *     variable-sized attributes is special. Instead of providing a single
 *     buffer for such an attribute, **two** must be provided: the second
 *     holds the variable-sized cell values, whereas the first holds the
 *     start offsets of each cell in the second buffer.
 * @param buffer_sizes The sizes (in bytes) of the input buffers (there is
 *     a one-to-one correspondence).
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_write(
    const TileDB_Array* tiledb_array,
    const void** buffers,
    const size_t* buffer_sizes);

/**
 * Performs a read operation in an array, which must be initialized with mode
 * TILEDB_ARRAY_READ. The function retrieves the result cells that lie inside
 * the subarray specified in tiledb_array_init() or 
 * tiledb_array_reset_subarray(). The results are written in input buffers 
 * provided by the user, which are also allocated by the user. Note that the
 * results are written in the buffers in the same order they appear on the
 * disk, which leads to maximum performance. 
 * 
 * @param tiledb_array The TileDB array.
 * @param buffers An array of buffers, one for each attribute. These must be
 *     provided in the same order as the attributes specified in
 *     tiledb_array_init() or tiledb_array_reset_attributes(). The case of
 *     variable-sized attributes is special. Instead of providing a single
 *     buffer for such an attribute, **two** must be provided: the second
 *     will hold the variable-sized cell values, whereas the first holds the
 *     start offsets of each cell in the second buffer.
 * @param buffer_sizes The sizes (in bytes) allocated by the user for the input
 *     buffers (there is a one-to-one correspondence). The function will attempt
 *     to write as many results as can fit in the buffers, and potentially
 *     alter the buffer size to indicate the size of the *useful* data written
 *     in the buffer. If a buffer cannot hold all results, the function will
 *     still succeed, writing as much data as it can and turning on an overflow
 *     flag which can be checked with function tiledb_array_overflow(). The
 *     next invocation will resume for the point the previous one stopped,
 *     without inflicting a considerable performance penalty due to overflow.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_array_read(
    const TileDB_Array* tiledb_array,
    void** buffers,
    size_t* buffer_sizes);

/**
 * Checks if a read operation for a particular attribute resulted in a
 * buffer overflow.
 * 
 * @param tiledb_array The TileDB array.
 * @param attribute_id The id of the attribute for which the overflow is
 *     checked. This id corresponds to the position of the attribute name
 *     placed in the *attributes* input of tiledb_array_init(), or 
 *     tiledb_array_reset_attributes(). If *attributes* was NULL in the
 *     above functions, then the attribute id corresponds to the order
 *     in which the attributes were defined in the array schema upon the
 *     array creation. Note that, in that case, the extra coordinates 
 *     attribute corresponds to the last extra attribute, i.e., its id
 *     is *attribute_num*. 
 * @return TILEDB_ERR for error, 1 for overflow, and 0 otherwise.
 */
TILEDB_EXPORT int tiledb_array_overflow(
    const TileDB_Array* tiledb_array,
    int attribute_id);

/**
 * Consolidates the fragments of an array into a single fragment. 
 * 
 * @param tiledb_array The TileDB array to be consolidated.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_consolidate(const TileDB_Array* tiledb_array);

/** 
 * Finalizes a TileDB array, properly freeing the memory space. 
 *
 * @param tiledb_array The array to be finalized.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_finalize(
    TileDB_Array* tiledb_array);

/** A TileDB array iterator. */
typedef struct TileDB_ArrayIterator TileDB_ArrayIterator;

/**
 * Initializes an array iterator for reading cells, potentially constraining it 
 * on a subset of attributes, as well as a subarray. The cells will be read
 * in the order they are stored on the disk, maximing performance. 
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_array_it The TileDB array iterator to be created. The function
 *     will allocate the appropriate memory space for the iterator. 
 * @param array The directory of the array the iterator is initialized for.
 * @param subarray The subarray in which the array iterator will be
 *     constrained on. If it is NULL, then the subarray is set to the entire
 *     array domain. 
 * @param attributes A subset of the array attributes the iterator will be
 *     constrained on. A NULL value indicates **all** attributes (including
 *     the coordinates in the case of sparse arrays).
 * @param attribute_num The number of the input attributes. If *attributes* is
 *     NULL, then this should be set to 0.
 * @param buffers This is an array of buffers similar to tiledb_array_read().
 *     It is the user that allocates and provides buffers that the iterator
 *     will use for internal buffering of the read cells. The iterator will
 *     read from the disk the relevant cells in batches, by fitting as many
 *     cell values as possible in the user buffers. This gives the user the
 *     flexibility to control the prefetching for optimizing performance 
 *     depending on the application. 
 * @param buffer_sizes The corresponding sizes (in bytes) of the allocated 
 *     memory space for *buffers*. The function will prefetch from the
 *     disk as many cells as can fit in the buffers, whenever it finishes
 *     iterating over the previously prefetched data.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_iterator_init(
    const TileDB_CTX* tiledb_ctx,
    TileDB_ArrayIterator** tiledb_array_it,
    const char* array,
    const void* subarray,
    const char** attributes,
    int attribute_num,
    void** buffers,
    size_t* buffer_sizes);

/** 
 * Retrieves the current cell value for a particular attribute.
 *
 * @param tiledb_array_it The TileDB array iterator.
 * @param attribute_id The id of the attribute for which the cell value
 *     is retrieved. This id corresponds to the position of the attribute name
 *     placed in the *attributes* input of tiledb_array_iterator_init(). 
 *     If *attributes* was NULL in the above function, then the attribute id
 *     corresponds to the order in which the attributes were defined in the
 *     array schema upon the array creation. Note that, in that case, the extra
 *     coordinates attribute corresponds to the last extra attribute, i.e.,
 *     its id is *attribute_num*. 
 * @param value The cell value to be retrieved. Note that its type is the
 *     same as that defined in the array schema.
 * @param value_size The size (in bytes) of the retrieved value.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_iterator_get_value(
    TileDB_ArrayIterator* tiledb_array_it,
    int attribute_id, 
    const void** value,
    size_t* value_size);

/**
 * Advances the iterator by one cell.
 *
 * @param tiledb_array_it The TileDB array iterator.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_iterator_next(
    TileDB_ArrayIterator* tiledb_array_it);

/**
 * Checks if the the iterator has reached its end.
 *
 * @param tiledb_array_it The TileDB array iterator.
 * @return TILEDB_ERR for error, 1 for having reached the end, and 0 otherwise.
 */
TILEDB_EXPORT int tiledb_array_iterator_end(
    TileDB_ArrayIterator* tiledb_array_it);

/**
 * Finalizes an array iterator, properly freeing the allocating memory space.
 * 
 * @param tiledb_array_it The TileDB array iterator to be finalized.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_array_iterator_finalize(
    TileDB_ArrayIterator* tiledb_array_it);




/* ********************************* */
/*             METADATA              */
/* ********************************* */

/** Specifies the metadata schema. */
typedef struct TileDB_MetadataSchema {
  /** 
   * The metadata name. It is a directory, whose parent must be a TileDB
   * workspace, group, or array.
   */
  char* metadata_name_;
  /** The attribute names. */
  char** attributes_;
  /** The number of attributes. */
  int attribute_num_;
  /** 
   * The tile capacity. If it is <=0, TileDB will use its default.
   */
  int64_t capacity_;
  /**
   * Specifies the number of values per attribute for a cell. If it is NULL,
   * then each attribute has a single value per cell. If for some attribute
   * the number of values is variable (e.g., in the case off strings), then
   * TILEDB_VAR_NUM must be used.
   */
  int* cell_val_num_;
  /** 
   * The compression type for each attribute (plus one extra at the end for the
   * key. It can be one of the following: 
   *    - TILEDB_NO_COMPRESSION
   *    - TILEDB_GZIP. 
   */
  int* compression_;
  /** 
   * The attribute types.
   * The attribute type can be one of the following: 
   *    - TILEDB_INT32
   *    - TILEDB_INT64
   *    - TILEDB_FLOAT32
   *    - TILEDB_FLOAT64
   *    - TILEDB_CHAR. 
   */
  int* types_;
} TileDB_MetadataSchema;

/** A TileDB metadata object. */
typedef struct TileDB_Metadata TileDB_Metadata;

/**
 * Populates a TileDB metadata schema struct.
 *
 * @param metadata_name The metadata name.
 * @param attributes The attribute names.
 * @param attribute_num The number of attributes.
 * @param types The attribute types.
 * @param cell_val_num The number of values per attribute per cell.
 * @param capacity The tile capacity.
 * @param compression The compression type for each attribute (plus an extra one
 *     in the end for the key).
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 * @see TileDB_MetadataSchema
 */
TILEDB_EXPORT int tiledb_metadata_set_schema(
    TileDB_MetadataSchema* tiledb_metadata_schema,
    const char* metadata_name,
    const char** attributes,
    int attribute_num,
    const int* types,
    const int* cell_val_num,
    int64_t capacity,
    const int* compression);

/**
 * Creates a new TileDB metadata object.
 *
 * @param tiledb_ctx The TileDB context.
 * @param metadata_schema The metadata schema. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_create(
    const TileDB_CTX* tiledb_ctx,
    const TileDB_MetadataSchema* metadata_schema);

/**
 * Initializes a TileDB metadata object.
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_metadata The metadata object to be initialized. The function
 *     will allocate memory space for it.
 * @param metadata The directory of the metadata to be initialized.
 * @param mode The mode of the metadata. It must be one of the following:
 *    - TILEDB_METADATA_WRITE 
 *    - TILEDB_METADATA_READ 
 * @param attributes A subset of the metadata attributes the read/write will be
 *     constrained on. A NULL value indicates **all** attributes (including
 *     the key as an extra attribute in the end).
 * @param attribute_num The number of the input attributes. If *attributes* is
 *     NULL, then this should be set to 0.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_init(
    const TileDB_CTX* tiledb_ctx,
    TileDB_Metadata** tiledb_metadata,
    const char* metadata,
    int mode,
    const char** attributes,
    int attribute_num);

/**
 * Resets the attributes used upon initialization of the metadata. 
 *
 * @param tiledb_metadata The TileDB metadata.
 * @param attributes The new attributes to focus on. If it is NULL, then
 *     all the attributes are used (including the key as an extra attribute
 *     in the end).
 * @param attribute_num The number of the attributes. If *attributes* is NULL,
 *     then this should be 0.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_reset_attributes(
    const TileDB_Metadata* tiledb_metadata,
    const char** attributes,
    int attribute_num);

/**
 * Retrieves the schema of an already initialized metadata object.
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_metadata The TileDB metadata object (must already be 
 *     initialized). 
 * @param tiledb_metadata_schema The metadata schema to be retrieved. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_get_schema(
    const TileDB_Metadata* tiledb_metadata,
    TileDB_MetadataSchema* tiledb_metadata_schema);

/**
 * Retrieves the schema of a metadata object from disk.
 *
 * @param tiledb_ctx The TileDB context.
 * @param metadata The directory of the metadata whose schema will be retrieved.
 * @param tiledb_metadata_schema The metadata schema to be retrieved. 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_load_schema(
    const TileDB_CTX* tiledb_ctx,
    const char* metadata,
    TileDB_MetadataSchema* tiledb_metadata_schema);

/**
 * Frees the input metadata schema struct, properly deallocating memory space.
 *
 * @param tiledb_metadata_schema The metadata schema to be freed.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_free_schema(
    TileDB_MetadataSchema* tiledb_metadata_schema);

/**
 * Performs a write operation in metadata object. The values are provided
 * in a set of buffers (one per attribute specified upon initialization).
 * Note that there must be a one-to-one correspondance between the 
 * values across the attribute buffers.
 *
 * The metadata must be initialized with mode TILEDB_METADATA_WRITE.
 * 
 * @param tiledb_metadata The TileDB metadata.
 * @param keys The buffer holding the metadata keys. These keys must be
 *     strings, serialized one after the other in the *keys* buffer.
 * @param keys_size The size (in bytes) of buffer *keys*.
 * @param buffers An array of buffers, one for each attribute. These must be
 *     provided in the same order as the attributes specified in
 *     tiledb_metadata_init() or tiledb_metadata_reset_attributes(). The case of
 *     variable-sized attributes is special. Instead of providing a single
 *     buffer for such an attribute, **two** must be provided: the second
 *     holds the variable-sized values, whereas the first holds the
 *     start offsets of each value in the second buffer.
 * @param buffer_sizes The sizes (in bytes) of the input buffers (there is
 *     a one-to-one correspondence).
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_write(
    const TileDB_Metadata* tiledb_metadata,
    const char* keys,
    size_t keys_size,
    const void** buffers,
    const size_t* buffer_sizes);

/**
 * Performs a read operation in a metadata object, which must be initialized
 * with mode TILEDB_METADATA_READ. The read is performed on a single key. 
 * 
 * @param tiledb_metadata The TileDB metadata.
 * @param key This is the query key, which must be a string.
 * @param buffers An array of buffers, one for each attribute. These must be
 *     provided in the same order as the attributes specified in
 *     tiledb_metadata_init() or tiledb_metadata_reset_attributes(). The case of
 *     variable-sized attributes is special. Instead of providing a single
 *     buffer for such an attribute, **two** must be provided: the second
 *     will hold the variable-sized values, whereas the first holds the
 *     start offsets of each value in the second buffer.
 * @param buffer_sizes The sizes (in bytes) allocated by the user for the input
 *     buffers (there is a one-to-one correspondence). The function will attempt
 *     to write value corresponding to the key. If a buffer cannot hold the
 *     result, the function will still succeed, turning on an overflow
 *     flag which can be checked with function tiledb_metadata_overflow(). 
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_metadata_read(
    const TileDB_Metadata* tiledb_metadata,
    const char* key,
    void** buffers,
    size_t* buffer_sizes);

/**
 * Checks if a read operation for a particular attribute resulted in a
 * buffer overflow.
 * 
 * @param tiledb_metadata The TileDB metadata.
 * @param attribute_id The id of the attribute for which the overflow is
 *     checked. This id corresponds to the position of the attribute name
 *     placed in the *attributes* input of tiledb_metadata_init(), or 
 *     tiledb_metadata_reset_attributes(). If *attributes* was NULL in the
 *     above functions, then the attribute id corresponds to the order
 *     in which the attributes were defined in the array schema upon the
 *     array creation. Note that, in that case, the extra key 
 *     attribute corresponds to the last extra attribute, i.e., its id
 *     is *attribute_num*. 
 * @return TILEDB_ERR for error, 1 for overflow, and 0 otherwise.
 */
TILEDB_EXPORT int tiledb_metadata_overflow(
    const TileDB_Metadata* tiledb_metadata,
    int attribute_id);

/**
 * Consolidates the fragments of a metadata object into a single fragment. 
 * 
 * @param tiledb_metadata The TileDB metadata to be consolidated.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_consolidate(
    const TileDB_Metadata* tiledb_metadata);

/** 
 * Finalizes a TileDB metadata object, properly freeing the memory space. 
 *
 * @param tiledb_metadata The metadata to be finalized.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_finalize(
    TileDB_Metadata* tiledb_metadata);

/** A TileDB metadata iterator. */
typedef struct TileDB_MetadataIterator TileDB_MetadataIterator;

/**
 * Initializes a metadata iterator, potentially constraining it 
 * on a subset of attributes. The values will be read in the order they are
 * stored on the disk, maximing performance. 
 *
 * @param tiledb_ctx The TileDB context.
 * @param tiledb_metadata_it The TileDB metadata iterator to be created. The
 *     function will allocate the appropriate memory space for the iterator. 
 * @param metadata The directory of the metadata the iterator is initialized
 *     for.
 * @param attributes A subset of the metadata attributes the iterator will be
 *     constrained on. A NULL value indicates **all** attributes (including
 *     the key as an extra attribute in the end).
 * @param attribute_num The number of the input attributes. If *attributes* is
 *     NULL, then this should be set to 0.
 * @param buffers This is an array of buffers similar to tiledb_metadata_read().
 *     It is the user that allocates and provides buffers that the iterator
 *     will use for internal buffering of the read values. The iterator will
 *     read from the disk the values in batches, by fitting as many
 *     values as possible in the user buffers. This gives the user the
 *     flexibility to control the prefetching for optimizing performance 
 *     depending on the application. 
 * @param buffer_sizes The corresponding sizes (in bytes) of the allocated 
 *     memory space for *buffers*. The function will prefetch from the
 *     disk as many values as can fit in the buffers, whenever it finishes
 *     iterating over the previously prefetched data.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_iterator_init(
    const TileDB_CTX* tiledb_ctx,
    TileDB_MetadataIterator** tiledb_metadata_it,
    const char* metadata,
    const char** attributes,
    int attribute_num,
    void** buffers,
    size_t* buffer_sizes);

/** 
 * Retrieves the current value for a particular attribute.
 *
 * @param tiledb_metadata_it The TileDB metadata iterator.
 * @param attribute_id The id of the attribute for which the value
 *     is retrieved. This id corresponds to the position of the attribute name
 *     placed in the *attributes* input of tiledb_metadata_iterator_init(). 
 *     If *attributes* was NULL in the above function, then the attribute id
 *     corresponds to the order in which the attributes were defined in the
 *     array schema upon the array creation. Note that, in that case, the extra
 *     key attribute corresponds to the last extra attribute, i.e.,
 *     its id is *attribute_num*. 
 * @param value The value to be retrieved. Note that its type is the
 *     same as that defined in the metadata schema.
 * @param value_size The size (in bytes) of the retrieved value.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_iterator_get_value(
    TileDB_MetadataIterator* tiledb_metadata_it,
    int attribute_id, 
    const void** value,
    size_t* value_size);

/**
 * Advances the iterator by one position.
 *
 * @param tiledb_metadata_it The TileDB metadata iterator.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_iterator_next(
    TileDB_MetadataIterator* tiledb_metadata_it);

/**
 * Checks if the the iterator has reached its end.
 *
 * @param tiledb_metadata_it The TileDB metadata iterator.
 * @return TILEDB_ERR for error, 1 for having reached the end, and 0 otherwise.
 */
TILEDB_EXPORT int tiledb_metadata_iterator_end(
    TileDB_MetadataIterator* tiledb_metadata_it);

/**
 * Finalizes the iterator, properly freeing the allocating memory space.
 * 
 * @param tiledb_metadata_it The TileDB metadata iterator.
 * @return TILEDB_OK on success, and TILEDB_ERR on error.
 */
TILEDB_EXPORT int tiledb_metadata_iterator_finalize(
    TileDB_MetadataIterator* tiledb_metadata_it);





/* ********************************* */
/*               MISC                */
/* ********************************* */

/**
 * Clears a TileDB directory. The corresponding TileDB object (workspace,
 * group, array, or metadata) will still exist after the execution of the
 * function, but it will be empty (i.e., as if it was just created).
 *
 * @param tiledb_ctx The TileDB context.
 * @param dir The directory to be cleared.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_clear( 
    const TileDB_CTX* tiledb_ctx, 
    const char* dir);

/**
 * Deletes a TileDB directory (workspace, group, array, or metadata) entirely. 
 *
 * @param tiledb_ctx The TileDB context.
 * @param dir The directory to be deleted.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_delete( 
    const TileDB_CTX* tiledb_ctx, 
    const char* dir);

/**
 * Moves a TileDB directory (workspace, group, array or metadata).
 *
 * @param tiledb_ctx The TileDB context.
 * @param old_dir The old directory.
 * @param new_dir The new directory.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_move( 
    const TileDB_CTX* tiledb_ctx, 
    const char* old_dir,
    const char* new_dir);

/**
 * Lists all the TileDB objects in a directory, copying them into the input
 * buffers.
 *
 * @param tiledb_ctx The TileDB context.
 * @param parent_dir The parent directory of the TileDB objects to be listed.
 * @param dirs An array of strings that will store the listed TileDB objects.
 *     Note that the user is responsible for allocating the appropriate memory
 *     space for this array of strings. A good idea for each string length is
 *     to set is to TILEDB_NAME_MAX_LEN.
 * @param dir_types The types of the corresponding TileDB objects, which can
 *     be the following (they are self-explanatory):
 *    - TILEDB_WORKSPACE
 *    - TILEDB_GROUP
 *    - TILEDB_ARRAY
 *    - TILEDB_METADATA
 * @param dir_num The number of elements allocated by the user for *dirs*. After
 *     the function terminates, this will hold the actual number of TileDB
 *     objects that were stored in *dirs*.
 * @return TILEDB_OK for success and TILEDB_ERR for error.
 */
TILEDB_EXPORT int tiledb_ls(
    const TileDB_CTX* tiledb_ctx,
    const char* parent_dir,
    char** dirs,
    int* dir_types,
    int* dir_num);

#undef TILEDB_EXPORT
#ifdef __cplusplus
}
#endif

#endif