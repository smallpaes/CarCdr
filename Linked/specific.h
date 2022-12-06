#pragma once

// struct cons
// {
//   struct cons *car;
//   struct cons *cdr;
//   atomtype leaf;
// };

// struct lisp
// {
//   struct cons *p;
//   int size;
// };

struct lisp
{
  struct lisp *car;
  struct lisp *cdr;
  atomtype val;
};
