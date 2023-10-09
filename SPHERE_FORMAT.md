# Sphere cassette tape format

This documents the binary cassette logical format used by Sphere microcomputers (i.e., the Sphere 1 and other Sphere machines). 

## Overview

The Sphere was an early microcomputer based on the Motorola 6800 and manufactured by Sphere Corp in Utah, from 1975-1977. Its primary nonvolatile storage means was the data cassette, driven by the "SIM/1" serial interface board. That board had a 256-byte onboard "cassette driver" PROM, whose firmware defines this logical format. 

Sphere used the very early (and very slow) 300bps "Kansas City"/Byte format for the generation and reading of audio signals. The KC format is notin the logical bytestream domain and not the audio domain.

The basic unit of data in Sphere cassette storage is a "block" (think: file). Each block can be of variable size and has a two-character "name". In general, a single program may be contained within one block, and in practice many Sphere cassettes contained a single block. Multiple blocks can be stored sequentially on a single cassette, differentiated by their names.

Note that the format encodes the name of the block but (unlike some other object code formats) does not encode load address information. The load address was generally provided on the label of the cassette, and specified at load time by the user. (The block name was also generally provided on the cassette label.)

### Note on block names

Block names are two-byte values, generally ASCII characters that have some useful connection to the program, akin to a rudimentary file name. E.g. the Sphere mini-assembler provided on cassette used `MA` for its block name. 

Some cassette interface firmware (Sphere's "SYS-2") would allow the user to specify a block name, load address, and *block count* to the load command. If the given block count exceeded one, the firmware would increment the (numeric) value of the 16-bit block name when looking for subsequent blocks. (This suggests short sequences of block names such as `B0`, `B1`, `B2` etc.) Other firmware versions did not use the block count idea.

## Block format

Each block on a cassette uses the following sequential format:

- (**3** bytes) synchronization bytes. Constant value `0x16` repeated for the three bytes.
- (**1** byte) escape marker. Constant value `0x1B`
- (**2** bytes) 16-bit count of data bytes, minus one. Stored high byte first.
- (**2** bytes) block "name" (typically two ASCII characters, but any 16-bit value is possible)
- (raw data) binary data for this block. The count of these bytes is equal to the count stored above, plus one. (There are thus no "zero-length" blocks.)
- (**1** byte) end-of-transmission marker. Constant value `0x17`
- (**1** byte) checksum byte (see below).
- (**3** bytes) final trailer bytes. (In practice usually the same value as the checksum, just repeated three more times.)

The checksum is just a trivial 8-bit sum across the data portion (only). The 8-bit counter rolls over when it overflows. _"Check sum"_ indeed!

The final trailer bytes are in practice optional and appear to have been in some cases omitted; default Sphere firmware will write them out, but they are not validated on read as long as the checksum matches.
 
There is no overall "catalog" structure for a cassette. A cassette can have multiple blocks present on the tape, which are typically differentiated to the user by the "name". 

### Note on synchronization

The cassette load firmware in the `SYS2NF` cassette ROM can be asked to load a specific block by name. If there are multiple blocks on the tape, the firmware will skip non-matching blocks and attempt to re-sync at the start of each following one. This should generally be a reliable system. However, is it _possible_ that the fixed header sync sequence (four bytes: `0x16` `0x16` `0x16` `0x1B`) occurs by chance within a valid data block. It's not *especially* likely, but for example the non-gibberish instruction sequence of `LDAA $1616; TAB; ABA` could cause a false sync, which could throw off the rest of the tape read.

For this reason, if you are creating cassette images, ensure that either (1) you only use one block per image, or (2) you don't have accidental sync sequences in your data stream. 

-----

This format was reverse-engineered from Sphere firmware and original example cassettes by Ben Zotto, 2022.

