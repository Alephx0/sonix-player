#!/usr/bin/env python3
"""Exercise the real packer guard without generating or flashing firmware."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
GUARD = ROOT / 'sonix-packer/r1_compatibility_check.sh'


class PackagingGuard(unittest.TestCase):
    def check(self, stock, overlay):
        return subprocess.run(['bash', '-c', 'source "$1"; r1_compatibility_check "$2" "$3"',
                               'guard', str(GUARD), str(stock), str(overlay)], capture_output=True).returncode

    def test_stock_revision_guard(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            stock = root / 'stock'
            overlay = root / 'overlay'
            stock.mkdir()
            info = overlay / 'usr/resource/sonix/components/system-info.json'
            info.parent.mkdir(parents=True)
            info.write_text('{"device-name":"HiBy R1"}')
            self.assertEqual(self.check(stock, overlay), 0)
            for marker in ['module_driver/r1_wireless_select.sh', 'module_driver/aic8800_fdrv.ko',
                           'etc/init.d/S52wifi_aic_init_config']:
                file = stock / marker
                file.parent.mkdir(parents=True, exist_ok=True)
                file.touch()
                self.assertNotEqual(self.check(stock, overlay), 0, marker)
                file.unlink()
            info.write_text('{"device-name":"HiBy R3 Pro II"}')
            self.assertNotEqual(self.check(stock, overlay), 0)

    def test_size_bounds(self):
        for size, stock, ok in [('100', '100', True), ('99', '100', True), ('101', '100', False),
                                ('1', '0', False), ('x', '100', False)]:
            rc = subprocess.run(['bash', '-c', 'source "$1"; r1_rootfs_size_check "$2" "$3"',
                                 'guard', str(GUARD), size, stock], capture_output=True).returncode
            self.assertEqual(rc == 0, ok)


if __name__ == '__main__':
    unittest.main()
