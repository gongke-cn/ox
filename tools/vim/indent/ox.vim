" Vim indent file
" Language:	Lua script
" Maintainer:	Marcus Aurelius Farias <marcus.cf 'at' bol.com.br>
" First Author:	Max Ischenko <mfi 'at' ukr.net>
" Last Change:	2017 Jun 13
"		2022 Sep 07: b:undo_indent added by Doug Kearns

" Only load this indent file when no other was loaded.
if exists("b:did_indent")
  finish
endif
let b:did_indent = 1

setlocal indentexpr=GetOXIndent()

setlocal indentkeys+=0]

setlocal autoindent

let b:undo_indent = "setlocal autoindent< indentexpr< indentkeys<"

" Only define the function once.
if exists("*GetOXIndent")
  finish
endif

function! GetOXIndent()
  let prevlnum = prevnonblank(v:lnum - 1)
  if prevlnum == 0
    return 0
  endif

  let ind = indent(prevlnum)
  let prevline = getline(prevlnum)
  let thisline = getline(v:lnum)

  if prevline =~ '^.*{\s*$' && thisline !~ '^\s*}'
    return ind + shiftwidth()
  endif

  if prevline =~ '^.*[\s*$' && thisline !~ "^\s*]"
    return ind + shiftwidth()
  endif

  if thisline =~ '^\s*}' && prevline !~ '^.*{\s*$'
    return ind - shiftwidth()
  endif

  if thisline =~ '^\s*]' && prevline !~ '^.*[\s*$'
    return ind - shiftwidth()
  endif

  return ind
endfunction

