#include "../lisp.h"
#include "specific.h"
#include <ctype.h>

#define LIST_BGN '('
#define LIST_END ')'
#define SEP ' '
#define LISTSTRLEN 1000

void _get_list_str(char *str, char *dest_str, int *end_idx);
lisp *_handle_num_bgn_str(const char *str);
lisp *_handle_nonnum_bgn_str(char *str);
void _close_string(char *str);
bool _format_string(char *str);
bool _is_num_or_sign(const char c);
bool _is_valid_char(const char c);
void _hdl_snpf_rtn(int i);
void test(void);

lisp *lisp_atom(const atomtype a)
{
  lisp *l = (lisp *)ncalloc(1, sizeof(lisp));
  l->car = NULL;
  l->cdr = NULL;
  l->val = a;
  return l;
}

lisp *lisp_cons(const lisp *l1, const lisp *l2)
{
  lisp *l = (lisp *)ncalloc(1, sizeof(lisp));
  l->car = (lisp *)l1;
  l->cdr = (lisp *)l2;
  l->val = 0;
  return l;
}

lisp *lisp_car(const lisp *l)
{
  if (!l)
  {
    return NULL;
  }
  return l->car;
}

lisp *lisp_cdr(const lisp *l)
{
  if (!l)
  {
    return NULL;
  }
  return l->cdr;
}

atomtype lisp_getval(const lisp *l)
{
  if (!l)
  {
    return 0;
  }
  return l->val;
}

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

void lisp_tostring(const lisp *l, char *str)
{
  if (!str)
  {
    return;
  }
  lisp *h = (lisp *)l;
  int r_v;
  if (lisp_isatomic(h))
  {
    r_v = snprintf(str, LISTSTRLEN, "%i", lisp_getval(h));
    _hdl_snpf_rtn(r_v);
    return;
  }
  _hdl_snpf_rtn(snprintf(str, LISTSTRLEN, "%c", LIST_BGN));
  while (h)
  {
    char temp[LISTSTRLEN];
    if (lisp_isatomic(h->car))
    {
      int val = lisp_getval(h->car);
      r_v = snprintf(temp, LISTSTRLEN, "%s%i%c", str, val, SEP);
    }
    else
    {
      char car_str[LISTSTRLEN];
      lisp_tostring(h->car, car_str);
      r_v = snprintf(temp, LISTSTRLEN, "%s%s%c", str, car_str, SEP);
    }
    _hdl_snpf_rtn(r_v);
    strcpy(str, temp);
    h = h->cdr;
  }
  _close_string(str);
}

void _hdl_snpf_rtn(int i)
{
  if (i <= 0)
  {
    on_error("Failed writing value");
  }
}

// Pads or replaces ending space with closing parenthesis
void _close_string(char *str)
{
  if (!str)
  {
    return;
  }
  int lng = strlen(str);
  char last_c = str[lng - 1];
  if (last_c != LIST_BGN && last_c != SEP)
  {
    on_error("Invalid string arg");
  }
  bool is_empty_list = last_c == LIST_BGN;
  char close_str[2];
  _hdl_snpf_rtn(snprintf(close_str, 2, "%c", LIST_END));
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
    free(h);
  }
  *l = NULL;
}

lisp *lisp_fromstring(const char *str)
{
  if (!str)
  {
    return NULL;
  }
  char cur_str[LISTSTRLEN];
  strcpy(cur_str, str);
  if (!_format_string(cur_str))
  {
    return NULL;
  }
  char cur_c = str[0];
  bool is_digit = _is_num_or_sign(cur_c);
  if (is_digit)
  {
    return _handle_num_bgn_str(cur_str);
  }
  return _handle_nonnum_bgn_str(cur_str);
}

lisp *_handle_nonnum_bgn_str(char *str)
{
  if (!str || _is_num_or_sign(str[0]))
  {
    return NULL;
  }
  char list_str[LISTSTRLEN] = "";
  int end_idx;
  int str_lng = strlen(str);
  char cur_c = str[0];
  char next_c = str[1];
  _get_list_str(str, list_str, &end_idx);
  bool is_sole_list = end_idx + 1 == str_lng;
  bool is_c_sep = cur_c == SEP;
  bool is_c_list_bgn = cur_c == LIST_BGN;
  bool is_n_list_bgn = next_c == LIST_BGN;
  bool is_empty_list = strlen(list_str) == 0;
  bool is_sub_list = is_c_list_bgn && !is_sole_list;
  bool has_next_list = is_c_sep && is_n_list_bgn;
  if (is_c_list_bgn && is_empty_list)
  {
    return NULL;
  }
  if (is_c_list_bgn && is_sole_list)
  {
    return lisp_fromstring(list_str);
  }
  if (is_c_sep && !is_n_list_bgn)
  {
    char *next_num = str + 1;
    return lisp_fromstring(next_num);
  }
  if (is_sub_list || has_next_list)
  {
    char *next_l = str + end_idx + 1;
    lisp *cdr = is_sole_list ? NULL : lisp_fromstring(next_l);
    return lisp_cons(lisp_fromstring(list_str), cdr);
  }
  return NULL;
}

