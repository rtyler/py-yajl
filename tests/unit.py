#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import unittest

if sys.version_info[0] == 3:
    from io import StringIO
else:
    from StringIO import StringIO

import yajl

class DecoderBase(unittest.TestCase):
    def decode(self, json):
        return yajl.Decoder().decode(json)

    def assertDecodesTo(self, json, value):
        rc = self.decode(json)
        assert rc == value, ('Failed to decode JSON correctly', 
                json, value, rc)
        return True

class BasicJSONDecodeTests(DecoderBase):
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


class EncoderBase(unittest.TestCase):
    def encode(self, value):
        return yajl.Encoder().encode(value)

    def assertEncodesTo(self, value, json):
        rc = self.encode(value)
        assert rc == json, ('Failed to encode JSON correctly', locals())
        return True

class BasicJSONEncodeTests(EncoderBase):
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

class DumpsOptionsTests(unittest.TestCase):
    def test_indent_four(self):
        rc = yajl.dumps({'foo' : 'bar'}, indent=4)
        expected = '{\n    "foo": "bar"\n}\n'
        self.assertEquals(rc, expected)

    def test_indent_zero(self):
        rc = yajl.dumps({'foo' : 'bar'}, indent=0)
        expected = '{\n"foo": "bar"\n}\n'
        self.assertEquals(rc, expected)

    def test_indent_str(self):
        self.failUnlessRaises(TypeError, yajl.dumps, {'foo' : 'bar'}, indent='4')

    def test_negative_indent(self):
        ''' Negative `indent` should not result in pretty printing '''
        rc = yajl.dumps({'foo' : 'bar'}, indent=-1)
        self.assertEquals(rc, '{"foo":"bar"}')

    def test_none_indent(self):
        ''' None `indent` should not result in pretty printing '''
        rc = yajl.dumps({'foo' : 'bar'}, indent=None)
        self.assertEquals(rc, '{"foo":"bar"}')

class DumpOptionsTests(unittest.TestCase):
    stream = None
    def setUp(self):
        self.stream = StringIO()

    def test_indent_four(self):
        rc = yajl.dump({'foo' : 'bar'}, self.stream, indent=4)
        expected = '{\n    "foo": "bar"\n}\n'
        self.assertEquals(self.stream.getvalue(), expected)

    def test_indent_zero(self):
        rc = yajl.dump({'foo' : 'bar'}, self.stream, indent=0)
        expected = '{\n"foo": "bar"\n}\n'
        self.assertEquals(self.stream.getvalue(), expected)

    def test_indent_str(self):
        self.failUnlessRaises(TypeError, yajl.dump, {'foo' : 'bar'}, self.stream, indent='4')

    def test_negative_indent(self):
        ''' Negative `indent` should not result in pretty printing '''
        rc = yajl.dump({'foo' : 'bar'}, self.stream, indent=-1)
        self.assertEquals(self.stream.getvalue(), '{"foo":"bar"}')

    def test_none_indent(self):
        ''' None `indent` should not result in pretty printing '''
        rc = yajl.dump({'foo' : 'bar'}, self.stream, indent=None)
        self.assertEquals(self.stream.getvalue(), '{"foo":"bar"}')


class IssueEightTest(unittest.TestCase):
    def runTest(self):
        ''' http://github.com/rtyler/py-yajl/issues#issue/8 '''
        encoded = yajl.dumps([(2,3,)])
        decoded = yajl.loads(encoded)
        self.assertEquals(len(decoded), 1)
        self.assertEquals(decoded[0][0], 2)
        self.assertEquals(decoded[0][1], 3)

if __name__ == '__main__':
    verbosity = '-v' in sys.argv and 2 or 1
    runner = unittest.TextTestRunner(verbosity=verbosity)
    if 'xml' in sys.argv:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(filename='Yajl-Tests.xml')
    suites = unittest.findTestCases(sys.modules[__name__])
    results = runner.run(unittest.TestSuite(suites))

