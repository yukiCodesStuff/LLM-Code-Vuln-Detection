typedef struct napi_extended_error_info {
  const char* error_message;
  void* engine_reserved;
  uint32_t engine_error_code;
  napi_status error_code;
};
```
- `error_message`: Textual representation of the error that occurred.
- `engine_reserved`: Opaque handle reserved for engine use only.
- `engine_error_code`: VM specific error code.
- `error_code`: n-api status code for the last error.

[`napi_get_last_error_info`][] returns the information for the last
N-API call that was made.

Do not rely on the content or format of any of the extended information as it
is not subject to SemVer and may change at any time. It is intended only for
logging purposes.

#### napi_get_last_error_info
<!-- YAML
added: v8.0.0
-->
```C
napi_status
napi_get_last_error_info(napi_env env,
                         const napi_extended_error_info** result);
```
- `[in] env`: The environment that the API is invoked under.
- `[out] result`: The `napi_extended_error_info` structure with more
information about the error.

Returns `napi_ok` if the API succeeded.

This API retrieves a `napi_extended_error_info` structure with information
about the last error that occurred.

The content of the `napi_extended_error_info` returned is only valid up until
an n-api function is called on the same `env`.

Do not rely on the content or format of any of the extended information as it
is not subject to SemVer and may change at any time. It is intended only for
logging purposes.

This API can be called even if there is a pending JavaScript exception.


### Exceptions
Any N-API function call may result in a pending JavaScript exception. This is
obviously the case for any function that may cause the execution of
JavaScript, but N-API specifies that an exception may be pending
on return from any of the API functions.

If the `napi_status` returned by a function is `napi_ok` then no
exception is pending and no additional action is required. If the
`napi_status` returned is anything other than `napi_ok` or
`napi_pending_exception`, in order to try to recover and continue
instead of simply returning immediately, [`napi_is_exception_pending`][]
must be called in order to determine if an exception is pending or not.

When an exception is pending one of two approaches can be employed.

The first approach is to do any appropriate cleanup and then return so that
execution will return to JavaScript. As part of the transition back to
JavaScript the exception will be thrown at the point in the JavaScript
code where the native method was invoked. The behavior of most N-API calls
is unspecified while an exception is pending, and many will simply return
`napi_pending_exception`, so it is important to do as little as possible
and then return to JavaScript where the exception can be handled.

The second approach is to try to handle the exception. There will be cases
where the native code can catch the exception, take the appropriate action,
and then continue. This is only recommended in specific cases
where it is known that the exception can be safely handled. In these
cases [`napi_get_and_clear_last_exception`][] can be used to get and
clear the exception.  On success, result will contain the handle to
the last JavaScript Object thrown. If it is determined, after
retrieving the exception, the exception cannot be handled after all
it can be re-thrown it with [`napi_throw`][] where error is the
JavaScript Error object to be thrown.

The following utility functions are also available in case native code
needs to throw an exception or determine if a `napi_value` is an instance
of a JavaScript `Error` object:  [`napi_throw_error`][],
[`napi_throw_type_error`][], [`napi_throw_range_error`][] and
[`napi_is_error`][].

The following utility functions are also available in case native
code needs to create an Error object: [`napi_create_error`][],
[`napi_create_type_error`][], and [`napi_create_range_error`][].
where result is the napi_value that refers to the newly created
JavaScript Error object.

The Node.js project is adding error codes to all of the errors
generated internally.  The goal is for applications to use these
error codes for all error checking. The associated error messages
will remain, but will only be meant to be used for logging and
display with the expectation that the message can change without
SemVer applying. In order to support this model with N-API, both
in internal functionality and for module specific functionality
(as its good practice), the `throw_` and `create_` functions
take an optional code parameter which is the string for the code
to be added to the error object.  If the optional parameter is NULL
then no code will be associated with the error. If a code is provided,
the name associated with the error is also updated to be:

```text
originalName [code]
```

where originalName is the original name associated with the error
and code is the code that was provided.  For example if the code
is 'ERR_ERROR_1' and a TypeError is being created the name will be:

```text
TypeError [ERR_ERROR_1]
```

#### napi_throw
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_throw(napi_env env, napi_value error);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] error`: The `napi_value` for the Error to be thrown.

Returns `napi_ok` if the API succeeded.

This API throws the JavaScript Error provided.


#### napi_throw_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_throw_error(napi_env env,
                                         const char* code,
                                         const char* msg);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional error code to be set on the error.
- `[in] msg`: C string representing the text to be associated with
the error.

Returns `napi_ok` if the API succeeded.

This API throws a JavaScript Error with the text provided.

#### napi_throw_type_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_throw_type_error(napi_env env,
                                              const char* code,
                                              const char* msg);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional error code to be set on the error.
- `[in] msg`: C string representing the text to be associated with
the error.

Returns `napi_ok` if the API succeeded.

This API throws a JavaScript TypeError with the text provided.

#### napi_throw_range_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_throw_range_error(napi_env env,
                                               const char* code,
                                               const char* msg);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional error code to be set on the error.
