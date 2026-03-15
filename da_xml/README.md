## DA Extensions for V6/XML devices

This directory contains DA Extensions for V6/XML  devices, allowing to define custom commands during DA2.

### Supported cmds

- `CMD:EXT-ACK` - used to aknowledge whether da extensions got loaded or not
- `CMD:EXT-DA-CTX` - used to setup the extensions context
- `CMD:EXT-READMEM` - used to read memory from device
- `CMD:EXT-WRITEMEM` - used to write memory to device
- `CMD:EXT-READREG` - used to read registers from device
- `CMD:EXT-WRITEREG` - used to write registers to device
- `CMD:EXT-KEY-DERIVE` - used to derive keys using the device crypto cell
- `CMD:EXT-SEJ-AES` - used to interact with SEJ for performing crypto operations
- `CMD:EXT-RPMB-INIT` - used to init RPMB key. Can be call called only after CMD:EXT-DA-CTX.
- `CMD:EXT-RPMB-READ` - used to read RPMB. Can be call called only after CMD:EXT-DA-CTX.
- `CMD:EXT-RPMB-WRITE` - used to write RPMB. Can be call called only after CMD:EXT-DA-CTX.

### Building

You'll need the arm-none-eabi toolchain and aarch64-none-linux-gnu to build the payload.

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
