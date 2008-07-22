#!/usr/bin/env python

import _sphinx3
import unittest

sphinx_config = { 'hmm' : '../model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd',
                  'lm' : '../model/lm/an4/an4.ug.lm.DMP',
                  'dict' : '../model/lm/an4/an4.dict',
                  'fdict' : '../model/lm/an4/filler.dict', }

class TestDecode(unittest.TestCase):
    def setUp(self):
        _sphinx3.parse_argdict(sphinx_config)
        _sphinx3.init()

    def tearDown(self):
        _sphinx3.close()

    def test_decode_raw(self):
        wav = open('../model/lm/an4/pittsburgh.littleendian.raw', 'rb')
        data = wav.read()
        text, segs = _sphinx3.decode_raw(data, 'foo')
        self.assertEqual(text, "P I T G S B U R G H")

    def test_process_raw(self):
        wav = open('../model/lm/an4/pittsburgh.littleendian.raw', 'rb')
        data = wav.read()
        _sphinx3.begin_utt('foo')
        for i in range(0, len(data)/4096):
            start = i * 4096
            end = min(len(data), (i + 1) * 4096)
            _sphinx3.process_raw(data[start:end])
        _sphinx3.end_utt()
        text, segs = _sphinx3.get_hypothesis()
        self.assertEqual(text, "K P I T T YES AND A U R G H")

    def test_decode_cep_file(self):
        text, segs = _sphinx3.decode_cep_file('../model/lm/an4/pittsburgh.littleendian.mfc')
        self.assertEqual(text, "P I T G S B U R G H")

if __name__ == '__main__':
    unittest.main()
