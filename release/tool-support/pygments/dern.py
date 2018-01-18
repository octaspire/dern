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

    builtins = [
        '!=', '*', '+', '++', '+=', '-', '--', '-=', '-==', '/', '<', '<=', '=', '==', '===', '>', '>=', 'abort',
        'and', 'define', 'do', 'doc', 'env-current', 'env-global', 'env-new', 'eval', 'exit',
        'find', 'fn', 'for', 'hash-map', 'if', 'len', 'mod', 'not', 'ln@', 'cp@', 'or',
        'pop-front', 'print', 'println', 'quote', 'read-and-eval-path', 'read-and-eval-string',
        'return', 'select', 'starts-with?', 'string-format', 'to-integer', 'to-string', 'uid',
        'vector', 'while', 'io-file-open', 'port-read', 'port-write', 'port-seek', 'port-flush',
        'port-close', 'port-dist', 'port-length', 'input-file-open', 'output-file-open',
        'port-supports-output?', 'port-supports-input?', 'require', 'queue', 'queue-with-max-length',
        'list', 'howto', 'howto-ok', 'howto-no', 'as', 'in', 'pop-back'

    ]

    builtins.sort(reverse=True)

    valid_name = r'[\w!$%&*+-/\:;,.<=>?@^~]+'

    tokens = {
            'root' : [
                (r';.*?$', Comment.Singleline),
                (r'\s+',  Text), #Whitespace
                (r'(true|false|nil)',  Name.Constant),
                (r'-?\d+\.\d+', Number.Float),
                (r'-?\d+', Number.Integer),
                (r'(?<=\()(%s)' % "|".join(re.escape(entry) + '[\s\)]+' for entry in builtins), Name.Builtin),
                (r'(?<=\(fn \()' + valid_name, Name.Variable),
                (r'(?<=[^\']\()' + valid_name, Name.Function),
                (valid_name, Name.Variable),
                (r'(\(|\))', Punctuation), #Parentheses
                ('\[', String, 'string'),
                ('\#\!', Comment.Multiline, 'comment_multiline'),
                (r"'" + valid_name, String.Symbol),
                (r"('|\.)", Operator),
                (r'(\|.?\||\|bar\||\|newline\||\|tab\||\|string-start\||\|string-end\||\|[0-9a-fA-F]+\|)', String.Char),
                ],
                'string' : [
                    (r'\]', String, '#pop'),
                    (r"(\|bar\||\|newline\||\|tab\||\|string-start\||\|string-end\||\|[0-9a-fA-F]+\|)", String.Escape),
                    (r"(\{\})", String.Escape),
                    (r'[^\]\|\{\}]+', String),
                    ],
                'comment_multiline' : [
                    ('[^\!\#]+', Comment.Multiline),
                    ('\!\#', Comment.Multiline, '#pop'),
                    ]
            }
