"""Protect the hardware-verified MCO integration from incidental cleanup edits.

The only application-integration exception is removal of four obsolete battery
PDO registrations in user.c. The CANopen stack and Node-ID-zero handling stay pinned.
"""
from pathlib import Path
import hashlib
import unittest

ROOT = Path(__file__).resolve().parents[1]
# Exact bytes from hardware-verified commit 65ced39, except user.c:
# its reviewed battery-PDO removal is pinned separately (no stack changes).
BASELINE = {'Embedded/Src/mco.c': 'c36586beb8071ea3f28f045e0309bfc84e64aaf59b50358f0e9a8ace7efb28bb', 'Embedded/Src/mcohw.c': '954fb77f469ae4cc60c251b93aa88a4a846aeb2ebe6176647aca4459ec95868e', 'Embedded/Src/user.c': '42dd2cc04583d69c262699c5168b2874666554c2fe63480b55f4c56eb6fb84b2', 'Embedded/Include/mco.h': '20e311809e32b1ada88db3f8178c785c8b1217a9249e82028998295f40b91270', 'Embedded/Include/mcohw.h': '20564d6fa36b8fc95e4dfb2c94fa6a5f2e836f0820b00de6c0ed175f5680f672', 'Embedded/Include/nodecfg.h': '403b462815c522478b222bb790391f02fe9f66aa88667d4e4586916590d7ad5d', 'Embedded/Include/procimg.h': 'a72a66ddae8593231b2bc27261b0ca1371b5033503568e961f79a3b1c8040269'}


class MCOBaselineTests(unittest.TestCase):
    def test_mco_integration_matches_working_baseline(self):
        for path, expected in BASELINE.items():
            with self.subTest(path=path):
                # Git checks out text using platform line endings on Windows.
                content = (ROOT/path).read_bytes().replace(b'\r\n', b'\n')
                self.assertEqual(hashlib.sha256(content).hexdigest(), expected)
