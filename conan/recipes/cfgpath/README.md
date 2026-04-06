## Usage

### Export recipe to local cache:

```bash
conan export conan/recipes/cfgpath/all
```

### Use in your conanfile.py:

```python
def requirements(self):
    self.requires("cfgpath/cci.20260404")
```

### CMake integration:

```cmake
find_package(cfgpath REQUIRED)
target_include_directories(mytarget PRIVATE ${cfgpath_INCLUDE_DIRS})
```
