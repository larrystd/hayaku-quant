#!/usr/bin/env bash
set -euo pipefail

test_binary="$1"
shift
runfiles="${TEST_SRCDIR}/${TEST_WORKSPACE}"
workdir="${TEST_TMPDIR}/hayaku_test_run"
mkdir -p "${workdir}"
export HOME="${workdir}/home"
mkdir -p "${HOME}"
cp -R "${runfiles}/test_data" "${workdir}/test_data"
cp -R "${runfiles}/hayaku/plugin" "${workdir}/plugin"
cp -R "${runfiles}/i18n" "${workdir}/i18n"
mkdir -p "${workdir}/test_data/tmp"
cd "${workdir}"
exec "${runfiles}/${test_binary}" "$@"