lisp *_handle_num_bgn_str(const char *str)
{
  if (!str || !_is_num_or_sign(str[0]))
  {
    return NULL;
  }
  int str_lng = strlen(str);
  int next_sep = strcspn(str, " ");
  bool is_end = next_sep == str_lng;
  int val;
  if (sscanf(str, "%d", &val) != 1)
  {
    on_error("Failed filling value");
  }
  char *rest_str = (char *)str + next_sep;
  lisp *next = is_end ? NULL : lisp_fromstring(rest_str);
  return lisp_cons(lisp_atom(val), next);
}

bool _is_num_or_sign(const char c)
{
  return (c == '-' || isdigit(c));
}

bool _is_valid_char(const char c)
{
  if (_is_num_or_sign(c))
  {
    return true;
  }
  switch (c)
  {
  case LIST_BGN:
  case LIST_END:
  case SEP:
    return true;
  default:
    return false;
  }
}

bool _format_string(char *str)
{
  int lng = strlen(str);
  // exclude the outer parenthesis
  for (int i = 1; i < lng - 1; i++)
  {
    if (!_is_valid_char(str[i]))
    {
      return false;
    }
    char prev_char = str[i - 1];
    bool is_c_start = str[i] == LIST_BGN;
    bool is_p_SEP = prev_char == SEP;
    bool is_p_start = prev_char == LIST_BGN;
    bool is_p_digit = !is_p_SEP && !is_p_start;
    bool should_pad_space = is_c_start && is_p_digit;
    if (should_pad_space)
    {
      char *cur_address = str + i;
      int copy_lng = lng - i + 1;
      memmove(cur_address + 1, cur_address, copy_lng);
      memset(cur_address, SEP, sizeof(char));
      lng = strlen(str);
    }
  }
  return true;
}

