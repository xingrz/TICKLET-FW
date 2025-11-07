TICKLET Firmware
================

## Prepare

See [Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```sh
python -m venv .venv
source .venv/bin/activate
pip install west

west init -l app
west update

pip install -r requirements.txt
```

## Build

```sh
west build -p -s app -b ticklet_0404_rev1
```

## License

[Apache-2.0](LICENSE)
