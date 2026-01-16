---
title: How to Verify Validity Behavior
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Verify Validity Behavior

This guide shows you how to check that validity is working correctly in your Weaver code.

## Goal

By the end of this guide, you'll be able to:
- Check if variables are valid or null
- Verify validity propagation through operations
- Debug validity-related issues
- Ensure correct timing behavior

## Prerequisites

- You understand [validity and truthiness](../reference/validity-truthiness)
- You know how to use [processes](../reference/processes)
- You've read about the [validity system](../explanations/validity-system)

## Step 1: Check Variable Validity

Use `valid()` to explicitly check if a variable is valid:

```weaver
func check_validity(chan<int> in) {
    var int value
    while {
        await in {
            value = in.recv()
            
            // Explicit validity check
            if valid(value) == vdd {
                // value is valid, proceed
                process(value)
            } else {
                // value is null, handle error
                handle_null()
            }
        }
    }
}
```

## Step 2: Verify Validity Propagation

Check that validity propagates correctly through operations:

```weaver
func verify_propagation(chan<int> a, chan<int> b) chan<int> out {
    var int x, y, result
    
    while {
        await a & b {
            x = a.recv()
            y = b.recv()
            
            // Both x and y should be valid here
            if valid(x) == vdd && valid(y) == vdd {
                result = x + y
                // result should be valid
                if valid(result) == vdd {
                    out.send(result)
                }
            }
        }
    }
}
```

## Step 3: Test Null Propagation

Verify that null values propagate correctly:

```weaver
func test_null_propagation() {
    var int<32> a, b, c
    
    // Set a to null
    a-
    
    // Verify a is null
    if valid(a) == gnd {
        // a is null, as expected
    }
    
    // Operations with null should produce null
    b = a + 5
    // b should be null
    if valid(b) == gnd {
        // Correct: null + valid = null
    }
    
    c = a * 2
    // c should be null
    if valid(c) == gnd {
        // Correct: null * valid = null
    }
}
```

## Step 4: Verify Await Behavior

Check that `await` correctly waits for validity:

```weaver
func verify_await(chan<int> in) {
    var int value
    
    while {
        // await should block until 'in' is valid
        await in {
            // At this point, 'in' should be valid
            if valid(in) == vdd {
                value = in.recv()
                // value should be valid
                if valid(value) == vdd {
                    process(value)
                }
            }
        }
    }
}
```

## Step 5: Check Truthiness vs Validity

Distinguish between validity and truthiness:

```weaver
func verify_truthiness() {
    var bool flag
    
    // Set to false (valid but false)
    flag = false
    if valid(flag) == vdd {
        // flag is valid
        if true(flag) == gnd {
            // flag is false (not truthy)
        }
    }
    
    // Set to null
    flag-
    if valid(flag) == gnd {
        // flag is null
        if true(flag) == gnd {
            // flag is also not truthy (because null)
        }
    }
    
    // Set to true (valid and truthy)
    flag = true
    if valid(flag) == vdd {
        // flag is valid
        if true(flag) == vdd {
            // flag is truthy
        }
    }
}
```

## Step 6: Verify Parallel Composition Validity

Check validity in parallel compositions:

```weaver
func verify_parallel_validity() {
    var int<32> a, b, c
    a-, b-, c-  // Initialize as null
    
    (
        a = 5
        // a should be valid here
        if valid(a) == vdd {
            // Correct
        }
    ) and (
        await a {
            // a should be valid here
            if valid(a) == vdd {
                b = a + 1
                // b should be valid
                if valid(b) == vdd {
                    // Correct
                }
            }
        }
    ) and (
        await b {
            // b should be valid here
            if valid(b) == vdd {
                c = b * 2
                // c should be valid
                if valid(c) == vdd {
                    // Correct
                }
            }
        }
    )
}
```

## Step 7: Debug Validity Issues

Add validity checks to debug problems:

```weaver
func debug_validity(chan<int> in) chan<int> out {
    var int value, intermediate
    
    while {
        await in {
            value = in.recv()
            
            // Debug: check validity at each step
            if valid(value) != vdd {
                // Error: value should be valid after recv
                signal_error()
            }
            
            intermediate = value * 2
            
            // Debug: check intermediate validity
            if valid(intermediate) != vdd {
                // Error: intermediate should be valid
                signal_error()
            }
            
            out.send(intermediate)
            
            // Debug: verify send doesn't invalidate
            if valid(intermediate) != vdd {
                // Note: send might invalidate, depends on implementation
            }
        }
    }
}
```

## Step 8: Complete Example: Validity Test Suite

Here's a complete example that tests various validity scenarios:

```weaver
func validity_test_suite() {
    // Test 1: Basic validity
    var int<32> a = 5
    if valid(a) == vdd {
        // Pass: a is valid
    }
    
    // Test 2: Null assignment
    a-
    if valid(a) == gnd {
        // Pass: a is null
    }
    
    // Test 3: Null propagation
    var int<32> b
    b-
    var int<32> c = a + b
    if valid(c) == gnd {
        // Pass: null + null = null
    }
    
    // Test 4: Valid + null
    a = 5
    if valid(a) == vdd && valid(b) == gnd {
        var int<32> d = a + b
        if valid(d) == gnd {
            // Pass: valid + null = null
        }
    }
    
    // Test 5: Valid + valid
    b = 3
    if valid(a) == vdd && valid(b) == vdd {
        var int<32> e = a + b
        if valid(e) == vdd {
            // Pass: valid + valid = valid
            if e == 8 {
                // Pass: correct value
            }
        }
    }
    
    // Test 6: Boolean validity and truthiness
    var bool flag1 = true
    var bool flag2 = false
    var bool flag3
    flag3-
    
    if valid(flag1) == vdd && true(flag1) == vdd {
        // Pass: flag1 is valid and truthy
    }
    
    if valid(flag2) == vdd && true(flag2) == gnd {
        // Pass: flag2 is valid but not truthy
    }
    
    if valid(flag3) == gnd && true(flag3) == gnd {
        // Pass: flag3 is null and not truthy
    }
}
```

## Best Practices

### Check Validity Explicitly When Needed

Don't assume validity—check it when behavior depends on it.

### Test Null Cases

Always test what happens with null values, not just valid ones.

### Verify Propagation

Ensure validity propagates correctly through your operations.

### Use `valid()` and `true()` Selectively

Most of the time, you don't need explicit checks. Use them when debugging or when behavior depends on validity.

## Verification

Test your validity handling:

1. **Valid values**: Verify normal operation
2. **Null values**: Test null propagation
3. **Mixed cases**: Test valid + null combinations
4. **Parallel composition**: Verify validity coordination
5. **Channel communication**: Verify validity through channels

## Common Issues

### Issue: Variable Always Null

**Symptom**: Variable never becomes valid.

**Possible causes**:
- Never assigned a value
- Assigned null explicitly
- Validity not propagating

**Fix**: Check assignments and validity propagation.

### Issue: Unexpected Null Propagation

**Symptom**: Valid operation produces null.

**Possible causes**:
- One operand is null
- Validity not checked before operation

**Fix**: Check operand validity before operations.

## Next Steps

- Learn about [debugging processes](./how-to-debug-processes)
- Understand the [validity system](../explanations/validity-system)
- Read about [timing assumptions](../explanations/timing-assumptions)
