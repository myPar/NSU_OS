#include "Tokenizer.h"

string Tokenizer::get_token(string src_string, string delimiter, int token_idx) {
    string result;

    for (int i = 0; i <= token_idx; i++) {
        size_t del_pos = src_string.find(delimiter);
        
        if (del_pos == string::npos) {
            if (i == token_idx) {
                return src_string;  // return remaining string (this is last token)
            }
            break;
        }
        if (i == token_idx) {
            return src_string.substr(0, del_pos);
        }
        src_string.erase(0, del_pos + delimiter.length());  // erase current token (del included)
    }

    return result;
}