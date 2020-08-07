# TODO

## `cppgrep`
- [ ] **Don't show parse results from included headers:** There are cases where e.g. definitions from header files are
      included in the grep result. The expected grep output is based on the code inside the corresponding files.
- [ ] **Fix `--help` bug:** Gives error message *"Missing grep query"* and returns error code `1`.
- [ ] **Add tests:** Tests are currently pretty bad because they don't test the output of the program. HINT: Test
      through the `klang::cppgrep::grep(...)` function.
- [ ] **Implement case insensitive grep query**
- [ ] **Implement colored grep:** Highlight the substring of the *spelling* that matches the grep query with some color.
- [x] **Implement incremental printing:** When a file parsed, print out the results right away. Currently the user must
      wait until all files are parsed.
- [x] **Fix most trivial clang-tidy warnings**
- [x] **Fix use of raw c-pointer arrays:** Instead, try to use std::array or similar. (**solution:** used `std::span`)