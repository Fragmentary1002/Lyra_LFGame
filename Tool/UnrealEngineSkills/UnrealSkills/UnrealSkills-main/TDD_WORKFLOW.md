# Test-Driven Development Workflow for Unreal Engine

This document guides Claude to proactively use TDD when implementing features in Unreal Engine projects.

## Core Principle

**When implementing any new feature or fixing bugs, ALWAYS follow this order:**
1. Write the test FIRST (or update existing test)
2. Verify test FAILS (proving it tests the right thing)
3. Implement the feature
4. Build the code
5. Run the test
6. If test fails, fix the code and repeat steps 4-5
7. Only consider task complete when tests pass

## When to Apply TDD

Apply this workflow for:
- ✅ New features or functionality
- ✅ Bug fixes (write test that reproduces the bug)
- ✅ Refactoring (ensure behavior doesn't change)
- ✅ Algorithm implementations
- ✅ API changes

Skip for:
- ❌ Trivial changes (comments, formatting)
- ❌ UI-only changes without logic
- ❌ Documentation updates
- ❌ Configuration files

## Proactive Behavior Rules

### Rule 1: Suggest Tests Proactively
When user requests a feature, BEFORE implementing:
```
User: "Add a function to calculate the centroid of a polygon"

Claude: "I'll implement this using TDD. First, let me create tests for:
1. Triangle centroid (known result)
2. Square centroid (center point)
3. Concave polygon (more complex case)
4. Edge case: empty polygon (should handle gracefully)

Then I'll implement the function to make these tests pass."
```

### Rule 2: Always Run Tests After Implementation
After writing code, AUTOMATICALLY (without being asked):
1. Use `/ue-build` to compile
2. Use `/ue-test` to run relevant tests
3. If tests fail, analyze and fix
4. Repeat until tests pass

### Rule 3: Test Before Declaring Success
NEVER say "I've completed X" until:
- ✅ Code compiles without errors
- ✅ Tests are written
- ✅ Tests pass

Instead say: "Let me build and test this..."

## Implementation Pattern

### Pattern 1: New Feature Implementation

```
Step 1: Understand Requirements
- Ask clarifying questions if needed
- Identify testable behaviors

Step 2: Design Tests
- Create test cases covering:
  * Happy path
  * Edge cases
  * Error conditions
- Write test code in appropriate test file

Step 3: Verify Test Fails
- Build project
- Run new test
- Confirm it fails (Red phase)
- This proves the test is actually testing something

Step 4: Implement Feature
- Write minimal code to make test pass
- Follow UE coding standards
- Add necessary includes and dependencies

Step 5: Verify Test Passes
- Build project
- Run test again
- If fails: analyze error, fix, repeat
- If passes: done (Green phase)

Step 6: Refactor (Optional)
- Clean up code if needed
- Re-run tests to ensure nothing broke
```

### Pattern 2: Bug Fix Implementation

```
Step 1: Reproduce Bug
- Write a test that fails due to the bug
- This documents the bug and prevents regression

Step 2: Fix Bug
- Modify code to fix the issue

Step 3: Verify Fix
- Test should now pass
- Run related tests to ensure no side effects
```

## Test File Organization

### Where to Put Tests

**For Project Modules**:
```
Source/
  MyModule/
    Private/
      MyClass.cpp
      Tests/
        MyClassTests.cpp  ← Test file here
    Public/
      MyClass.h
```

**For Plugins**:
```
Plugins/
  MyPlugin/
    Source/
      MyPlugin/
        Private/
          MyFeature.cpp
          Tests/
            MyFeatureTests.cpp  ← Test file here
        Public/
          MyFeature.h
```

### Test Naming Convention

```cpp
// Test class name: <FeatureName>Tests
class FMyFeatureTests

// Test names: <Feature>_<Scenario>_<ExpectedResult>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCalculateCentroid_Triangle_ReturnsCorrectPoint,
    "MyPlugin.Geometry.CalculateCentroid",
    EAutomationTestFlags::ApplicationContextMask |
    EAutomationTestFlags::ProductFilter
)
```

## Communication Templates

### When Starting Implementation

**Good** (proactive):
```
"I'll implement this using TDD. Let me start by writing tests for:
1. [Test case 1]
2. [Test case 2]
3. [Test case 3]

Then I'll implement the feature to make these pass."
```

**Bad** (passive):
```
"I'll implement the feature now."
[implements without tests]
```

### During Implementation

**Good** (transparent):
```
"Tests written. Now implementing the feature...
[implements]
Building project...
[builds]
Running tests...
[runs tests]
❌ Test 'TriangleCentroid' failed: Expected (0,0,0) but got (1,1,1)
Let me fix the calculation...
[fixes]
Building and testing again...
✅ All tests pass!"
```

**Bad** (opaque):
```
"Done!"
[no indication of testing]
```

### When Complete

**Good**:
```
"✅ Feature complete and tested!

Implementation:
- Added CalculateCentroid() function
- Handles triangles, quads, and polygons

Tests:
- ✅ Triangle centroid (exact match)
- ✅ Square centroid (center point)
- ✅ Concave L-shape (weighted correctly)
- ✅ Empty polygon (returns origin)

All 4 tests passing. Build time: 23.4s, Test time: 1.2s"
```

**Bad**:
```
"Done! I added the CalculateCentroid function."
```

## Test Categories in UE

Use appropriate test categories for filtering:

```cpp
// Fast unit tests (run frequently)
"MyPlugin.Unit.FeatureName"
EAutomationTestFlags::ApplicationContextMask |
EAutomationTestFlags::SmokeFilter

// Medium integration tests
"MyPlugin.Integration.FeatureName"
EAutomationTestFlags::ApplicationContextMask |
EAutomationTestFlags::ProductFilter

// Slow system tests (run less frequently)
"MyPlugin.System.FeatureName"
EAutomationTestFlags::ApplicationContextMask |
EAutomationTestFlags::ProductFilter
```

Run fast tests after each change, full suite before commit.

## Handling Test Failures

### Systematic Debugging Process

```
1. Read the error message carefully
   - What assertion failed?
   - What were expected vs actual values?

2. Identify the root cause
   - Is the test wrong?
   - Is the implementation wrong?
   - Are there missing dependencies?

3. Fix the issue
   - Usually fix implementation
   - Sometimes fix test if expectations were wrong
   - Sometimes fix both

4. Re-test
   - Build and run test again
   - May need multiple iterations

5. Verify no regressions
   - Run related tests
   - Ensure fix didn't break other things
```

### Common Test Failure Patterns

| Error Pattern | Likely Cause | Solution |
|---------------|--------------|----------|
| Expected X but got Y | Logic error | Fix calculation/algorithm |
| Assertion failed | Precondition violated | Add validation or fix setup |
| Crash/exception | Null pointer, bad memory | Add null checks, validate input |
| Timeout | Infinite loop, deadlock | Review loop conditions |
| Link error (LNK2019) | Missing dependency | Update .Build.cs file |

## Integration with Skills

### Automatic Skill Invocation

Claude should automatically use:

1. **`/ue-build`** after:
   - Writing or modifying any C++ code
   - Before running tests
   - When fixing compilation errors

2. **`/ue-test`** after:
   - Successful build
   - Writing new tests
   - Fixing code based on test failures
   - To verify all tests still pass

### Skill Chaining Example

```
User: "Add a function to check if a point is inside a polygon"

Claude: "I'll implement this with TDD..."
[writes test cases]
[writes implementation]
[automatically invokes /ue-build]
Build successful ✅
[automatically invokes /ue-test PointInPolygon]
❌ Test failed: expected true for point (1,1) in square
[analyzes error]
[fixes implementation]
[automatically invokes /ue-build again]
Build successful ✅
[automatically invokes /ue-test PointInPolygon]
✅ All 5 tests pass!
"Feature complete and tested!"
```

## Examples from Real Projects

### Example 1: Corner-Attracting CVT (from BoidsFormation)

**What was done right**:
```cpp
// Test was written to verify corner concentration
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBoidsDisperseCornerFilling,
    "BoidsFormation.CVT.CornerFilling",
    EAutomationTestFlags::ApplicationContextMask |
    EAutomationTestFlags::ProductFilter
)

bool FBoidsDisperseCornerFilling::RunTest(const FString& Parameters)
{
    // Setup: Create square polygon
    // Action: Generate points with corner weighting
    // Assert: Corners have > 10% concentration
    // This test guided the CVT implementation
}
```

**Result**: Feature implemented correctly first time, test caught edge cases

### Example 2: Grid Distribution

**What was done right**:
```cpp
// Test specified exact expected behavior
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBoidsDisperseGridDistribution3x3,
    "BoidsFormation.CVT.GridDistribution",
    EAutomationTestFlags::ApplicationContextMask |
    EAutomationTestFlags::ProductFilter
)

bool FBoidsDisperseGridDistribution3x3::RunTest(const FString& Parameters)
{
    // Test verified:
    // 1. Exact number of points
    // 2. Grid spacing
    // 3. Alignment
    // 4. Boundary constraints
}
```

**Result**: Implementation matched specification exactly

## Quick Reference Checklist

Before marking any feature as complete:

- [ ] Tests written BEFORE implementation
- [ ] Tests failed initially (Red phase)
- [ ] Feature implemented
- [ ] Code compiled successfully (`/ue-build`)
- [ ] Tests run and passed (`/ue-test`)
- [ ] Edge cases covered
- [ ] Error handling tested
- [ ] No compiler warnings
- [ ] Related tests still pass (no regression)

## Summary

**Key Mindset Shift**:
Tests are not an afterthought - they are the specification.
Write them first, implement second, verify third.

**Claude's Responsibility**:
Don't wait to be asked. Proactively suggest tests, write them,
run them, and iterate until they pass. This is quality software engineering.
