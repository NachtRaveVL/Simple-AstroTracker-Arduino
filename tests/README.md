# Astruino Tests

Run the host-testable core without Arduino hardware:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The host suite covers astronomy math, catalog/cache behavior, automation, enum conversion, infrastructure, and serialization.

Development Arduino sketches are included for tasks that are useful on actual hardware:

* `AstroLibExportToCPP` exports the built-in target library into C++ PROGMEM cases.
* `EnumConversionTests` tests enum string round trips.
* `EnumTrieExportToCPP` prints optimized discriminator trees for enum decoding.
* `JSONExportTests` tests catalog and pin JSON serialization.
