"""Focused checks for the optional realtime Python boundary."""

import importlib.util
import subprocess
import sys
import unittest


class RealtimeBoundaryTest(unittest.TestCase):

    def test_core_and_advanced_import_without_realtime_package(self):
        code = """
import sys

class BlockRealtime:
    def find_spec(self, fullname, path=None, target=None):
        if fullname.startswith('hayaku_realtime_native'):
            raise ModuleNotFoundError(name='hayaku_realtime_native')

sys.meta_path.insert(0, BlockRealtime())
import hayaku
import hayaku.advanced as advanced
assert not any(name.startswith('hayaku_realtime_native') for name in sys.modules)
try:
    advanced.start_spot_agent
except ImportError as error:
    assert 'hayaku-realtime' in str(error)
else:
    raise AssertionError('missing realtime package was not reported')
"""
        result = subprocess.run((sys.executable, "-c", code), capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_explicit_access_loads_realtime_extension(self):
        if importlib.util.find_spec("hayaku_realtime_native") is None:
            self.skipTest("optional realtime extension is not installed")
        import hayaku.advanced as advanced

        self.assertTrue(callable(advanced.start_spot_agent))
        self.assertFalse(advanced.is_shm_server_running())


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(RealtimeBoundaryTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
