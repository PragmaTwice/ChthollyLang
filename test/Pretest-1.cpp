/// Chtholly Test
/// <license> Under Apache License </license>


#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <tuple>

using namespace std;

class lexer
{
    using Lang = string;
    using Iter = string::const_iterator;
    using Res = vector<pair<string,string>>;

    Lang lang;
    Res res;

    /// S = (Int{push} | Float{push}) SLoop
    Iter S(Iter i)
    {
        if(auto j = Float(i); j!=i)
        {
            res.push_back({"float",{i,j}});
			return SLoop(j);
        }
        else if(auto j = Int(i); j!=i)
        {
            res.push_back({"int",{i,j}});
			return SLoop(j);
        }

        return i;
    }

	/// SLoop = Blanks S?
	Iter SLoop(Iter i)
	{
		if (auto j = Blank(i); j != i)
		{
			return S(j);
		}

		return i;
	}

    /// Int = Digit Int?
    Iter Int(Iter i)
    {
		if (auto j = Digit(i); j != i)
        {
            return Int(j);
        }

        return i;
    }

    /// Float = Int Dot Int (E Int)?
    Iter Float(Iter i)
    {
		if (auto j=Int(i); j != i)
			if (auto k = Dot(j); k != j)
                if(auto l = Int(k); l!=k)
                {
                    if(auto m = E(l); m!=l)
                        if(auto n = Int(m); n!=m) return n;
                    return l;
                }
        
        return i;
    }

    /// Digit = '0'...'9'
    Iter Digit(Iter i)
    {
        if(i!=lang.end()&&*i>='0'&&*i<='9') return ++i;
        return i;
    }

    /// Blanks = ' ' Blanks?
    Iter Blank(Iter i)
    {
        if(i!=lang.end()&&*i==' ') return Blank(++i);
        return i;
    }

    /// Dot = '.'
    Iter Dot(Iter i)
    {
        if(i!=lang.end()&&*i=='.') return ++i;
        return i;
    }

    /// E = 'E' | 'e'
    Iter E(Iter i)
    {
        if(i!=lang.end()&&(*i=='e'||*i=='E')) return ++i;
        return i;
    }

public:

    lexer(const Lang& _lang): lang(_lang)
    {
        auto end = S(lang.begin());
        cout << "total process: " << (end-lang.begin()) << endl;
        for (auto& r : res) cout << r.first << ":" << r.second << endl;
    }

};

int main()
{
    string input;
    getline(cin,input);

    lexer l(input);

    return cin.get(),0;
}
