extern "C" int function(const char* some_string)
{
    if (some_string != nullptr) {
        return static_cast<int>(*some_string);
    }
    return -1;
}
