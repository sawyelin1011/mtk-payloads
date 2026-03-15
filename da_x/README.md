## DA Extensions for V5/XFlash devices

This directory contains DA Extensions for V5/XFlash devices, allowing to define custom commands during DA2.

DA Extensions are originally developed by [B.Kerler](https://github.com/bkerler) for V5 devices, and later adapted for more cmds by [Shomy](https://github.com/shomykohai).

### Supported cmds

- `0xF0000` - CMD ACK, used to aknowledge wether da extensions got loaded
- `0xF0001` - CMD DA CTX, used to setup the extensions context
- `0xF0002` - CMD READMEM, used to read memory from device
- `0xF0003` - CMD WRITEMEM, used to write memory to device
- `0xF0004` - CMD READREG, used to read registers from device
- `0xF0005` - CMD WRITEREG, used to write registers to device
- `0xF0006` - CMD KEY DERIVE, used to derive keys using the device crypto cell
- `0xF0007` - CMD SEJ AES, used to interact with SEJ for performing crypto operations
- `0xF0008` - CMD RPMB INIT, used to init RPMB key. Can be call called only after CMD DA CTX.
- `0xF0009` - CMD RPMB READ, used to read RPMB. Can be call called only after CMD DA CTX.
- `0xF000A` - CMD RPMB WRITE, used to write RPMB. Can be call called only after CMD DA CTX.

### Building

You'll need the arm-linux-gnueabihf toolchain to build the payload.

To build the payload, run:

```bash
make clean
make
```

### License
This payload is licensed under the *AGPL-3.0-or-later* License, copyright (C) 2026 Shomy.
See the [LICENSE](../LICENSE.agpl) file for details.

Additionally, this payload makes use of: 
* [libsej](../libsej), which is licensed under the *GPL-3.0* License, copyright (C) 2024 B.Kerler, 2025 Shomy.
* [nanoprintf](../common/include/nanoprintf.h), dual-licensed under the *Unlicense* and the *Zero-Clause BSD (0BSD)* licenses, copyright (C) 2019 Charles Nicholson. Original source [here](https://github.com/charlesnicholson/nanoprintf)
* [libc](../common/libc.c), of which some functions are derived from the work of musl libc. Copyright (c) 2005-2018 Rich Felker.

### Disclaimer
This payload is intended for educational purposes only. Use it at your own risk. The author is not responsible for any damage caused by using this payload.
