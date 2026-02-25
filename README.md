# MLXkit
Modern support for 80s-era paper binary distribution

_Compute!'s Gazette_ published many pieces of C64 software in magazines and books as raw binary dumps to be entered into the computer's RAM, one byte at a time. To ease this process they created several versions of a support program called "MLX", which supplemented the listing with a checksum every few bytes to ensure that the program was entered correctly. 

In 2025, the _Gazette_ brand was revived and in their January 2026 issue they published a third edition of MLX.

These three versions of MLX, released in 1983, 1986, and 2026, differ primarily in their checksum algorithms and in how many bytes are represented in each checksum.

MLXkit replicates the checksum and output formats closely enough to output C64 binary programs as MLX listings. The 1983 edition sometimes required special preparation; the listings in MLX 1 mode will provide these instructions.

## Usage

MLXkit is a command line program. The command `mlxkit sample.prg` will load the C64 binary `sample.prg` and output a listing for it in the MLX 2 format. The options `-1`, `-2`, and `-3` select which version to use if the default is not desired.

## License and Credits

MLXkit is Copyright 2026 Michael C. Martin. It is provided under the terms of the zlib license.
