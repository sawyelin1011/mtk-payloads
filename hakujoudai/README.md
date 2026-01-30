## Hakujoudai (白杖代)

This directory contains Hakujoudai, a payload for MediaTek V6/XML devices exploiting [heapb8](https://blog.r0rt1z2.com/posts/exploiting-mediatek-datwo/).

### Supported cmds

- `CMD:BOOT-TO` - used to boot [DA extensions](../da_xml/README.md)
- `CMD:EXP-PATCH-MEM` - used to patch memory
- `CMD:EXP-CALL-FUNC` - used to call arbitrary functions

### Buuilding

You'll need the arm-none-eabi toolchain and aarch64-none-linux-gnu to build the payload.

To build the payload, run:

```bash
make clean
make
```

### License

This payload is licensed under the *AGPL-3.0-or-later* license, copyright (C) 2026 Shomy, R0rt1z2.
See the [LICENSE](../LICENSE.agpl) file for details.

Additionally, this payload makes use of:
* [nanoprintf](lib/nanoprintf.h), dual-licensed under the *Unlicense* and the *Zero-Clause BSD (0BSD)* licenses, copyright (C) 2019 Charles Nicholson. Original source [here](https://github.com/charlesnicholson/nanoprintf)
* [libc](src/libc.c), of which some functions are derived from the work of musl libc. Copyright (c) 2005-2018 Rich Felker.

### Disclaimer

This payload is intended for educational purposes only. Use it at your own risk. The authors are not responsible for any damange caused by using thus payload.
