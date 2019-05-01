#include <string>
#include <iostream>

void replace(char *string, char c1, char c2)
{
    char *p = string;
    while (*p)
    {
        if (*p == c1)
            *p = c2;
        p++;
    }
}

void delete_sym(char *string, char c1)
{
    char *p = string;
    while (*p)
    {
        if (*p == c1)
        {
            char *helpptr = p;
            while (*helpptr)
                *helpptr = *(++helpptr);
        }
        else p++;
    }
}

void replace(std::string &str, char c1, char c2)
{
    for (auto iter = str.begin(); iter != str.end(); iter++)
    {
        if (*iter == c1)
            *iter = c2;
    }
}

// int main()
// {
//     char str[] = "hehe manho";
//     delete_sym(str, 'h');
//     std::cout << str << std::endl;
// }