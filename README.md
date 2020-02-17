# Plugin providing named pipe client functions for Kirikiri

This plugin adds named pipe client functions in Kirikiri2 / 吉里吉里2 / KirikiriZ / 吉里吉里Z

## Building

After cloning submodules and placing `ncbind` and `tp_stub` in the parent directory, a simple `make` will generate `krnamedpipeclient.dll`.

## How to use

After `Plugins.link("krnamedpipeclient.dll");` is used, the additional functions will be exposed under the `NamedPipeClient` class.

## License

This project is licensed under the MIT license. Please read the `LICENSE` file for more information.
