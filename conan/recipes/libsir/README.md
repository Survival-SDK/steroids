## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/libsir/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("libsir/2.2.5")
```

### CMake integration:

```cmake
find_package(libsir REQUIRED)
target_link_libraries(mytarget libsir::libsir)
# Or explicitly:
target_include_directories(mytarget PRIVATE ${libsir_INCLUDE_DIRS})
target_link_libraries(mytarget ${libsir_LIBRARIES})
```
