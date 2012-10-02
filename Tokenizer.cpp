#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string &str) 
{
	tokenizeFast(str);
}

Tokenizer::Tokenizer(const std::string &str, const std::string &chars)
{
	tokenize(str, chars);
}

    // Fast version that only looks for space ' '
void Tokenizer::tokenizeFast(const std::string &str)
{
    tokens.clear();

    size_t begin = 0;

    for(size_t i=0; i<str.length(); ++i)
    {
        if(str[i] == ' ')
        {
            if(begin != i)
            {
                tokens.push_back(str.substr(begin,i-begin));
                begin = i+1;
            }
            else
                ++begin;
        }
    }
    if(begin != str.length())
        tokens.push_back(str.substr(begin));

    counter = 0;
}

// Generic version that checks for any characters in the string chars.
void Tokenizer::tokenize(const std::string &str, const std::string &chars)
{
    tokens.clear();

    size_t begin = 0;
    size_t u;

    for(size_t i=0; i<str.length(); ++i)
    {
        for(u=0; u<chars.length(); ++u)
        {
            if(str[i]==chars[u])
            {
                if(begin != i)
                {
                    tokens.push_back(str.substr(begin,i-begin));
                    begin = i+1;
                }
                else
                    ++begin;
            }
        }
    }
    if(begin != str.length())
        tokens.push_back(str.substr(begin));

    counter = 0;
}

const std::string &Tokenizer::getNext()
{
    if(counter < tokens.size())
    {
        return tokens[counter++];
    }
    else
        return defstr;
}

