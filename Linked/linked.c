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
void get_list_string(char *str, char *dest_str, int *end_idx);
void close_string(char *str);
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
  close_string(str);
}

// Pads or replaces ending space with closing parenthesis
void close_string(char *str)
{
  if (!str)
  {
    return;
  }
  int lng = strlen(str);
  char last_c = str[lng - 1];
  if (last_c != LEFT_PAREN && last_c != BOARD)
  {
    ERROR("Invalid string arg");
  }
  bool is_empty_list = last_c == LEFT_PAREN;
  char close_str[2];
  snprintf(close_str, 2, "%c", RIGHT_PAREN);
  char *dest = is_empty_list ? str + lng : str + lng - 1;
  memcpy(dest, &close_str, 2);
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
    char sub_str[LISTSTRLEN];
    get_list_string(cur_str, sub_str, &end_idx);

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
      strncpy(rest_str, cur_str + end_idx + 1, LISTSTRLEN);
      printf("sub str else: %s \n", sub_str);
      printf("rest str: %s \n", rest_str);
      return lisp_cons(lisp_fromstring(sub_str), lisp_fromstring(rest_str));
    }

    return lisp_fromstring(sub_str);
  }
  else if (str[0] == BOARD)
  {
    int end_idx;
    char sub_str[LISTSTRLEN];
    get_list_string(cur_str, sub_str, &end_idx);

    if (str[1] == LEFT_PAREN)
    {
      char rest_str[LISTSTRLEN];
      strncpy(rest_str, cur_str + end_idx + 1, LISTSTRLEN);
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

  char rest_str[LISTSTRLEN];
  strncpy(rest_str, str + end, LISTSTRLEN);
  printf("atom rest str: %s \n", rest_str);
  printf("is end: %i\n", is_end);
  lisp *next = is_end ? NULL : lisp_fromstring(rest_str);

  return lisp_cons(lisp_atom(val), next);
}

// Locates the first list from the input string
// Retrieves the components within the list
// Updates the ending index to where the list ends
// TODO: Add assertions
void get_list_string(char *str, char *dest_str, int *end_idx)
{
  if (!str || !dest_str || !end_idx)
  {
    ERROR("Missing one or more input args")
  }
  int lng = strlen(str);
  char *l_str = ncalloc(lng + 1, sizeof(char));
  int open_idx, left_paren_num = 0;
  for (int i = 0; i < lng; i++)
  {
    if (str[i] == LEFT_PAREN)
    {
      open_idx = left_paren_num == 0 ? i : open_idx;
      left_paren_num++;
    }
    else if (str[i] == RIGHT_PAREN)
    {
      if (left_paren_num == 1)
      {
        *end_idx = i;
        strncpy(l_str, str + open_idx + 1, *end_idx - open_idx - 1);
        strcpy(dest_str, l_str);
        free(l_str);
        return;
      }
      left_paren_num--;
    }
  }
  free(l_str);
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

  char open_str1[LISTSTRLEN] = {"("};
  char open_str2[LISTSTRLEN] = {"(1 (2 3) "};
  char *open_str3 = NULL;

  char sub_list_str1[LISTSTRLEN] = {"(1 2 3)"};
  char sub_list_str2[LISTSTRLEN] = {"2 (2 3)"};
  char sub_list_str3[LISTSTRLEN] = {" 2 (2 3)"};
  char sub_list_str4[LISTSTRLEN] = {"()"};
  char sub_list_str5[LISTSTRLEN] = {"1 2 3"};
  char dest_str[LISTSTRLEN];
  int end_idx = 0;

  close_string(open_str1);
  assert(strcmp(open_str1, "()") == 0);
  close_string(open_str2);
  assert(strcmp(open_str2, "(1 (2 3))") == 0);
  close_string(open_str3);
  assert(open_str3 == NULL);

  get_list_string(sub_list_str1, dest_str, &end_idx);
  assert(strcmp(dest_str, "1 2 3") == 0);
  assert(end_idx == 6);
  get_list_string(sub_list_str2, dest_str, &end_idx);
  assert(strcmp(dest_str, "2 3") == 0);
  assert(end_idx == 6);
  get_list_string(sub_list_str3, dest_str, &end_idx);
  assert(strcmp(dest_str, "2 3") == 0);
  assert(end_idx == 7);
  get_list_string(sub_list_str4, dest_str, &end_idx);
  assert(strcmp(dest_str, "") == 0);
  assert(end_idx == 1);
  get_list_string(sub_list_str5, dest_str, &end_idx);
  assert(strcmp(dest_str, "") == 0);
  // remain the previous index
  assert(end_idx == 1);
}
