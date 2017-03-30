" Vim syntax file for Octaspire Dern programming language

if exists("b:current_syntax")
    finish
endif

setlocal iskeyword=33-39,42-90,92,94-126

syn match dernChar "\v\|.\|"
syn match dernChar "\v\|newline\|"
syn match dernChar "\v\|tab\|"
syn match dernChar "\v\|bar\|"
hi link dernChar Character

syn region dernString contains=dernEscape start=/\v\[/ skip=/\v\|\]\|/ end=/\v\]/
syn match dernEscape "\v\|.\|" contained
syn match dernEscape "\v\|newline\|" contained
syn match dernEscape "\v\|tab\|" contained
syn match dernEscape "\v\|bar\|" contained
syn match dernEscape "\v\{\}" contained
hi link dernString String

syn keyword dernKeyword println env-new env-current env-global ++ -- * - find hash-map exit mutable doc len read-and-eval-path read-and-eval-string define quote if while = == <= fn '
hi link dernKeyword Keyword

syn keyword dernBoolean true false nil
hi link dernBoolean Boolean

syn match dernReal "\v\d+\.\d+"
hi link dernReal Float

syn match dernInt "\v\d+"
hi link dernInt Number

syn match dernVarArg "\v \.\.\."
hi link dernVarArg Delimiter

syn match dernSexpr "\v\("
syn match dernSexpr "\v\)"
hi link dernSexpr Delimiter

syn match dernComment "\v;.*\n"
hi link dernComment Comment

let b:current_syntax = "dern"

