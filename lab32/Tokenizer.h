#pragma once
#include <string>
using namespace std;

class Tokenizer {
public:
    static string get_token(string src_string, string delimiter, int token_idx);
};