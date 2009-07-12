import unittest

import yajl

class BasicJSONTests(unittest.TestCase):
    def setUp(self):
        self.d = yajl.Decoder()

    def assertDecodesTo(self, json, value):
        rc = self.d.decode(json)
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

class ErrorCasesTests(unittest.TestCase):
    def setUp(self):
        self.d = yajl.Decoder()

    def test_EmptyString(self):
        self.failUnlessRaises(ValueError, self.d.decode, '')

    def test_None(self):
        self.failUnlessRaises(ValueError, self.d.decode, None)


if __name__ == '__main__':
    unittest.main()
