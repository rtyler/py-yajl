#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
    File for keeping Python 2 specific things, that raise SyntaxError
    exceptions in Python 3
'''


IssueSevenTest_latin1_char = u'f\xe9in'
# u'早安, 爸爸' # Good morning!
IssueSevenTest_chinese_char = u'\u65e9\u5b89, \u7238\u7238'
