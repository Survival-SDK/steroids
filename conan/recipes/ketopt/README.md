## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/ketopt/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("ketopt/cci.20260404")
```

### CMake integration:

```cmake
find_package(ketopt REQUIRED)
# Or explicitly:
target_include_directories(mytarget PRIVATE ${ketopt_INCLUDE_DIRS})
```

Note: Header is installed to `include/klib/ketopt.h`