- `[in] msg`: C string representing the text to be associated with
the error.

Returns `napi_ok` if the API succeeded.

This API throws a JavaScript RangeError with the text provided.


#### napi_is_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_is_error(napi_env env,
                                      napi_value value,
                                      bool* result);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] msg`: The `napi_value` to be checked.
- `[out] result`: Boolean value that is set to true if `napi_value` represents
an error, false otherwise.

Returns `napi_ok` if the API succeeded.

This API queries a `napi_value` to check if it represents an error object.


#### napi_create_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_create_error(napi_env env,
                                          napi_value code,
                                          napi_value msg,
                                          napi_value* result);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional `napi_value` with the string for the error code to
               be associated with the error.
- `[in] msg`: napi_value that references a JavaScript String to be
used as the message for the Error.
- `[out] result`: `napi_value` representing the error created.

Returns `napi_ok` if the API succeeded.

This API returns a JavaScript Error with the text provided.

#### napi_create_type_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_create_type_error(napi_env env,
                                               napi_value code,
                                               napi_value msg,
                                               napi_value* result);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional `napi_value` with the string for the error code to
               be associated with the error.
- `[in] msg`: napi_value that references a JavaScript String to be
used as the message for the Error.
- `[out] result`: `napi_value` representing the error created.

Returns `napi_ok` if the API succeeded.

This API returns a JavaScript TypeError with the text provided.


#### napi_create_range_error
<!-- YAML
added: v8.0.0
-->
```C
NODE_EXTERN napi_status napi_create_range_error(napi_env env,
                                                napi_value code,
                                                const char* msg,
                                                napi_value* result);
```
- `[in] env`: The environment that the API is invoked under.
- `[in] code`: Optional `napi_value` with the string for the error code to
               be associated with the error.
- `[in] msg`: napi_value that references a JavaScript String to be
used as the message for the Error.
- `[out] result`: `napi_value` representing the error created.

Returns `napi_ok` if the API succeeded.

This API returns a JavaScript RangeError with the text provided.

#### napi_get_and_clear_last_exception
<!-- YAML
added: v8.0.0
-->
```C
napi_status napi_get_and_clear_last_exception(napi_env env,
                                              napi_value* result);
```

- `[in] env`: The environment that the API is invoked under.
- `[out] result`: The exception if one is pending, NULL otherwise.

Returns `napi_ok` if the API succeeded.

This API returns true if an exception is pending.

This API can be called even if there is a pending JavaScript exception.

#### napi_is_exception_pending
<!-- YAML
added: v8.0.0
-->
```C
napi_status napi_is_exception_pending(napi_env env, bool* result);
```

- `[in] env`: The environment that the API is invoked under.
- `[out] result`: Boolean value that is set to true if an exception is pending.

Returns `napi_ok` if the API succeeded.

This API returns true if an exception is pending.

This API can be called even if there is a pending JavaScript exception.

#### napi_fatal_exception
<!-- YAML
added: v9.10.0
-->
```C
napi_status napi_fatal_exception(napi_env env, napi_value err);
```

- `[in] env`: The environment that the API is invoked under.
- `[in] err`: The error you want to pass to `uncaughtException`.

Trigger an `uncaughtException` in JavaScript. Useful if an async
callback throws an exception with no way to recover.

### Fatal Errors

In the event of an unrecoverable error in a native module, a fatal error can be
thrown to immediately terminate the process.

#### napi_fatal_error
<!-- YAML
added: v8.2.0
-->
```C
NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                                 size_t location_len,
                                                 const char* message,
                                                 size_t message_len);
```

- `[in] location`: Optional location at which the error occurred.
- `[in] location_len`: The length of the location in bytes, or
`NAPI_AUTO_LENGTH` if it is null-terminated.
- `[in] message`: The message associated with the error.
- `[in] message_len`: The length of the message in bytes, or
`NAPI_AUTO_LENGTH` if it is
null-terminated.

The function call does not return, the process will be terminated.

This API can be called even if there is a pending JavaScript exception.

## Object Lifetime management

As N-API calls are made, handles to objects in the heap for the underlying
VM may be returned as `napi_values`. These handles must hold the
objects 'live' until they are no longer required by the native code,
otherwise the objects could be collected before the native code was
finished using them.

As object handles are returned they are associated with a
'scope'. The lifespan for the default scope is tied to the lifespan
of the native method call. The result is that, by default, handles
remain valid and the objects associated with these handles will be
held live for the lifespan of the native method call.

In many cases, however, it is necessary that the handles remain valid for
either a shorter or longer lifespan than that of the native method.
The sections which follow describe the N-API functions than can be used
to change the handle lifespan from the default.

### Making handle lifespan shorter than that of the native method
It is often necessary to make the lifespan of handles shorter than
the lifespan of a native method. For example, consider a native method
that has a loop which iterates through the elements in a large array:

```C
for (int i = 0; i < 1000000; i++) {
  napi_value result;
  napi_status status = napi_get_element(e, object, i, &result);
  if (status != napi_ok) {
    break;
  }
  // do something with element
}