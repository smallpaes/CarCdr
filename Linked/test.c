#include "../lisp.h"
#include "specific.h"

#define LEFT_PAREN '('
#define RIGHT_PAREN ')'
#define BOARD ' '
#define LISTSTRLEN 1000
#define ERROR(MSG)          \
  {                         \
    fprintf(stderr, "Fatal Error occurred: %s \
    in function %s\n",      \
            MSG, __func__); \
    exit(EXIT_FAILURE);     \
  }

lisp *_lisp_copy_cdr(lisp *l_c);
char *find_substring(char *str, int *end_idx);
void test(void);

// Returns element 'a' - this is not a list, and
// by itelf would be printed as e.g. "3", and not "(3)"
lisp *lisp_atom(const atomtype a)
{
  lisp *l = (lisp *)ncalloc(1, sizeof(lisp));
  l->car = NULL;
  l->cdr = NULL;
  l->val = a;
  return l;
}

// Returns a list containing the car as 'l1'
// and the cdr as 'l2'- data in 'l1' and 'l2' are reused,
// and not copied. Either 'l1' and/or 'l2' can be NULL
lisp *lisp_cons(const lisp *l1, const lisp *l2)
{
  lisp *l = (lisp *)ncalloc(1, sizeof(lisp));
  l->car = (lisp *)l1;
  l->cdr = (lisp *)l2;
  l->val = 0;
  return l;
}

// Returns the car (1st) component of the list 'l'.
// Does not copy any data.
lisp *lisp_car(const lisp *l)
{
  if (!l)
  {
    return NULL;
  }
  return l->car;
}

// Returns the cdr (all but the 1st) component of the list 'l'.
// Does not copy any data.
lisp *lisp_cdr(const lisp *l)
{
  if (!l)
  {
    return NULL;
  }
  return l->cdr;
}

// Returns the data/value stored in the cons 'l'
atomtype lisp_getval(const lisp *l)
{
  if (!l)
  {
    ERROR("Invalid arg")
  }
  return l->val;
}

// Returns a deep copy of the list 'l'
lisp *lisp_copy(const lisp *l)
{
  if (!l)
  {
    return NULL;
  }
  lisp *h = (lisp *)l;

  if (lisp_isatomic(h))
  {
    return lisp_atom(lisp_getval(h));
  }

  lisp *next_cdr = h->cdr ? lisp_copy(h->cdr) : NULL;
  return lisp_cons(lisp_copy(h->car), next_cdr);
}

// Returns number of components in the list.
int lisp_length(const lisp *l)
{
  if (!l || lisp_isatomic(l))
  {
    return 0;
  }
  lisp *h = (lisp *)l;
  int cnt = 0;
  while (h)
  {
    cnt++;
    h = h->cdr;
  }
  return cnt;
}

// Returns stringified version of list
void lisp_tostring(const lisp *l, char *str)
{
  if (!str)
  {
    ERROR("Missing output string")
  }
  lisp *h = (lisp *)l;
  if (lisp_isatomic(h))
  {
    snprintf(str, LISTSTRLEN, "%i", lisp_getval(h));
    return;
  }

  // config the string
  snprintf(str, LISTSTRLEN, "%c", LEFT_PAREN);

  while (h)
  {
    if (lisp_isatomic(h->car))
    {
      int val = lisp_getval(h->car);
      snprintf(str, LISTSTRLEN, "%s%i%c", str, val, BOARD);
    }
    else
    {
      char car_str[LISTSTRLEN];
      lisp_tostring(h->car, car_str);
      snprintf(str, LISTSTRLEN, "%s%s%c", str, car_str, BOARD);
    }
    h = h->cdr;
  }

  // replace the space with the right parenthesis
  int lng = strlen(str);
  bool is_empty_list = str[lng - 1] == LEFT_PAREN;
  char close[2];
  snprintf(close, 2, "%c", RIGHT_PAREN);
  void *dest = is_empty_list ? str + lng : str + lng - 1;
  memcpy(dest, &close, 2);
}

bool lisp_isatomic(const lisp *l)
{
  if (!l)
  {
    return false;
  }
  return !l->car && !l->cdr;
}

// Clears up all space used
// Double pointer allows function to set 'l' to NULL on success
void lisp_free(lisp **l)
{
  if (*l == NULL)
  {
    return;
  }

  lisp *h = (lisp *)*l;

  if (lisp_isatomic(h))
  {
    free(h);
  }
  else
  {
    lisp_free(&(h->car));
    lisp_free(&(h->cdr));
  }
  *l = NULL;
}
/* ------------- Tougher Ones : Extensions ---------------*/

