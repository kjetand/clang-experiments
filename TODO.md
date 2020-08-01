# TODO

## `cppgrep`
- [ ] **Fix `--help` bug:** Gives error message *"Missing grep query"* and returns error code `1`.
- [ ] **Add tests:** Tests are currently pretty bad because they don't test the output of the program. HINT: Test
      through the `klang::cppgrep::grep(...)` function.
- [ ] **Implement case insensitive grep query**
- [ ] **Implement colored grep:** Highlight the substring of the *spelling* that matches the grep query with some color.
- [x] **Fix most trivial clang-tidy warnings**
- [ ] **Fix use of raw c-pointer arrays:** Instead, try to use std::array or similar.