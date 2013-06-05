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

## Results

Using a lookup table for converting binary to base16 results in a 10x speedup
measured on my Intel Core-i7 system and a 14x speedup measured on my BeagleBone
when compared to using `sprintf()`.  This is very significant!

Using a lookup table for converting base16 to binary results in a 3x speedup
measured on my Intel Core-i7 system but only a slightly less than 2x speedup
measured on my Beaglebone when compared to doing a `memcpy()` followed by
`strtol()`.  This isn't very significant but is still a worthwhile improvement.

It's interesting to note that the speedups between my Intel Core-i7 and
BeagleBone rely on system configuration, not just processing power difference.
If they were simply linear with processing power difference, I'd expect to see
similar gains for any algorithm changes which is not what's observed (14x vs 10x
followed by 1.8x vs 3x).  I wonder if a Cortex-A8 with a 32 bit wide memory bus
would fair better than the BeagleBone's 16 bit wide bus.

After more investigation, it seems `isxdigit()` is quite expensive.  By removing
calls to `isxdigit()` the speedup becomes more dramatic but at the possible
expense of encountering error conditions.  Without `isxdigit()` the base16 to
binary LUT measures a 6x improvement on BeagleBone and a 5x improvement on my
Intel Core-i7.
