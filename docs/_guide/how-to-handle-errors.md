---
title: How to Handle Errors in Processes
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Handle Errors in Processes

This guide shows you how to detect, signal, and handle errors in Weaver processes.

## Goal

By the end of this guide, you'll be able to:
- Detect error conditions in your processes
- Signal errors to other processes
- Implement error recovery mechanisms
- Design robust error handling

## Prerequisites

- You understand [processes](../reference/processes)
- You know how to use [channels](../tutorials/channels)
- You understand [validity and truthiness](../reference/validity-truthiness)

## Step 1: Define Error Types

First, define what constitutes an error in your system:

```weaver
type Error {
    int<2> code    // 0=OK, 1=INVALID, 2=TIMEOUT, 3=OVERFLOW
    bool occurred
}

type Result {
    int<32> data
    Error error
}
```

## Step 2: Detect Error Conditions

Check for error conditions in your process:

```weaver
func process_with_error_check(chan<int<32>> in) chan<Result> out {
    var int<32> value
    var Result result
    
    while {
        await in {
            value = in.recv()
            
            // Check for errors
            if value < 0 {
                result.error = {code: 1, occurred: true}  // INVALID
                result.data = 0
            } else if value > 1000 {
                result.error = {code: 3, occurred: true}  // OVERFLOW
                result.data = 0
            } else {
                result.error = {code: 0, occurred: false}  // OK
                result.data = value * 2
            }
            
            out.send(result)
        }
    }
}
```

## Step 3: Signal Errors Through Channels

Use dedicated error channels or include errors in result types:

### Dedicated Error Channel

```weaver
func process(chan<int> in) (chan<int> out, chan<Error> errors) {
    var int value
    var Error error
    
    while {
        await in {
            value = in.recv()
            
            if is_valid(value) {
                out.send(process_value(value))
            } else {
                error = {code: 1, occurred: true}
                errors.send(error)
            }
        }
    }
}
```

### Error in Result Type

```weaver
type Result {
    int<32> data
    Error error
}

func process(chan<int> in) chan<Result> out {
    var int value
    var Result result
    
    while {
        await in {
            value = in.recv()
            if is_valid(value) {
                result = {data: process_value(value), error: {code: 0, occurred: false}}
            } else {
                result = {data: 0, error: {code: 1, occurred: true}}
            }
            out.send(result)
        }
    }
}
```

## Step 4: Handle Errors in Consumers

Process errors in the consumer:

```weaver
func consumer(chan<Result> in) {
    var Result result
    while {
        await in {
            result = in.recv()
            if result.error.occurred {
                handle_error(result.error)
            } else {
                use_data(result.data)
            }
        }
    }
}
```

## Step 5: Implement Error Recovery

Add recovery logic to handle errors:

```weaver
func process_with_recovery(chan<int> in) chan<int> out {
    var int value, retry_count
    var bool success
    
    while {
        await in {
            value = in.recv()
            retry_count = 0
            success = false
            
            while retry_count < 3 && !success {
                if try_process(value) {
                    out.send(process(value))
                    success = true
                } else {
                    retry_count = retry_count + 1
                    // Wait before retry
                    await retry_delay {
                        retry_delay.recv()
                    }
                }
            }
            
            if !success {
                // Give up after retries
                signal_error()
            }
        }
    }
}
```

## Step 6: Implement Timeout Handling

Handle timeouts as a form of error:

```weaver
func process_with_timeout(chan<int> in) chan<Result> out {
    var int value
    var Result result
    
    while {
        await in {
            value = in.recv()
            result = try_with_timeout(value)
            out.send(result)
        }
    }
}

func try_with_timeout(int value) Result {
    var Result result
    var bool completed = false
    
    // Start processing
    (
        process_value(value)
        completed = true
        result = {data: processed_value, error: {code: 0, occurred: false}}
    ) and (
        await timeout_signal {
            timeout_signal.recv()
            if !completed {
                result = {data: 0, error: {code: 2, occurred: true}}  // TIMEOUT
            }
        }
    )
    
    return result
}
```

## Step 7: Propagate Errors Through Pipelines

Pass errors through pipeline stages:

```weaver
func stage1(chan<int> in) chan<Result> out {
    var int value
    var Result result
    
    while {
        await in {
            value = in.recv()
            if is_valid(value) {
                result = {data: value, error: {code: 0, occurred: false}}
            } else {
                result = {data: 0, error: {code: 1, occurred: true}}
            }
            out.send(result)
        }
    }
}

func stage2(chan<Result> in) chan<Result> out {
    var Result input, output
    
    while {
        await in {
            input = in.recv()
            
            // Propagate errors
            if input.error.occurred {
                out.send(input)  // Pass error through
            } else {
                // Process only if no error
                if can_process(input.data) {
                    output = {data: process(input.data), error: {code: 0, occurred: false}}
                } else {
                    output = {data: 0, error: {code: 1, occurred: true}}
                }
                out.send(output)
            }
        }
    }
}
```

## Step 8: Complete Example: Error Handling System

Here's a complete example with comprehensive error handling:

```weaver
type ErrorCode {
    int<3> value  // 0=OK, 1=INVALID_INPUT, 2=TIMEOUT, 3=OVERFLOW, 4=UNDERFLOW
}

type ProcessResult {
    int<32> data
    ErrorCode error
    bool valid
}

func robust_processor(chan<int<32>> in) chan<ProcessResult> out {
    var int<32> value
    var ProcessResult result
    
    while {
        await in {
            value = in.recv()
            
            // Validate input
            if value < 0 {
                result = {data: 0, error: {value: 1}, valid: false}  // INVALID_INPUT
            } else if value > 1000000 {
                result = {data: 0, error: {value: 3}, valid: false}  // OVERFLOW
            } else if value == 0 {
                result = {data: 0, error: {value: 4}, valid: false}  // UNDERFLOW
            } else {
                // Process valid input
                var int<32> processed = compute(value)
                if processed > 0 && processed < 1000000 {
                    result = {data: processed, error: {value: 0}, valid: true}
                } else {
                    result = {data: 0, error: {value: 3}, valid: false}  // Result overflow
                }
            }
            
            out.send(result)
        }
    }
}

func error_handler(chan<ProcessResult> in) {
    var ProcessResult result
    while {
        await in {
            result = in.recv()
            if !result.valid {
                handle_error(result.error)
            } else {
                use_result(result.data)
            }
        }
    }
}
```

## Best Practices

### Fail Fast

Detect errors as early as possible to avoid propagating bad data.

### Make Errors Explicit

Use error types rather than magic values or null returns.

### Document Error Conditions

Clearly document what errors can occur and when.

### Test Error Paths

Verify that error handling works correctly, not just the happy path.

### Recover When Possible

Implement recovery mechanisms for transient errors.

## Verification

Test your error handling:

1. **Valid inputs**: Verify normal operation
2. **Invalid inputs**: Test error detection
3. **Error propagation**: Verify errors flow correctly
4. **Recovery**: Test recovery mechanisms
5. **Edge cases**: Test boundary conditions

## Next Steps

- Learn about [testing processes](./how-to-test-processes)
- Understand [validity system](../explanations/validity-system)
- Read about [debugging processes](./how-to-debug-processes)
