Base 16 Tests
=============

## Purpose

To test the speed of performing different base16 to binary and binary to base16
conversions on various platforms.  Converting base16 to binary and back is very
important to certain applications, doing this as fast as possible for the
platform in question (software / hardware combo) may not be a trivial problem.

## Usage

```
make
./test FILE
```

Where FILE is the input to operate on.  Up to 32 MiB of FILE will be read in and
operated on by each different test.  For a quick test, use `/dev/urandom` for
FILE.
