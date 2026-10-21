# PIC32 legacy peripheral library (plib)

This directory vendors the **PIC32MX legacy peripheral library** (`plib.h` +
`peripheral/*.h` + `libmchp_peripheral_32MX360F256L.a`), which Microchip no
longer ships with the XC32 compiler (removed since XC32 v1.40 / fully removed
in v4.x+). The firmware uses the legacy plib API extensively (`OpenTimer3`,
`mCNOpen`, `SetDCOC1PWM`, ...), so the headers and the prebuilt library for
the **PIC32MX360F256L** are vendored here.

## Source

Extracted from the last MPIDE release (chipKIT), which bundles the Microchip
PIC32 peripheral library for the exact device this firmware targets:

- Archive: `mpide-0150-linux64-20150820.tgz`
- URL: https://chipkit.s3.amazonaws.com/0150release/mpide-0150-linux64-20150820.tgz
- Path inside archive:
  `hardware/pic32/compiler/pic32-tools/pic32mx/{include/plib.h,include/peripheral,lib/libmchp_peripheral_32MX360F256L.a}`

## License

Microchip Software License Agreement (see the header of `plib.h` and
`peripheral/*.h`). The library is supplied "AS IS" by Microchip for use solely
on Microchip PICmicro microcontroller products.

## Usage

The Makefile adds `-Ipic32libs` and `-Ipic32libs/peripheral` to the include
path and links `pic32libs/libmchp_peripheral_32MX360F256L.a` explicitly.

If you install the official "PIC32 Legacy Peripheral Libraries" package from
Microchip on top of XC32, you can instead point the build at the compiler's
own include/lib directories (set `PLIB_DIR` accordingly).
