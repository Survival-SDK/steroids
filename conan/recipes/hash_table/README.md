## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/hash_table/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("hash_table/cci.20260404")
```

### CMake integration:

```cmake
find_package(hash_table REQUIRED)
target_link_libraries(mytarget hash_table::hash_table)
# Or explicitly:
target_include_directories(mytarget PRIVATE ${hash_table_INCLUDE_DIRS})
target_link_libraries(mytarget ${hash_table_LIBRARIES})
```
