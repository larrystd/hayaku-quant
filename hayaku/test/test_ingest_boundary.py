"""Focused checks for the optional ingestion build and Python boundary."""

import subprocess
import sys
import unittest
from types import SimpleNamespace
from unittest import mock

from hayaku import ingest


class IngestBoundaryTest(unittest.TestCase):

    def test_core_import_does_not_load_ingest_extension(self):
        code = """
import sys
class BlockIngest:
    def find_spec(self, fullname, path=None, target=None):
        if fullname.startswith('hayaku_ingest_native.ingest'):
            raise AssertionError('core import loaded optional ingestion extension')
sys.meta_path.insert(0, BlockIngest())
import hayaku
import hayaku.advanced
assert not any(name.startswith('hayaku_ingest_native.ingest') for name in sys.modules)
"""
        result = subprocess.run((sys.executable, "-c", code), capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_selected_backend_only_is_constructed(self):
        constructed = []

        def fake_type(name):
            class FakeImporter:
                def __init__(self):
                    constructed.append(name)

                def set_config(self, *args):
                    self.args = args
                    return True

            return FakeImporter

        native = SimpleNamespace(
            KDataToHdf5Importer=fake_type("hdf5"),
            KDataToMySQLImporter=fake_type("mysql"),
            KDataToClickHouseImporter=fake_type("clickhouse"),
        )
        with mock.patch.object(ingest, "_load_native", return_value=native):
            hdf5 = ingest.open_kdata_importer("HDF5", datapath="/tmp/data")
            self.assertEqual(constructed, ["hdf5"])
            self.assertEqual(hdf5.args[0], "/tmp/data")

            mysql = ingest.open_kdata_importer("mysql", host="localhost")
            self.assertEqual(constructed, ["hdf5", "mysql"])
            self.assertEqual(mysql.args, ("localhost", 3306, "root", "", "hayaku_base"))

            clickhouse = ingest.open_kdata_importer("clickhouse", host="localhost")
            self.assertEqual(constructed, ["hdf5", "mysql", "clickhouse"])
            self.assertEqual(clickhouse.args, ("localhost", 9000, "default", "", "hayaku_base"))

    def test_invalid_configuration_and_backend(self):
        native = SimpleNamespace(KDataToHdf5Importer=lambda: SimpleNamespace(set_config=lambda *args: False))
        with mock.patch.object(ingest, "_load_native", return_value=native):
            self.assertIsNone(ingest.open_kdata_importer("hdf5", datapath="/tmp/data"))
        with self.assertRaisesRegex(ValueError, "datapath"):
            ingest.open_kdata_importer("hdf5")
        with self.assertRaisesRegex(ValueError, "unsupported"):
            ingest.open_kdata_importer("postgres")

    def test_missing_extension_has_actionable_error(self):
        module_name = f"hayaku_ingest_native.ingest{sys.version_info.major}{sys.version_info.minor}"
        missing = ModuleNotFoundError(f"No module named {module_name}", name=module_name)
        with mock.patch.object(ingest.importlib, "import_module", side_effect=missing):
            with self.assertRaisesRegex(ImportError, "xmake ingest"):
                ingest._load_native()
        missing_package = ModuleNotFoundError("No module named hayaku_ingest_native",
                                              name="hayaku_ingest_native")
        with mock.patch.object(ingest.importlib, "import_module", side_effect=missing_package):
            with self.assertRaisesRegex(ImportError, "xmake ingest"):
                ingest._load_native()

    def test_legacy_imports_are_removed(self):
        import hayaku.core
        import hayaku.advanced

        self.assertFalse(hasattr(hayaku.core, "KDataToHdf5Importer"))
        self.assertFalse(hasattr(hayaku.advanced, "KDataToMySQLImporter"))
        self.assertTrue(hasattr(ingest, "KDataToHdf5Importer"))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(IngestBoundaryTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
