## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/cmake_barebones/all --version=cci.20260404
```

### Use as build requirement in your conanfile.py:

```python
def build_requirements(self):
    self.tool_requires("cmake_barebones/cci.20260404")
```

### CMake usage:

CMake modules are automatically added to `CMAKE_MODULE_PATH`.

```cmake
# Your CMakeLists.txt
list(APPEND CMAKE_MODULE_PATH ${CMAKE_PREFIX_PATH}/lib/cmake/barebones)

# Or use modules directly if Conan integration is set up
include(SomeBarebonesModule)
```

### Make usage:

```makefile
# Your Makefile
include $(CONAN_PACKAGE_PATH)/include/barebones.mk
```
