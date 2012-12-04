#!/bin/sh
# These are a few minimal integration level regression tests.
# Everything 'cept check_eq() should be pretty self-explanitory.
# Look up 'indirect references' if you want to know what that hacky-ass
# eval nonesense is about.

set -e

optbot=bin/optbot

check_eq() {
  eval val=\$$1

  if ! [ "$val" = "$2" ]; then
    echo >&2 "$1 set to: $val not: $2"
    exit 1
  fi
}

test_initial() {
  eval `$optbot -aa,opt_a,"blah",false -- $@`
  check_eq arg_opt_a true
}
test_initial -a

test_multiple() {
  eval `$optbot -aa,opt_a,"blah",false -ab,opt_b,"blah",false -- $@`
  check_eq arg_opt_b true
  check_eq arg_opt_a true
}
test_multiple -a -b

test_values() {
  eval `$optbot -aa,opt_a,"blah",true -- $@`
  check_eq arg_opt_a_n 2
  check_eq arg_opt_a one
  check_eq arg_opt_a_1 one
  check_eq arg_opt_a_2 two
}
test_values -a one -a two

test_help() {
  $optbot -aa,opt_a,"does great things",true -- $@ 2>&1 |
    if ! grep -q "does great things"; then
      echo 'Help output seems broken'
      exit 1
    fi
}
test_help -h
