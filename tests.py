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

    def test_Integer(self):
        self.assertDecodesTo("1234", 1234)

    def test_Float(self):
        self.assertDecodesTo("53.2", 53.2)

    def test_List(self):
        self.assertDecodesTo('[1,2]', [1, 2])

    def test_ListOfFloats(self):
        self.assertDecodesTo('[3.14, 2.718]', [3.14, 2.718])

    def test_ListOfBools(self):
        self.assertDecodesTo('[true, true, false, true, false, false]',
                [True, True, False, True, False, False])

    def test_ListOfStrings(self):
        self.assertDecodesTo('["a", "foo", "bar", "blah"]',
                ['a', 'foo', 'bar', 'blah'])

    def test_ListOfNulls(self):
        self.assertDecodesTo('[null, null, null]', [None, None, None])

    def test_List_WhitespaceIgnored(self):
        self.assertEquals(self.decode("[1,2,3,4,5]"), self.decode("[1, 2, 3, 4, 5]"))

    def test_HeterogeneousList(self):
        self.assertDecodesTo('[1, 1.3, "foobar", null, true]', [1, 1.3, "foobar", None, True])

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

    def assertEncodesTo(self, value, expected):
        got = self.encode(value)
        assert expected == got, ("Failed to encode JSON correctly. expected: '%s', got: '%s'" % (expected, got))
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
    def python3_UnicodeDict(self):
        self.assertEncodesTo({'foō' : 'bār'}, '{"fo\\u014d": "b\\u0101r"}')

    # Python 2 version
    def python2_UnicodeDict(self):
        self.assertEncodesTo(
                {'fo\xc5\x8d'.decode('utf-8'): 'b\xc4\x81r'.decode('utf-8')},
                r'{"fo\u014d": r"b\u0101r"}')

    def test_NestedDictAndList(self):
        self.assertEncodesTo({'key' : {'subkey' : [1,2,3]}},
            '{"key":{"subkey":[1,2,3]}}')

if sys.version_info[0] > 2:
    BasicJSONEncodeTests.test_UnicodeDict = BasicJSONEncodeTests.python3_UnicodeDict
    del BasicJSONEncodeTests.python3_UnicodeDict
    BasicJSONEncodeTests.test_UnicodeDict.__name__ = "test_UnicodeDict"
else:
    BasicJSONEncodeTests.test_UnicodeDict = BasicJSONEncodeTests.python2_UnicodeDict
    del BasicJSONEncodeTests.python2_UnicodeDict
    BasicJSONEncodeTests.test_UnicodeDict.im_func.__name__ = "test_UnicodeDict"


class LoadsTest(BasicJSONDecodeTests):
    def decode(self, json):
        return yajl.loads(json)


class DumpsTest(BasicJSONEncodeTests):
    def encode(self, value):
        return yajl.dumps(value)


class DecodeErrorTests(unittest.TestCase):
    def decode(self, text):
        return yajl.Decoder().decode(text)

    def assertDecodeFails(self, exc_type, text):
        self.assertRaises(exc_type, self.decode, text)

    def test_EmptyString(self):
        self.assertDecodeFails(ValueError, '')

    def test_None(self):
        self.assertDecodeFails(ValueError, None)

    def test_Nonsense(self):
        self.assertDecodeFails(ValueError, "asdjklfakl2")

    def test_BadInt(self):
        self.assertDecodeFails(ValueError, "123xxx")

    def test_BadFloat(self):
        self.assertDecodeFails(ValueError, "4.5xxx")


class LoadsErrorTests(DecodeErrorTests):
    def decode(self, text):
        return yajl.loads(text)


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