// Builds a new list based on the string 'str'
lisp *lisp_fromstring(const char *str)
{
  if (!str)
  {
    return NULL;
  }

  char cur_str[LISTSTRLEN];
  strcpy(cur_str, str);
  int str_lng = strlen(str);

  printf("In str: %s \n", str);

  if (str[0] == LEFT_PAREN)
  {
    int end_idx;
    char *sub_str;
    sub_str = find_substring(cur_str, &end_idx);
    bool is_no_split = end_idx + 1 == str_lng;

    if (strlen(sub_str) == 0)
    {
      return NULL;
    }

    if (is_no_split)
    {
      printf("sub str if: %s \n", sub_str);
      return lisp_fromstring(sub_str);
    }
    else
    {
      char rest_str[LISTSTRLEN];
      strncpy(rest_str, cur_str + end_idx + 2, LISTSTRLEN);
      printf("sub str else: %s \n", sub_str);
      printf("rest str: %s \n", rest_str);
      return lisp_cons(lisp_fromstring(sub_str), lisp_fromstring(rest_str));
    }

    return lisp_fromstring(sub_str);
  }
  else if (str[0] == BOARD)
  {
    int end_idx;
    char *sub_str;
    sub_str = find_substring(cur_str, &end_idx);

    if (str[1] == LEFT_PAREN)
    {
      char rest_str[LISTSTRLEN];
      strncpy(rest_str, cur_str + end_idx + 2, LISTSTRLEN);
      printf("sub str else if : %s \n", sub_str);
      printf("rest str: %s \n", rest_str);

      bool is_end = end_idx + 1 >= str_lng;
      printf("end idx: %i, str len: %i \n", end_idx, str_lng);
      lisp *next = is_end ? NULL : lisp_fromstring(rest_str);

      return lisp_cons(lisp_fromstring(sub_str), next);
    }
    else
    {
      char rest_str[LISTSTRLEN];
      strncpy(rest_str, cur_str + 1, LISTSTRLEN);
      printf("else if rest: %s \n", rest_str);
      return lisp_fromstring(rest_str);
    }
  }

  int end = strcspn(str, " ");
  printf("end: %i, str lng: %i \n", end, str_lng);
  bool is_end = end >= str_lng;
  int val;
  sscanf(str, "%d", &val);
  // printf("Rsu: %i\n", val);

  char rest_str[LISTSTRLEN];
  strncpy(rest_str, str + end, LISTSTRLEN);
  printf("atom rest str: %s \n", rest_str);
  printf("is end: %i\n", is_end);
  lisp *next = is_end ? NULL : lisp_fromstring(rest_str);

  return lisp_cons(lisp_atom(val), next);
}

char *find_substring(char *str, int *end_idx)
{
  int lng = strlen(str);
  char *sub_string = calloc(lng + 1, sizeof(char));
  char *stack = calloc(lng + 1, sizeof(char));
  int stack_num = 0;
  int open_idx;
  for (int i = 0; i < lng; i++)
  {
    if (str[i] == LEFT_PAREN)
    {
      stack[stack_num] = LEFT_PAREN;
      if (stack_num == 0)
      {
        open_idx = i;
      }
      stack_num++;
    }
    else if (str[i] == RIGHT_PAREN)
    {
      if (stack_num > 1)
      {
        stack_num--;
      }
      else
      {
        *end_idx = i;
        strncpy(sub_string, str + open_idx + 1, *end_idx - open_idx - 1);
        free(stack);
        return sub_string;
      }
    }
  }
  free(stack);
  // TODO: handle unmatch open and closing parenthesis nums
  return NULL;
}

// // Returns a new list from a set of existing lists.
// // A variable number 'n' lists are used.
// // Data in existing lists are reused, and not copied.
// // You need to understand 'varargs' for this.
// lisp *lisp_list(const int n, ...);

// // Allow a user defined function 'func' to be applied to
// // each component of the list 'l'.
// // The user-defined 'func' is passed a pointer to a cons,
// // and will maintain an accumulator of the result.
// atomtype lisp_reduce(atomtype (*func)(lisp *l), lisp *l);

void test(void)
{
  char inp[LISTSTRLEN] = {"(1 (2 3)) 4 (5 2)"};
  int end;
  char *sub = find_substring(inp, &end);

  assert(strcmp(sub, "1 (2 3)") == 0);
  assert(end == 8);
}
