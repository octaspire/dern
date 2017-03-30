#This file has some influences from pygments/lexers/lisp.py
import re
import pygments
from pygments.lexer import RegexLexer
from pygments.token import *

__all__ = ['DernLexer']

class DernLexer(RegexLexer):
    """All your code goes here"""

    name= 'Dern'
    aliases = ['dern']
    filenames = '*.dern'
    flags = re.MULTILINE | re.DOTALL

    builtins = (
            'if', 'define'
    )

    valid_name = r'[\w!$%&*/\:;,.<=>?@^~]+'

    tokens = {
            'root' : [
                (r';.*$', Comment.Single),
                (r'\s+',  Text), #Whitespace
                (r'(true|false|nil)',  Name.Constant),
                (r'-?\d+\.\d+', Number.Float),
                (r'-?\d+', Number.Integer),
                ("(?<=\()(%s)" % '|'.join(re.escape(entry) + ' ' for entry in builtins), Name.Builtin),
                (r'(?<=\()' + valid_name, Name.Function),
                (valid_name, Name.Variable),
                (r'(\(|\))', Punctuation), #Parentheses
                ('\[', String, 'string'),
                (r"'" + valid_name, String.Symbol),
                (r"('|\.)", Operator),
                (r'(\|.?\||\|bar\||\|newline\||\|tab\|)', String.Char),
                ],
                'string' : [
                    ('[^\]]+', String),
                    ('\]', String, '#pop'),
                    ]
            }
