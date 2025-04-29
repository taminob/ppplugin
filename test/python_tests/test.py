string_global = "abc"
int_global = 12
float_global = 42.0
bool_global = False
char_global = "a"
dict_global = {"a": 1, "b": 2}
list_global = [[], [0], [0, 0]]


def get_global(name):
    return globals()[name]


def accept_int_string_bool_float(i, s, b, f):
    is_int = type(i) is int
    is_string = type(s) is str
    is_bool = type(b) is bool
    is_float = type(f) is float

    return is_int and is_string and is_bool and is_float


def accept_dict(d, key):
    return d[key]


def accept_list(l):
    result = ""
    for e in l:
        result += e + ","
    return result


def identity(x):
    return x
