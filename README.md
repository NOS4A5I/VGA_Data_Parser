# **What is this?**

This is a library to parse through a custom file format containing Video Graphics Array (VGA) data. Information about file format TBA.

## **Compilation:**

Compilation for this library is rather simplistic. I used `gcc` to compile the output object file  `vgd_parse.o`.

Using the supplied Makefile supports 3 targets:

- `all`: Generates the `vgd_parse.o` object file in the `build` directory.
- `unit_testing`: Compiles an executable `unit_tests` in the `build` directory. The executable interacts with the test files directory when run from the `build` directory. I used this to test for edge cases. Check `unit_tests.c` and run the executable to see the exact cases I tested for.
- `clean`: Removes all build outputs.

## **Usage:**

The data struct used for storing file information is defined as follows:

```
typedef struct {
	FILE *vgd_fd;
	unsigned char line_data[4];
	unsigned int log_count;
	unsigned long long int *log_offset;
	unsigned long long int current_log_offset;
} VGD_input;
```

A user should not interact with these structs directly unless trying to debug an error. The functions supplied by the library handle the manipulation of all data in the VGD input structs. The struct members indicate the information:

- A file descriptor for the current log file being used.
- A 4-byte buffer to hold up to a line of data from the log.
- The number of logs in the current log file.
- An array of the file offsets pointing to the beginning of each log.
- The current value of the offset to look at in the log file.

This implementation supports the following functions:

### `VGD_input *VGD_open(const char *fname);`

Provide the filepath to a valid log file, and a pointer to a populated VGD input struct is returned.

If the file cannot be opened (as determined by `fopen()`'s behavior) no struct is allocated and the return is `NULL`. If the file is opened but no valid log information can be read, a struct pointer with only a populated file descriptor is returned for debugging purposes. This is typically due to an `fseek()` failure or passing in a file with no valid logs.

### `FILE *VGD_close(VGD_input **vgd_input);`

When a user no longer needs to read information out of a VGD log file, this function closes the log file and the struct memory is free'd. The *address* of the struct pointer should be passed to the close function since the input struct's reference is `NULL`'d out. If successful, a `NULL` file pointer is returned.

If the log file cannot be closed for whatever reason (indicated by return value of `fclose()`), the struct is still `free()`'d and the offending file's pointer is returned for debugging purposes.

### `unsigned int VGD_select_log(VGD_input *vgd_input, unsigned int log_nbr);`

The input parameter `log_nbr` is constrained to a range of 1 and the total number of logs found in the log file. The struct member indicating the current log file offset is updated according to the log number selected. Upon success, the return is 0. Otherwise, a return of 1 may indicate an invalid log number or an error return value from `fseek()`. A return of greater than 1 indicates an error from `fseek()`.

### `unsigned int VGD_read_line(VGD_input *vgd_input);`

Reads a single line from the selected log in the log file. A successful call to `VGD_select_log()`
must be made before using this function; otherwise, the behavior is undefined. This will read lines until it
hits an EOF or reads in a 4-byte aligned `{'V', 'G', 0xFF, 0xFF}` pattern representing a VGD log header. A return of zero typically indicates that the end of a log. A nonzero return value indicates more lines are left in the log.