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

    def test_Dict(self):
        self.assertDecodesTo('{"key" : "pair"}', {'key' : 'pair'})


if __name__ == '__main__':
    unittest.main()
