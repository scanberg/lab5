//  Tokenizer.h
//
//  A lightweight tokenizer for strings.
//  Internally it uses two different versions,
//  one to recognize blank space ' ' and one generic
//  to recognize characters placed in a string, ex. " ,."
//
//  Author: Robin Skånberg
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <vector>
#include <iostream>
#include <string>

class Tokenizer
{
private:
    std::vector<std::string> tokens;
    const std::string defstr;
    unsigned int counter;

public:
    Tokenizer() {}
    Tokenizer(const std::string &str);

    Tokenizer(const std::string &str, const std::string &chars);

    // Fast version that only looks for space ' '
    void tokenizeFast(const std::string &str);

    // Generic version that checks for any characters in the string chars.
    void tokenize(const std::string &str, const std::string &chars = " ");

    const std::string &getNext();
    void reset() { counter=0; }
    bool end() { return counter==tokens.size(); }
    unsigned int size() { return tokens.size(); }
};

#endif
