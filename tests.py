#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import unittest

if sys.version_info[0] == 3:
    from io import StringIO
else:
    from StringIO import StringIO


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

    # Python 3 version
    #def test_UnicodeDict(self):
    #        self.assertEncodesTo({'foō' : 'bār'}, '{"foō":"bār"}')

    # Python 2 version
    #def test_UnicodeDict(self):
    #        self.assertEncodesTo({u'foō' : u'bār'}, '{"foō":"bār"}')

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


class StreamBlockingDecodingTests(unittest.TestCase):
    def setUp(self):
        self.stream = StringIO('{"foo":["one","two", ["three", "four"]]}')

    def test_no_object(self):
        self.failUnlessRaises(TypeError, yajl.load)

    def test_bad_object(self):
        self.failUnlessRaises(TypeError, yajl.load, 'this is no stream!')

    def test_simple_decode(self):
        obj = yajl.load(self.stream)
        self.assertEquals(obj, {'foo' : ['one', 'two', ['three', 'four']]})

class StreamIterDecodingTests(object): # TODO: Change to unittest.TestCase when I start to think about iterative
    def setUp(self):
        self.stream = StringIO('{"foo":["one","two",["three", "four"]]}')

    def test_no_object(self):
        self.failUnlessRaises(TypeError, yajl.iterload)

    def test_bad_object(self):
        self.failUnlessRaises(TypeError, yajl.iterload, 'this is no stream!')

    def test_simple_decode(self):
        for k, v in yajl.iterload(self.stream):
            print(k, v)


class StreamEncodingTests(unittest.TestCase):
    def test_blocking_encode(self):
        obj = {'foo' : ['one', 'two', ['three', 'four']]}
        stream = StringIO()
        buffer = yajl.dump(obj, stream)
        self.assertEquals(stream.getvalue(), '{"foo":["one","two",["three","four"]]}')


if __name__ == '__main__':
    unittest.main()
