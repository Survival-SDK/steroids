## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/lwrb/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("lwrb/3.2.0")
```

### CMake integration:

```cmake
find_package(lwrb REQUIRED)
target_link_libraries(mytarget lwrb::lwrb)
# Or explicitly:
target_include_directories(mytarget PRIVATE ${lwrb_INCLUDE_DIRS})
target_link_libraries(mytarget ${lwrb_LIBRARIES})
```
