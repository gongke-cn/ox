" OX syntax file
" Language:	OX
" Maintainer:	Gong Ke <3188045634@qq.com>

if !exists("main_syntax")
  " quit when a syntax file was already loaded
  if exists("b:current_syntax")
    finish
  endif
  let main_syntax = 'ox'
elseif exists("b:current_syntax") && b:current_syntax == "ox"
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn match   oxLineComment       "^#!.*"
syn match   oxLineComment       "\/\/.*"
syn region  oxComment           start="/\*"  end="\*/"

syn match   oxCharacter         "'\(\\x[0-9a-fA-F]\{2}\|\\u[0-9a-fA-F]\{4}\|\\u{[0-9a-fA-F]\+}\|\\.\|.\)'"

syn region  oxStringD           start=+"+    skip=+\\\\\|\\"+  end=+"+  contains=oxEmbedD
syn region  oxStringS           start=+''+   end=+''+ contains=oxEmbedS

syn region  oxEmbedD            start=+\(\\\)\@<!{+    end=+}+  contained contains=@oxEmbededExpr
syn region  oxEmbedS            start=+{{+   end=+}}+ contained contains=@oxEmbededExpr

syn region  oxRegExp            start=+/\([/*]\)\@!+    skip=+\\\\\|\\/+ end=+/[a-z]*+ oneline

syn match   oxNumber            "\<0[bB][0-1_]\+\>"
syn match   oxNumber            "\<0[oO][0-7_]\+\>"
syn match   oxNumber            "\<0[xX][0-9a-fA-F_]\+\>"
syn match   oxNumber            "\<\d[0-9_]*\(\.[0-9_]\+\)\?\([eE][+-]\?\d\+\)\?\>"

syn match   oxIdentifier        "[#@]\?[$a-zA-Z_][_$a-zA-Z0-9]*"

syn keyword oxConditional       if elif else case
syn keyword oxRepeat            do while for as
syn keyword oxOperator          typeof global owned instof
syn keyword oxBranch            break continue
syn keyword oxException         try catch finally throw
syn keyword oxBoolean           true false
syn keyword oxSpecial           null this argv
syn keyword oxFunction          func
syn keyword oxReserved          class static enum bitfield public ref return yield

syn cluster oxEmbededExpr       contains=oxLineComment,oxComment,oxStringD,oxStringS,oxNumber,oxConditional,oxOperator,oxBoolean,oxFunction,oxReserved,oxSpecial,oxIdentifier

if main_syntax == "ox"
  syn sync fromstart
  syn sync maxlines=100

  syn sync ccomment oxComment
endif

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

hi def link oxLineComment       Comment
hi def link oxComment           Comment
hi def link oxNumber            Number
hi def link oxCharacter         Character
hi def link oxStringD           String
hi def link oxStringS           String
hi def link oxRegExp            String
hi def link oxConditional       Conditional
hi def link oxRepeat            Repeat
hi def link oxOperator          Operator
hi def link oxBranch            Conditional
hi def link oxException         Exception
hi def link oxBoolean           Boolean
hi def link oxSpecial           Special
hi def link oxFunction          Function
hi def link oxReserved          Keyword
hi def link oxIdentifier        Identifier

let b:current_syntax = "ox"
if main_syntax == 'ox'
  unlet main_syntax
endif
let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=4
