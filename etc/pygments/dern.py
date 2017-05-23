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
        '!=', '*', '+', '++', '+=', '-', '--', '-=', '/', '<', '<=', '=', '==', '>', '>=', 'abort',
        'and', 'define', 'do', 'doc', 'env-current', 'env-global', 'env-new', 'eval', 'exit',
        'find', 'fn', 'for', 'hash-map', 'if', 'len', 'mod', 'not', 'nth', 'or',
        'pop-front', 'print', 'println', 'quote', 'read-and-eval-path', 'read-and-eval-string',
        'return', 'select', 'starts-with?', 'string-format', 'to-integer', 'to-string', 'uid',
        'vector', 'while', 'io-file-open', 'port-read', 'port-write', 'port-seek', 'port-flush',
        'port-close', 'port-dist', 'port-length', 'input-file-open', 'output-file-open',
        'port-supports-output?', 'port-supports-input?', 'require'
    )

    valid_name = r'[\w!$%&*+-/\:;,.<=>?@^~]+'

    tokens = {
            'root' : [
                (r';.*?$', Comment.Singleline),
                (r'\s+',  Text), #Whitespace
                (r'(true|false|nil)',  Name.Constant),
                (r'-?\d+\.\d+', Number.Float),
                (r'-?\d+', Number.Integer),
                ("(?<=\()(%s)" % '|'.join(re.escape(entry) + ' ' for entry in builtins), Name.Builtin),
                (r'(?<=\()' + valid_name, Name.Function),
                (valid_name, Name.Variable),
                (r'(\(|\))', Punctuation), #Parentheses
                ('\[', String, 'string'),
                ('\#\!', Comment.Multiline, 'comment_multiline'),
                (r"'" + valid_name, String.Symbol),
                (r"('|\.)", Operator),
                (r'(\|.?\||\|bar\||\|newline\||\|tab\|)', String.Char),
                ],
                'string' : [
                    ('[^\]]+', String),
                    ('\]', String, '#pop'),
                    ],
                'comment_multiline' : [
                    ('[^\!\#]+', Comment.Multiline),
                    ('\!\#', Comment.Multiline, '#pop'),
                    ]
            }
