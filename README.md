# SteleDB

A lightweight, in-process relational database engine written in C++23. SteleDB provides a SQL-like query interface with B-Tree indexed primary keys, page-based storage, and efficient query processing.

## Overview

SteleDB is designed as an educational and practical database implementation demonstrating core database concepts including:

- **Query Processing**: Full SQL parser and query execution engine
- **Storage Layer**: Page-based storage system with efficient slot allocation
- **Indexing**: B-Tree indices on primary keys for fast lookups
- **Memory Management**: Efficient memory layout and data serialization
- **Type System**: Support for INT, VARCHAR(16), BOOL, and FLOAT data types

## Architecture

```
Engine (REPL interface)
  ├── QueryProcessor (SQL parsing and tokenization)
  ├── MemoryLayer (Query execution and row management)
  │   ├── PageIO (Physical page read/write)
  │   └── BTree (Primary key indexing)
  └── Table (Schema and metadata management)
```

### Core Components

- **Engine**: Main entry point; orchestrates query execution and table management
- **QueryProcessor**: Tokenizes and parses SQL queries into executable tokens
- **MemoryLayer**: Executes queries, manages row insertion/deletion, coordinates with storage
- **PageIO**: Handles page-level I/O operations for persistent storage
- **BTree**: B-Tree index for primary key lookups and range queries
- **Table**: Stores schema metadata, column definitions, and primary key index

## Building

### Prerequisites

- C++23 compatible compiler (MSVC 193+ or Clang 16+)
- CMake 3.20+

### Build Instructions

```bash
cd stele_db
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Running Tests

```bash
ctest
```

Individual tests can be run directly:
```bash
./test_b_tree
./test_query_processor
./test_engine
./test_memory_layer
./test_page_io
```

## Usage

### Starting the Database

```bash
./main
```

This launches an interactive REPL where you can enter SQL queries.

### SQL Syntax

#### CREATE TABLE

```sql
CREATE TABLE users (id INT, name VARCHAR_16, email VARCHAR_16, active BOOL);
```

The first column is automatically treated as the primary key. Currently supported types:
- `INT` - 32-bit signed integer
- `VARCHAR_16` - String up to 16 characters
- `BOOL` - Boolean (true/false)
- `FLOAT` - 64-bit double-precision float

#### INSERT

```sql
INSERT INTO users (id, name, email, active) VALUES (1, "alice", "alice@example.com", true);
INSERT INTO users (id, name, email, active) VALUES (2, "bob", "bob@example.com", false);
```

All columns must be specified (no NULL support yet).

#### SELECT

Select all rows:
```sql
SELECT * FROM users;
```

Select specific columns:
```sql
SELECT id, name FROM users;
```

Select with WHERE clause (supports equality and comparison operators):
```sql
SELECT * FROM users WHERE id = 1;
SELECT * FROM users WHERE id >= 2;
SELECT * FROM users WHERE active = true;
```

**Note**: Primary key equality queries (`WHERE pk = value`) are optimized using the B-Tree index for O(log n) lookup. Other predicates require full table scans.

#### DELETE

```sql
DELETE FROM users WHERE id = 1;
DELETE FROM users;  -- Deletes all rows
```

#### DROP TABLE

```sql
DROP TABLE users;
```

## Storage Format

### Table Layout

Tables are stored in the `tables/` directory by table name:
```
tables/
  users/
    metadata.data      -- Schema and metadata
    page_0.data        -- Data pages
    page_1.data
    btrees/
      primary_key_*    -- B-Tree index files
```

### Page Format

Pages are fixed-size (8KB) with:
- Occupancy bitmap (tracks which row slots are in use)
- Slot space (stores row data)
- One row per slot, contiguous layout

### Primary Key Index

Primary keys are indexed via B-Tree with:
- 255 max keys per node
- Fast equality lookups: O(log n)
- Range queries: O(log n + results)
- Supports INT, BOOL, and VARCHAR_16 primary keys

## Data Types and Limitations

| Type | Size | Notes |
|------|------|-------|
| INT | 4 bytes | 32-bit signed |
| VARCHAR_16 | 16 bytes | Fixed-width, null-padded |
| BOOL | 1 byte | true/false |
| FLOAT | 8 bytes | 64-bit double |

### Known Limitations

- No NULL value support (all columns required on INSERT)
- VARCHAR limited to 16 characters
- Single-threaded (no concurrency control)
- No transactions or ACID guarantees
- No joins or subqueries
- No views or triggers
- B-Tree indices only on primary keys
- No aggregate functions or GROUP BY

## Project Structure

```
.
├── CMakeLists.txt           -- Build configuration
├── inc/                     -- Header files
│   ├── engine.hpp
│   ├── query_processor.hpp
│   ├── memory_layer.hpp
│   ├── b_tree.hpp
│   ├── page_io.hpp
│   ├── table.hpp
│   ├── data_type.hpp
│   ├── predicate.hpp
│   ├── column.hpp
│   └── ...
├── src/                     -- Implementation
│   ├── main.cpp
│   ├── engine.cpp
│   ├── query_processor.cpp
│   ├── memory_layer.cpp
│   ├── b_tree.cpp
│   └── page_io.cpp
├── test/                    -- Test suite
│   ├── engine.test.cpp
│   ├── query_processor.test.cpp
│   ├── memory_layer.test.cpp
│   ├── b_tree.test.cpp
│   └── page_io.test.cpp
└── build/                   -- Build output (generated)
```

## Example Session

```sql
Welcome to SteleDB!

CREATE TABLE products (sku VARCHAR_16, price FLOAT, in_stock BOOL);
INSERT INTO products (sku, price, in_stock) VALUES ("ABC123", 29.99, true);
INSERT INTO products (sku, price, in_stock) VALUES ("XYZ789", 49.99, true);
INSERT INTO products (sku, price, in_stock) VALUES ("DEF456", 15.50, false);

SELECT * FROM products;
sku         price   in_stock
ABC123      29.99   1
XYZ789      49.99   1
DEF456      15.50   0

SELECT sku, price FROM products WHERE price >= 30;
sku         price
XYZ789      49.99

DELETE FROM products WHERE in_stock = false;

SELECT * FROM products;
sku         price   in_stock
ABC123      29.99   1
XYZ789      49.99   1
```

## Performance

- **Insert**: O(log n) for primary key verification, amortized O(1) page writes
- **Primary key lookup**: O(log n) via B-Tree
- **Full table scan**: O(n)
- **Delete**: O(log n) for primary key queries, O(n) for other predicates

## Development Notes

### Code Style

- C++23 standard library features (std::variant, std::optional, std::bitset)
- Header-only templates for predicates and data operations
- Zero-copy where possible (move semantics, string_view)

### Testing

Tests are organized by component and can be run individually or via CTest. Each test file validates:
- Parsing and tokenization (query_processor.test.cpp)
- Storage and retrieval (page_io.test.cpp, memory_layer.test.cpp)
- Indexing (b_tree.test.cpp)
- End-to-end integration (engine.test.cpp)

## Future Enhancements

- Secondary indices (non-primary key columns)
- NULL value support
- Larger VARCHAR capacity (variable-length strings)
- Query optimization and planning
- Multi-threaded access with locking
- Transaction support with rollback
- Persistence recovery and crash recovery
- Query language extensions (JOIN, GROUP BY, aggregates)

## License

Educational project - use for learning and reference purposes.
