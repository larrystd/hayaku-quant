# Optional Python integration

This directory keeps the existing Python interface while C++ is the default
development path. The C++ engine and tests remain in `../hayaku_cpp/`.

- `hayaku/`: installable Python API
- `hayaku_pywrap/`: pybind11 bindings to the C++ libraries
- `hayaku_ingest_native/`, `hayaku_realtime_native/`: optional native packages
- `tests/`, `examples/`: Python regression tests and examples; older docs
  examples are preserved in `examples/legacy_docs/`
- `tools/`, `wheels/`, `setup.py`: staging and wheel packaging

From the repository root, use Python 3.10:

```sh
python3.10 -m pip install -r python/requirements.txt
./op.sh python-build
./op.sh python-smoke
./op.sh python-test
./op.sh import-test
```

The three wheel commands are `./op.sh wheel`, `./op.sh wheel-ingest`, and
`./op.sh wheel-realtime`. They write to `python/dist/`. The regular
`./op.sh build`, `./op.sh test`, and `./op.sh all` commands work on C++ only.
