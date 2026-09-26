"""Tests for the lower-layer/application dependency guard."""

from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from check_app_dependencies import scan


class CheckAppDependenciesTest(unittest.TestCase):
    def setUp(self):
        self.workspace = TemporaryDirectory()
        self.addCleanup(self.workspace.cleanup)
        self.root = Path(self.workspace.name)
        (self.root / "hayaku_cpp/src/data").mkdir(parents=True)
        (self.root / "hayaku_cpp/src/operators").mkdir(parents=True)
        (self.root / "hayaku_cpp/src/metrics").mkdir(parents=True)

    def test_allows_comments_and_core_includes(self):
        source = self.root / "hayaku_cpp/src/data/Stock.cpp"
        source.write_text(
            '// #include "application/GlobalInitializer.h"\n'
            '/* src/application/PluginRuntime.h */\n'
            '#include "data/Stock.h"\n',
            encoding="utf-8",
        )
        self.assertEqual(scan(self.root), (1, []))

    def test_rejects_app_include_and_path(self):
        source = self.root / "hayaku_cpp/src/operators/Report.cpp"
        source.write_text(
            '#include "../../application/PluginRuntime.h"\n'
            'auto path = "hayaku_cpp/src/application/runtime";\n',
            encoding="utf-8",
        )
        scanned, violations = scan(self.root)
        self.assertEqual(scanned, 1)
        self.assertEqual(len(violations), 2)
        self.assertIn("Report.cpp:1", violations[0])
        self.assertIn("Report.cpp:2", violations[1])

    def test_missing_directory_is_an_error(self):
        (self.root / "hayaku_cpp/src/metrics").rmdir()
        with self.assertRaises(FileNotFoundError):
            scan(self.root)


if __name__ == "__main__":
    unittest.main()