// Locates the first list from the input string
// Retrieves the components within the list
// Updates the ending index to where the list ends
void _get_list_str(char *str, char *dest_str, int *end_idx)
{
  if (!str || !dest_str || !end_idx)
  {
    on_error("Missing one or more input args");
  }
  int ori_str_lng = strlen(str);
  char *l_str = ncalloc(ori_str_lng + 1, sizeof(char));
  int open_idx, left_paren_num = 0;
  for (int i = 0; i < ori_str_lng; i++)
  {
    if (str[i] == LIST_BGN)
    {
      open_idx = left_paren_num == 0 ? i : open_idx;
      left_paren_num++;
    }
    else if (str[i] == LIST_END)
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

lisp *lisp_list(const int n, ...)
{
  if (n == 0)
  {
    return NULL;
  }
  va_list valist;
  va_start(valist, n);
  lisp *head = NULL;
  lisp *cur = NULL;
  for (int i = n; i > 0; i--)
  {
    lisp *n = va_arg(valist, lisp *);
    lisp *l = lisp_cons(n, NULL);
    if (!head)
    {
      cur = head = l;
    }
    else
    {
      cur->cdr = l;
      cur = cur->cdr;
    }
  }
  va_end(valist);
  return head;
}

void lisp_reduce(void (*func)(lisp *l, atomtype *n), lisp *l, atomtype *acc)
{
  if (!l || !func || !acc)
  {
    return;
  }
  lisp_reduce(func, l->car, acc);
  lisp_reduce(func, l->cdr, acc);
  if (lisp_isatomic(l))
  {
    func(l, acc);
  }
}

void test(void)
{
  char str[LISTSTRLEN];

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

  char f_str1[LISTSTRLEN] = "(1(2(3(4 5))))";
  char f_str2[LISTSTRLEN] = "(1 (2 (3 (4 5))))";
  char f_str3[LISTSTRLEN] = "()";

  char n_str1[LISTSTRLEN] = "1";
  char n_str2[LISTSTRLEN] = "1 2";
  char n_str3[LISTSTRLEN] = "1 (2 3) 4";
  char n_str4[LISTSTRLEN] = " 1 2";
  char *n_str5 = NULL;

  char non_n_str1[LISTSTRLEN] = "(1)";
  char non_n_str2[LISTSTRLEN] = "(1 (2 3) 4)";
  char non_n_str3[LISTSTRLEN] = "1 2";
  char *non_n_str4 = NULL;
  char non_n_str5[LISTSTRLEN] = " 1 2";
  char non_n_str6[LISTSTRLEN] = "(";
  char non_n_str7[LISTSTRLEN] = " (1 2)";

  char invalid_str1[LISTSTRLEN] = "snlsnld";
  char invalid_str2[LISTSTRLEN] = "(1 (2 3) 4 e)";

  _close_string(open_str1);
  assert(strcmp(open_str1, "()") == 0);
  _close_string(open_str2);
  assert(strcmp(open_str2, "(1 (2 3))") == 0);
  _close_string(open_str3);
  assert(open_str3 == NULL);

  _get_list_str(sub_list_str1, dest_str, &end_idx);
  assert(strcmp(dest_str, "1 2 3") == 0);
  assert(end_idx == 6);
  _get_list_str(sub_list_str2, dest_str, &end_idx);
  assert(strcmp(dest_str, "2 3") == 0);
  assert(end_idx == 6);
  _get_list_str(sub_list_str3, dest_str, &end_idx);
  assert(strcmp(dest_str, "2 3") == 0);
  assert(end_idx == 7);
  _get_list_str(sub_list_str4, dest_str, &end_idx);
  assert(strcmp(dest_str, "") == 0);
  assert(end_idx == 1);
  _get_list_str(sub_list_str5, dest_str, &end_idx);
  assert(strcmp(dest_str, "") == 0);
  // should remain the previous index
  assert(end_idx == 1);

  assert(_format_string(f_str1) == true);
  assert(strcmp(f_str1, "(1 (2 (3 (4 5))))") == 0);
  assert(_format_string(f_str2) == true);
  assert(strcmp(f_str2, "(1 (2 (3 (4 5))))") == 0);
  assert(_format_string(f_str3) == true);
  assert(strcmp(f_str3, "()") == 0);

  lisp *l1 = _handle_num_bgn_str(n_str1);
  assert(lisp_getval(lisp_car(l1)) == 1);
  assert(lisp_cdr(l1) == NULL);
  assert(lisp_length(l1) == 1);
  assert(lisp_length(lisp_car(l1)) == 0);
  lisp_free(&l1);
  lisp *l2 = _handle_num_bgn_str(n_str2);
  assert(lisp_getval(lisp_car(l2)) == 1);
  assert(lisp_cdr(l2) != NULL);
  assert(lisp_getval(lisp_car(lisp_cdr(l2))) == 2);
  assert(lisp_length(l2) == 2);
  lisp_free(&l2);
  lisp *l3 = _handle_num_bgn_str(n_str3);
  assert(lisp_getval(lisp_car(l3)) == 1);
  assert(lisp_length(l3) == 3);
  lisp_tostring(l3, str);
  assert(strcmp(str, "(1 (2 3) 4)") == 0);
  lisp_free(&l3);
  lisp *l4 = _handle_num_bgn_str(n_str4);
  lisp *l5 = _handle_num_bgn_str(n_str5);
  assert(l4 == NULL);
  assert(l5 == NULL);

  lisp *l6 = _handle_nonnum_bgn_str(non_n_str1);
  assert(lisp_getval(lisp_car(l6)) == 1);
  assert(lisp_cdr(l6) == NULL);
  assert(lisp_length(l6) == 1);
  assert(lisp_length(lisp_car(l6)) == 0);
  lisp_free(&l6);
  lisp *l7 = _handle_nonnum_bgn_str(non_n_str2);
  assert(lisp_getval(lisp_car(l7)) == 1);
  assert(lisp_length(l7) == 3);
  lisp_tostring(l7, str);
  assert(strcmp(str, "(1 (2 3) 4)") == 0);
  lisp_free(&l7);
  lisp *l8 = _handle_nonnum_bgn_str(non_n_str3);
  lisp *l9 = _handle_nonnum_bgn_str(non_n_str4);
  assert(l8 == NULL);
  assert(l9 == NULL);
  lisp *l10 = _handle_nonnum_bgn_str(non_n_str5);
  assert(lisp_getval(lisp_car(l10)) == 1);
  assert(lisp_cdr(l10) != NULL);
  assert(lisp_getval(lisp_car(lisp_cdr(l10))) == 2);
  assert(lisp_length(l10) == 2);
  lisp_free(&l10);
  lisp *l11 = _handle_nonnum_bgn_str(non_n_str6);
  assert(l11 == NULL);
  lisp *l12 = _handle_nonnum_bgn_str(non_n_str7);
  lisp_tostring(l12, str);
  assert(strcmp(str, "((1 2))") == 0);
  assert(lisp_length(l12) == 1);
  lisp_free(&l12);

  assert(_is_num_or_sign('-') == true);
  assert(_is_num_or_sign('1') == true);
  assert(_is_num_or_sign('9') == true);
  assert(_is_num_or_sign(' ') == false);
  assert(_is_num_or_sign('c') == false);
  assert(_is_num_or_sign('J') == false);

  assert(_is_valid_char('-') == true);
  assert(_is_valid_char(' ') == true);
  assert(_is_valid_char('(') == true);
  assert(_is_valid_char(')') == true);
  assert(_is_valid_char('c') == false);
  assert(_is_valid_char('J') == false);

  lisp *l13 = lisp_fromstring(invalid_str1);
  assert(l13 == NULL);
  lisp *l14 = lisp_fromstring(invalid_str2);
  assert(l14 == NULL);
}
