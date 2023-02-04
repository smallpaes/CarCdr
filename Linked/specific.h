#pragma once

#define LISPIMPL "Linked"

struct lisp
{
  struct lisp *car;
  struct lisp *cdr;
  atomtype val;
};
