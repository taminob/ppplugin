def accept_int_string_bool_float(i, s, b, f):
    is_int = type(i) is int
    is_string = type(s) is str
    is_bool = type(b) is bool
    is_float = type(f) is float

    return is_int and is_string and is_bool and is_float


def identity(x):
    return x
