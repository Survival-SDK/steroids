## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/scv/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("scv/cci.20260404")
```

### CMake integration:

```cmake
find_package(scv REQUIRED)
target_link_libraries(mytarget scv::scv)
# Or explicitly:
target_include_directories(mytarget PRIVATE ${scv_INCLUDE_DIRS})
target_link_libraries(mytarget ${scv_LIBRARIES})
```
