#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest

import yajl

class BasicJSONDecodeTests(unittest.TestCase):
    def decode(self, json):
        return yajl.Decoder().decode(json)

    def assertDecodesTo(self, json, value):
        rc = self.decode(json)
        assert rc == value, ('Failed to decode JSON correctly', 
                json, value, rc)
        return True
    
    def test_TrueBool(self):
        self.assertDecodesTo('true', True)

    def test_FalseBool(self):
        self.assertDecodesTo('false', False)

    def test_Null(self):
        self.assertDecodesTo('null', None)

    def test_List(self):
        self.assertDecodesTo('[1,2]', [1, 2])

    def test_ListOfFloats(self):
        self.assertDecodesTo('[3.14, 2.718]', [3.14, 2.718])

    def test_Dict(self):
        self.assertDecodesTo('{"key" : "pair"}', {'key' : 'pair'})

    def test_ListInDict(self):
        self.assertDecodesTo('''
            {"key" : [1, 2, 3]}
        ''', {'key' : [1, 2, 3]})

    def test_DictInDict(self):
        self.assertDecodesTo('''
            {"key" : {"subkey" : true}}''',
                {'key' : {'subkey' : True}})

    def test_NestedDictAndList(self):
        self.assertDecodesTo('''
            {"key" : {"subkey" : [1, 2, 3]}}''',
                {'key' : {'subkey' : [1,2,3]}})

class BasicJSONEncodeTests(unittest.TestCase):
    def encode(self, value):
        return yajl.Encoder().encode(value)

    def assertEncodesTo(self, value, json):
        rc = self.encode(value)
        assert rc == json, ('Failed to encode JSON correctly', locals())
        return True

    def test_TrueBool(self):
        self.assertEncodesTo(True, 'true')

    def test_FalseBool(self):
        self.assertEncodesTo(False, 'false')

    def test_Null(self):
        self.assertEncodesTo(None, 'null')

    def test_List(self):
        self.assertEncodesTo([1,2], '[1,2]')

    def test_Dict(self):
        self.assertEncodesTo({'key' : 'value'}, '{"key":"value"}')

    def test_UnicodeDict(self):
        self.assertEncodesTo({u'foō' : u'bār'}, '{"foō":"bār"}')

    def test_NestedDictAndList(self):
        self.assertEncodesTo({'key' : {'subkey' : [1,2,3]}},
            '{"key":{"subkey":[1,2,3]}}')


class LoadsTest(BasicJSONDecodeTests):
    def decode(self, json):
        return yajl.loads(json)

class DumpsTest(BasicJSONEncodeTests):
    def encode(self, value):
        return yajl.dumps(value)

class ErrorCasesTests(unittest.TestCase):
    def setUp(self):
        self.d = yajl.Decoder()

    def test_EmptyString(self):
        self.failUnlessRaises(ValueError, self.d.decode, '')

    def test_None(self):
        self.failUnlessRaises(ValueError, self.d.decode, None)


if __name__ == '__main__':
    unittest.main()
