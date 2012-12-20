/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SymHop.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-07-19
//!
//! @brief Contains the SymHop library for symbolic expressions
//!
//$Id$

#include <cassert>
#include <cmath>

#include "SymHop.h"

using namespace std;
using namespace SymHop;

//! @class Expression
//! @brief The Expression class implement a class for symbolic expressions
//! @author Robert Braun <robert.braun@liu.se>
//!
//! Expressions are stored in a tree structures.
//!
//! Allowed operators are:
//! "*", "/", "+", "%"
//! (and "^", but it is to be replaced by pow() function)
//!
//! Allowed functions are:
//! "div", "rem", "mod", "tan", "cos", "sin", "atan", "acos", "asin", "atan2",
//! "sinh", "cosh", "tanh", "log", "exp", "sqrt", "sign", "abs", "der", "onPositive", "onNegative",
//! "signedSquareL", "limit", "integer", "floor", "ceil", "hopsanLimit", "hopsanDxLimit", "onPositive",
//! "onNegative", "signedSquareL", "limit"


//! @brief Constructor for Expression class using QString
//! @param indata String containg a numerical expression
//! @param simplifications Specifies the degree of simplification
//FIXED
Expression::Expression(const QString indata, const ExpressionSimplificationT simplifications)
{
    commonConstructorCode(QStringList() << indata, simplifications);
}


//! @brief Constructor for Expression class using QStringList
//! @details This constructor is faster than the one using a string, because the string does not need to be parsed.
//! @param symbols String list containg numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree, this shall never be used when defining a new expression
//FIXED
Expression::Expression(QStringList symbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    commonConstructorCode(symbols, simplifications, parentSeparator);
}


////! @brief Constructor for Expression using left and right expressions and an operator
////! @details This is the fastest way to create a new expression of operator type, because no evaulations or parsing needs to be performed.
////! @param left Left side Expression
////! @param right Right side expression
////! @param mid Operator ("+", "*" or "/")
////! @param simplifications Specifies the degree of simplification
//Expression::Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications)
//{
//    if(mid == "+" || mid == "*" || mid == "/" || mid == "=")
//    {
//        mType = Expression::Operator;
//        mString = mid;
//        mChildren.append(left);
//        mChildren.append(right);
//        _simplify(simplifications);
//    }
//}


//! @brief Constructor for Expression using a list of expressions joined by operaetors
//! @example Expression([A,B,C,D], "+") will result in A+B+C+D
//! @param children List of child expressions
//! @param separator Operator used to combine child expressions ("+", "*" or "/")
//! @param simplifications Specifies the degree of simplification
//FIXED
Expression::Expression(const QList<Expression> children, const QString separator)
{
    if(separator == "+")
    {
        mTerms = children;
    }
    else if(separator == "*")
    {
        mFactors = children;
    }
    else if(separator == "/")
    {
        mFactors.append(children[0]);
        mDivisors = children.mid(1, children.size()-1);
    }
}


//! @brief Constructor for creating an expression from a numerical value
//! @param value Value of new symbol
//FIXED
Expression::Expression(const double value)
{
    mpLeft = 0;
    mpRight = 0;
    mpBase = 0;
    mpPower = 0;
    mpDividend = 0;

    mString = QString::number(value);

    //Make sure numerical symbols have double precision
    bool isInt;
    mString.toInt(&isInt);
    if(isInt && !mString.contains("."))
    {
        mString.append(".0");
    }
}


//! @brief Common constructor code that constructs an Expression from a string list
//! @param symbols String list containg numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree
//FIXED
void Expression::commonConstructorCode(QStringList symbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    mpLeft = 0;
    mpRight = 0;
    mpBase = 0;
    mpPower = 0;
    mpDividend = 0;

    //Only one symbol, so parse it as a string
    if(symbols.size() == 1)
    {
        QString str = symbols.first();
        symbols.clear();

        //Don't create empty expressions
        if(str.isEmpty())
        {
            return;
        }

        //Trivial simplifications before parsing
        if(str.size() > 1)
        {
            str.replace("**", "^");
            str.remove(" ");
            str.replace("--", "+");
            str.replace("-", "+-");
            str.replace("--", "+");
            str.replace("/+", "/");
            str.replace("*+", "*");
            str.replace("^+", "^");
        }
        while(str.contains("++"))
        {
            str.replace("++", "+");
        }
        while(str.contains("+-+-"))
        {
            str.replace("+-+-","+-");
        }
        while(str.contains("-("))
        {
            str.replace("-(", "(-");
        }
        while(str.startsWith("+"))
        {
            str = str.right(str.size()-1);
        }
        while(str.contains("=+"))
        {
            str.replace("=+", "=");
        }

        //Remove all excessive parentheses
        while(str.startsWith("(") && str.endsWith(")"))
        {
            QString testString = str.mid(1, str.size()-2);
            if(verifyParantheses(testString))
            {
                str = testString;
            }
            else
            {
                break;
            }
        }

        //Remove "+" sign at the beginning
        while(str.startsWith("+"))
        {
            str = str.right(str.size()-1);
        }

        //Generate a list of symbols from the string
        bool var = false;   //True if current symbol might be  a variable
        int parBal = 0;     //Parenthesis balance counter
        int start=0;        //Start index of each symbol
        for(int i=0; i<str.size(); ++i)
        {
            if(!var && parBal==0 && (str.at(i).isLetterOrNumber() || str.at(i) == '_' || str.at(i) == '-' || str.at(i) == '.')) //New variable or function string
            {
                var = true;
                start = i;
            }
            else if(var && str.at(i) == '(')    //String contains parentheses, so it is not a variable (but a function)
            {
                var = false;
                parBal++;
            }
            else if(!var && str.at(i) == '(')   //New string that begins with parenthesis
            {
                parBal++;
            }
            else if(str.at(i) == ')')      //End parenthesis, append string to symbols
            {
                parBal--;
                if(parBal == 0)
                {
                    var = false;
                    symbols.append(str.mid(start, i-start+1));
                }
            }
            else if(var && !(str.at(i).isLetterOrNumber() || str.at(i) == '_' || str.at(i) == '.'))     //End of variable, append it to symbols
            {
                var = false;
                symbols.append(str.mid(start, i-start));
                --i;
            }
            else if(!var && parBal == 0)    //Something else than a variable and a function (i.e. an operator), append it to the list of symbols
            {
                symbols.append(str.at(i));
                start = i+1;
            }


            if(i == str.size()-1 && !var && parBal == 0 && str.at(i) != ')')    //Make sure to append last symbol
            {
                symbols.append(str.at(i));
            }
            else if(i == str.size()-1 && (var || parBal>0) && str.at(i) != ')')
            {
                symbols.append(str.mid(start, i-start+1));
            }
        }
    }


    //Store function derivatives in object
    //! @todo This shall probably not be stored in every expression
    mFunctionDerivatives.insert("sin", "cos");
    mFunctionDerivatives.insert("cos", "-sin");
    mFunctionDerivatives.insert("abs", "sign");
    mFunctionDerivatives.insert("onPositive", "dxOnPositive");
    mFunctionDerivatives.insert("onNegative", "dxOnNegative");
    mFunctionDerivatives.insert("signedSquareL", "dxSignedSquareL");
    mFunctionDerivatives.insert("limit", "dxLimit");

    //Store reserved symbols in object
    reservedSymbols << "mTime" << "Z";


    //Find top level symbol, set correct string and type, generate children
    if(splitAtSeparator("=", symbols, simplifications))                        //Equality
    {
        //mType = Expression::Equality;
    }
    else if(splitAtSeparator("+", symbols, simplifications))                   //Addition
    {
        //mType = Expression::Operator;
    }
    else if(splitAtSeparator("*", symbols, simplifications))                   //Multiplication/division
    {
       // mType = Expression::Operator;
    }
    else if(splitAtSeparator("^", symbols, simplifications))                   //Power
    {
        //mType = Expression::Operator;
    }
    else if(splitAtSeparator("%", symbols, simplifications))                   //Modulus
    {
        //mType = Expression::Operator;
    }
    else if(symbols.size() == 1 && symbols.first().contains("("))                   //Function
    {
        QString str = symbols.first();
        //mType = Expression::Function;
        mFunction = str.left(str.indexOf("("));
        str = str.mid(str.indexOf("(")+1, str.size()-2-str.indexOf("("));

        QStringList args = splitWithRespectToParentheses(str, ',');
        //qDebug() << "Function: " << mString << ", arguments: " << args;
        for(int i=0; i<args.size(); ++i)
        {
            mArguments.append(Expression(args.at(i)));
        }
    }
    else                                                                            //Symbol
    {
        //mType = Expression::Symbol;
        mString = symbols.first();

        //Make sure numerical symbols have double precision
        bool isInt;
        mString.toInt(&isInt);
        if(isInt && !mString.contains("."))
        {
            mString.append(".0");
        }

        //Replace negative symbols with multiplication with -1
        if(mString.startsWith("-") && mString != "-1.0" && int(mString.toDouble()) != -1)
        {
            mString = mString.right(mString.size()-1);
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("-1") << (*this), QList<Expression>()));
        }
    }


    //Perform simplifications (but not for symbols, because that is pointless)
    if(this->isVariable() || this->isNumericalSymbol())
    {
        _simplify(simplifications);
    }
}


//! @brief Equality operator for expressions
//FIXED
bool Expression::operator==(const Expression &other) const
{
    bool baseOk = !(mpBase && other.getBase()) || (mpBase && other.getBase() && *mpBase == (*other.getBase()));
    bool powerOk = !(mpPower && other.getPower()) || (mpPower && other.getPower() && *mpPower == (*other.getPower()));
    bool leftOk = !(mpLeft && other.getLeft()) || (mpLeft && other.getLeft() && *mpLeft == (*other.getLeft()));
    bool rightOk = !(mpRight && other.getRight()) || (mpRight && other.getRight() && *mpRight == (*other.getRight()));
    bool dividendOk = !(mpDividend && other.getDividends()) || (mpDividend && other.getDividends() && *mpDividend == (*other.getDividends()));
    bool termsOk=true;
    if(this->isAdd() && other.isAdd())
    {
        termsOk = (mTerms == other.getTerms());
    }
    bool stringOk = mString == other._getString();
    bool divisorOk = mDivisors == other.getDivisors();
    bool factorOk = true;
    if(this->isMultiplyOrDivide() && other.isMultiplyOrDivide())
    {
        factorOk = mFactors == other.getFactors();
    }

    return (stringOk &&
            termsOk &&
            divisorOk &&
            factorOk &&
            baseOk && powerOk && leftOk && rightOk && dividendOk);
}


//FIXED
Expression Expression::fromTwoTerms(const Expression term1, const Expression term2)
{
    return Expression::fromTerms(QList<Expression>() << term1 << term2);
}

//FIXED
Expression Expression::fromTerms(const QList<Expression> terms)
{
    Expression ret;
    ret.mTerms = terms;
    return ret;
}

//FIXED
Expression Expression::fromTwoFactors(const Expression factor1, const Expression factor2)
{
    return Expression::fromFactorsDivisors(QList<Expression>() << factor1 << factor2, QList<Expression>());
}


//FIXED
Expression Expression::fromFactorDivisor(const Expression factor, const Expression divisor)
{
    return Expression::fromFactorsDivisors(QList<Expression>() << factor, QList<Expression>() << divisor);
}


//FIXED
Expression Expression::fromFactorsDivisors(const QList<Expression> factors, const QList<Expression> divisors)
{
    assert(factors.size()+divisors.size() > 1);
    Expression ret;
    ret.mFactors = factors;
    ret.mDivisors = divisors;
    return ret;
}

//FIXED
Expression Expression::fromBasePower(const Expression base, const Expression power)
{
    Expression ret;
    (*ret.mpBase) = base;
    (*ret.mpPower) = power;
    return ret;
}

//FIXED
Expression Expression::fromFunctionArguments(const QString function, const QList<Expression> arguments)
{
    Expression ret;
    ret.mFunction = function;
    ret.mArguments = arguments;
    return ret;
}

//FIXED
Expression Expression::fromEquation(const Expression left, const Expression right)
{
    Expression ret;
    (*ret.mpLeft) = left;
    (*ret.mpRight) = right;
    return ret;
}



//! @brief Counts how many times a sub Expression is used in the expression
//! @param var Expression to count
//FIXED
int Expression::count(const Expression &var) const
{
    if(*this == var)
    {
        return 1;
    }

    int retval=0;
    for(int c=0; c<mFactors.size(); ++c)
    {
        retval += mFactors[c].count(var);
    }
    for(int c=0; c<mDivisors.size(); ++c)
    {
        retval += mDivisors[c].count(var);
    }
    for(int c=0; c<mTerms.size(); ++c)
    {
        retval += mTerms[c].count(var);
    }
    retval += mpBase->count(var);
    retval += mpPower->count(var);
    retval += mpLeft->count(var);
    retval += mpRight->count(var);
    retval += mpDividend->count(var);

    return retval;
}


//! @brief Replaces this expression by another one
//! @param expr Expression to replace by
//FIXED
void Expression::replaceBy(const Expression expr)
{
    mString = expr._getString();
    mFactors = expr.getFactors();
    mDivisors = expr.getDivisors();
    mTerms = expr.getTerms();
    if(expr.getBase())
        *mpBase = *expr.getBase();
    if(expr.getPower())
        *mpPower = *expr.getPower();
    if(expr.getLeft())
        *mpLeft = *expr.getLeft();
    if(expr.getRight())
        *mpRight = *expr.getRight();
    if(expr.getDividends())
        *mpDividend = *expr.getDividends();
}


//! @brief Divides the expression by specified divisor
//! @param div Divisor expression
//FIXED
void Expression::divideBy(const Expression div)
{
    assert(!(div == Expression(0.0)));

    QList<Expression> divSet;
    divSet << *this;
    divSet << div;
    this->replaceBy(Expression(divSet, "/"));
}


//! @brief Multiplies the expression by specified divisor
//! @param fac Factor expression
//FIXED
void Expression::multiplyBy(const Expression fac)
{
    QList<Expression> mulSet;
    mulSet << *this;
    mulSet << fac;
    this->replaceBy(Expression(mulSet, "*"));
}


//! @brief Adds the expression with specified tern
//! @param tern Term expression
//FIXED
void Expression::addBy(const Expression term)
{
    QList<Expression> addSet;
    addSet << *this;
    addSet << term;
    this->replaceBy(Expression(addSet, "+"));
}


//! @brief Subtracts the expression by specified term
//! @param tern Term expression
//FIXED
void Expression::subtractBy(const Expression term)
{
    QList<Expression> subSet;
    subSet << *this;
    subSet << term;
    this->replaceBy(Expression(subSet, "-"));
}


//! @brief Returns the expression converted to a string
//FIXED
QString Expression::toString() const
{
    QString ret;

    if(this->isVariable() || this->isNumericalSymbol())
    {
        ret = mString;
    }
    else if(this->isFunction())
    {
        ret = mFunction+"(";
        for(int i=0; i<mArguments.size(); ++i)
        {
            ret.append(mArguments[i].toString()+",");
        }
        ret.chop(1);
        ret.append(")");
    }
    else if(this->isEquation())
    {
        QString leftStr =mpLeft->toString();
        QString rightStr = mpRight->toString();
        ret = leftStr + "=" + rightStr;
    }
    else if(this->isAdd())
    {
        Q_FOREACH(const Expression &term, mTerms)
        {
            QString termString = term.toString();
            ret.append(termString);
            ret.append("+");
        }
        ret.chop(1);
    }
    else if(this->isMultiplyOrDivide())
    {
        Q_FOREACH(const Expression &factor, mFactors)
        {
            QString factString = factor.toString();
            if(factor.isAdd())
            {
                factString.prepend("(");
                factString.append(")");
            }
            ret.append(factString);
            ret.append("*");
        }
        if(mFactors.isEmpty())
        {
            ret.append("1");
        }
        else
        {
            ret.chop(1);
        }
        Q_FOREACH(const Expression &divisor, mDivisors)
        {
            QString divString = divisor.toString();
            ret.append("/");
            if(mDivisors.size() > 1)
            {
                ret.append("(");
            }
            if(divisor.isAdd())
            {
                divString.prepend("(");
                divString.append(")");
            }
            ret.append(divString);
            ret.append("*");
        }
        if(mDivisors.size() > 0)
        {
            ret.chop(1);
        }
        else if(mDivisors.size() > 1)
        {
            ret.append(")");
        }
    }
    else if(this->isPower())
    {
        QString baseStr = mpBase->toString();
        if(mpBase->isAdd() || mpBase->isMultiplyOrDivide())
        {
            baseStr.prepend("(");
            baseStr.append(")");
        }
        QString powerStr = mpPower->toString();
        if(mpPower->isAdd() || mpBase->isMultiplyOrDivide())
        {
            powerStr.prepend("(");
            powerStr.append(")");
        }
        ret = baseStr+"^"+powerStr;
    }

    //Simplify output
    ret.replace("+-", "-");
    ret.replace("--", "");

    return ret;
}


//! @brief Converts all delay operators ("Z") to delay functions ("mDelay") and extracts the delay terms
//! @details This function assumes that the function is linearized, so that there are no Z operators in divisors.
//! @param delayTerms Reference to list of delayed terms; new terms are appended
//! @param delaySteps Reference to list of delay steps for each term; new values are appended
//FIXED
void Expression::toDelayForm(QList<Expression> &rDelayTerms, QStringList &rDelaySteps)
{
    //Generate list of terms
    QList<QList<Expression> > termMap;
    QList<Expression> terms = getTerms();
    if(terms.isEmpty())
    {
        terms.append(*this);
    }

    //Cycle terms
    QList<Expression>::iterator t;
    for (t = terms.begin(); t != terms.end(); ++t)
    {
        int idx = (*t).getFactors().count(Expression("Z"));

        //Remove all Z operators
        if(idx > 0)
        {
            (*t).removeFactor(Expression("Z"));
        }

        while(termMap.size() < idx+1)
        {
            termMap.append(QList<Expression>());
        }

        //Store delay term
        termMap[idx].append(*t);
    }

    //Replace delayed terms with delay function and store delay terms and delay steps in reference vectors
    QStringList ret;
    for(int i=termMap.size()-1; i>0; --i)
    {
        if(!termMap[i].isEmpty())
        {
            QStringList delayTermSymbols;
            for(int t=0; t<termMap[i].size(); ++t)
            {
                delayTermSymbols << "("+termMap[i][t].toString()+")" << "+";
            }
            delayTermSymbols.removeLast();
            Expression delayTerm = Expression(delayTermSymbols);

            delayTerm.factorMostCommonFactor();

            QString term = "mDelay"+QString::number(rDelayTerms.size())+".getIdx(1.0)";
            ret.append(term);
            ret.append("+");

            rDelayTerms.append(delayTerm);
            rDelaySteps.append(QString::number(i));
        }
    }
    for(int t=0; t<termMap[0].size(); ++t)
    {
        ret.append("("+termMap[0][t].toString()+")");
        ret.append("+");
    }
    ret.removeLast();

    //Replace this expression by the new one
    this->replaceBy(Expression(ret));

    //Simplify
    this->_simplify(Expression::FullSimplification, Recursive);
}


//! @brief Converts expression to double if possible
//! @param ok True if success, false if failed (= not a numerical symbol)
//FIXED
double Expression::toDouble(bool *ok) const
{
    if(this->isVariable() || this->isNumericalSymbol())
    {
        return mString.toDouble(ok);
    }
    else
    {
        *ok = false;
        return 0.0;
    }
}



//! @brief Tells whether or not this is a power operator
//FIXED
bool Expression::isPower() const
{
    return !(mpPower == 0);
}


//! @brief Tells whether or not this is a multiplication or division
//FIXED
bool Expression::isMultiplyOrDivide() const
{
    return !mFactors.isEmpty();
}


//! @brief Tells whether or not this is an addition
//FIXED
bool Expression::isAdd() const
{
    return !mTerms.isEmpty();
}


//FIXED
bool Expression::isFunction() const
{
    return !mFunction.isEmpty();
}


//! @brief Tells whether or not this is a numerical symbol
//FIXED
bool Expression::isNumericalSymbol() const
{
    return !mString.isEmpty();
}


//FIXED
bool Expression::isVariable() const
{
    return (mString[0].isLetter());
}


//! @brief Tells whether or not this is an assignment
//FIXED
bool Expression::isAssignment() const
{
    return (this->isEquation() && (mpLeft->isVariable() || mpLeft->isNumericalSymbol()));
}


//! @brief Tells whether or not this is an equation
//FIXED
bool Expression::isEquation() const
{
    return (this->isEquation());
}


//! @brief Tells whether or not this is a negative symbol
//FIXED
bool Expression::isNegative() const
{
    if(this->isVariable() || this->isNumericalSymbol() && mString == "-1")
    {
        return true;
    }
    else if(mFactors.contains(Expression("-1")))
    {
        return true;
    }
    return false;
}


//FIXED
void Expression::changeSign()
{
    if(this->isNegative())
    {
        mFactors.removeOne(Expression("-1"));
    }
    else
    {
        if(isMultiplyOrDivide())
        {
            mFactors << Expression("-1");
        }
        else
        {
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("-1") << (*this), QList<Expression>()));
        }
    }
}


//! @brief Returns the derivative of the expression
//! @param x Expression to differentiate with
//! @param ok True if successful, otherwise false
//FIXED
Expression Expression::derivative(const Expression x, bool &ok) const
{
    ok = true;
    QString ret;

    //Equality, differentiate left and right expressions
    if(this->isEquation())
    {
        bool success;
        ret.append(mpLeft->derivative(x, success).toString());
        if(!success) { ok = false; }
        ret.append("=");
        ret.append(mpRight->derivative(x, success).toString());
        if(!success) { ok = false; }
    }
    //Function
    else if(this->isFunction())
    {
        QString f = this->toString();
        QString g;
        QString dg;
        if(!mArguments.isEmpty())
        {
            g = mArguments[0].toString();    //First argument
            bool success;
            dg = mArguments[0].derivative(x, success).toString();    //Derivative of first argument
            if(!success) { ok = false; }
        }

        QString func = mString;
        bool negative = false;
        if(func.startsWith('-'))        //Rememver that the function was negative
        {
            func = func.right(func.size()-1);
            negative = true;
        }

        //Custom functions
        if(func == "log")
        {
            ret = "("+dg+")/("+g+")";
        }
        else if(func == "exp")
        {
            ret = "("+dg+")*("+f+")";
        }
        else if(func == "tan")
        {
            ret = "2*("+dg+")/(cos(2*"+g+")+1)";
        }
        else if(func == "atan" || func == "atan2")
        {
            ret = "("+dg+")/(("+g+")^2+1)";
        }
        else if(func == "asin")
        {
            ret = "("+dg+")/sqrt(1-("+g+")^2)";
        }
        else if(func == "acos")
        {
            ret = "-("+dg+")/sqrt(1-("+g+")^2)";
        }
        else if(func == "mod")
        {
            ret = "0.0";
        }
        else if(func == "rem")
        {
            ret = "0.0";
        }
        else if(func == "sqrt")
        {
            ret = "("+dg+")/(2*sqrt("+g+"))";
        }
        else if(func == "sign")
        {
            ret = "0.0";
        }
        else if(func == "re")
        {
            ret = "0.0";
        }
        else if(func == "ceil")
        {
            ret = "0.0";
        }
        else if(func == "floor")
        {
            ret = "0.0";
        }
        else if(func == "int")
        {
            ret = "0.0";
        }
        else if(func == "dxLimit")
        {
            ret = "0.0";
        }
        else if(func == "mDelay")
        {
            ret = "0.0";
        }
        else if(func.startsWith("mDelay"))
        {
            ret = "0.0";
        }
        else if(func == "pow")
        {
            if(g == "Z" || g == "-Z")
            {
                ret = "0.0";
            }
            else
            {
                bool success;
                QString f = mArguments[0].toString();
                QString df = mArguments[0].derivative(x, success).toString();
                if(!success) { ok = false; }
                QString g = mArguments[1].toString();
                QString dg = mArguments[1].derivative(x, success).toString();
                if(!success) { ok = false; }

                ret = "pow("+f+","+g+"-1)*(("+g+")*("+df+")+("+f+")*log(("+f+"))*("+dg+"))";
            }
        }
        //No special function, so use chain rule
        else
        {
            if(!mFunctionDerivatives.contains(func))
            {
                //QMessageBox::critical(0, "SymHop", "Could not compute function derivative of \"" + func +"\": Not implemented.");
                ok = false;
            }
            else
            {
                ret = this->toString();
                ret = ret.mid(ret.indexOf("("), ret.size()-1);
                ret.prepend(mFunctionDerivatives.find(func).value());
                ret.append("*");
                bool success;
                ret.append(mArguments.first().derivative(x, success).toString());
                if(!success) { ok = false; }
            }
        }

        if(negative)
        {
            ret.prepend("-");
        }
    }

    //Multiplication, d/dx(f(x)*g(x)) = f'(x)*g(x) + f(x)*g'(x)
    else if(isMultiplyOrDivide())
    {
        //Derivative of Z is zero
        if(mFactors.contains(Expression("Z")))
        {
            ret = "0.0";
        }
        else if(!mDivisors.isEmpty())
        {
            Expression exp1 = Expression::fromFactorsDivisors(QList<Expression>() << mFactors, QList<Expression>());
            Expression exp2 = Expression::fromFactorsDivisors(QList<Expression>() << mDivisors, QList<Expression>());

            bool success;
            Expression der1 = derivative(exp1, success);
            if(!success) { ok = false; }
            Expression der2 = derivative(exp2, success);
            if(!success) { ok = false; }

            QString expstr1 = exp1.toString();
            QString expstr2 = exp2.toString();
            QString derstr1 = der1.toString();
            QString derstr2 = der2.toString();

            ret = "(("+expstr2+")*("+derstr1+")-("+expstr1+")*("+derstr2+"))/("+expstr2+")^2";
        }
        else
        {
            bool success;
            Expression exp1 = (*mFactors.begin());
            Expression der1 = exp1.derivative(x, success);
            if(!success) { ok = false; }
            QList<Expression> rest = mFactors;
            rest.removeFirst();
            Expression exp2;
            if(rest.size() == 1)
            {
                exp2 = rest.first();
            }
            else
            {
                exp2 = Expression::fromFactorsDivisors(rest, QList<Expression>());
            }
            Expression der2 = exp2.derivative(x, success);
            if(!success) { ok = false; }

            Expression firstTerm = Expression::fromTwoFactors(der1, exp2);
            Expression secondTerm = Expression::fromTwoFactors(exp1, der2);
            return Expression::fromTwoTerms(firstTerm, secondTerm);
            //! @todo Only this part is returning an expression, make the other ones do the same
        }
    }
    //Addition
    else if(isAdd())
    {
        QList<Expression> derSet;
        Q_FOREACH(const Expression &term, mTerms)
        {
            derSet << term.derivative(x, ok);
        }
        ret = Expression::fromTerms(derSet).toString();
    }

    //Power, d/dx(f(x)^g(x)) = (g(x)*f'(x)+f(x)*log(f(x))*g'(x)) * f(x)^(g(x)-1)
    else if(this->isPower())
    {
        bool success;
        QString f = mpBase->toString();
        QString df = mpBase->derivative(x, success).toString();
        if(!success) { ok = false; }
        QString g = mpPower->toString();
        QString dg = mpPower->derivative(x, success).toString();
        if(!success) { ok = false; }

        ret = "("+f+")^("+g+"-1)*(("+g+")*("+df+")+("+f+")*log(("+f+"))*("+dg+"))";
    }

    //Symbol
    else
    {
        if(*this == x)
        {
            ret = "1.0";
        }
        else
        {
            ret = "0.0";
        }
    }

    return Expression(ret);
}


//! @brief Returns whether or not expression contains a sub expression
//! @param expr Expression to check for
//FIXED
bool Expression::contains(const Expression expr) const
{
    if(*this == expr)
    {
        return true;
    }

    Q_FOREACH(const Expression &term, mTerms)
    {
        if(term.contains(expr))
        {
            return true;
        }
    }
    for(int i=0; i<mArguments.size(); ++i)
    {
        if(mArguments[i].contains(expr))
        {
            return true;
        }
    }
    Q_FOREACH(const Expression &factor, mTerms)
    {
        if(factor.contains(expr))
        {
            return true;
        }
    }
    Q_FOREACH(const Expression &divisor, mDivisors)
    {
        if(divisor.contains(expr))
        {
            return true;
        }
    }
    if(mpBase->contains(expr))
    {
        return true;
    }
    if(mpPower->contains(expr))
    {
        return true;
    }
    if(mpLeft->contains(expr))
    {
        return true;
    }
    if(mpRight->contains(expr))
    {
        return true;
    }
    if(mpDividend->contains(expr))
    {
        return true;
    }

    return false;
}


//! @brief Converts time derivatives (der) in the expression to Z operators with bilinar transform
//FIXED
Expression Expression::bilinearTransform() const
{
    Expression retExpr;
    retExpr.replaceBy(*this);
    QStringList res;

    if(this->isAdd())
    {
        QList<Expression> newTerms = mTerms;
        QList<Expression>::iterator it;
        for(it=newTerms.begin(); it!=newTerms.end(); ++it)
        {
            (*it).bilinearTransform();
        }
        retExpr = Expression::fromTerms(newTerms);
    }
    else if(this->isEquation())
    {
        Expression newLeft = *mpLeft;
        Expression newRight = *mpRight;
        newLeft.bilinearTransform();
        newRight.bilinearTransform();
        retExpr = Expression::fromEquation(newLeft, newRight);
    }
    else if(this->isMultiplyOrDivide())
    {
        QList<Expression> newFactors = mFactors;
        QList<Expression>::iterator it;
        for(it=newFactors.begin(); it!=newFactors.end(); ++it)
        {
            (*it).bilinearTransform();
        }
        QList<Expression> newDivisors = mDivisors;
        for(it=newDivisors.begin(); it!=newDivisors.end(); ++it)
        {
            (*it).bilinearTransform();
        }
        retExpr = Expression::fromFactorsDivisors(newFactors, newDivisors);
    }
    else if(this->isFunction())
    {
        if(mFunction == "der")
        {
            QString arg = retExpr.getArgument(0).toString();
            res << "2.0" << "/" << "mTimestep" << "*" << "(1.0-Z)" << "/" << "(1.0+Z)" << "*" << "("+arg+")";
            retExpr = Expression(res);
        }
    }

    return retExpr;
}


//! @brief Returns a list with all contained symbols in the expression
//FIXED
QList<Expression> Expression::getSymbols() const
{
    QList<Expression> retval;

    if(this->isAdd())
    {
        Q_FOREACH(const Expression &term, mTerms)
        {
            retval.append(term.getSymbols());
        }
    }
    else if(this->isEquation())
    {
        retval.append(mpLeft->getSymbols());
        retval.append(mpRight->getSymbols());
    }
    else if(this->isMultiplyOrDivide())
    {
        Q_FOREACH(const Expression &factor, mFactors)
        {
            retval.append(factor.getSymbols());
        }
        Q_FOREACH(const Expression &divisor, mDivisors)
        {
            retval.append(divisor.getSymbols());
        }
    }
    else if(this->isFunction())
    {
        Q_FOREACH(const Expression &argument, mArguments)
        {
            retval.append(argument.getSymbols());
        }
    }
    else if(this->isPower())
    {
        retval.append(mpBase->getSymbols());
        retval.append(mpPower->getSymbols());
    }
    else if(this->isVariable() && !reservedSymbols.contains(mString))
    {
        retval.append(*this);
    }

    removeDuplicates(retval);

    return retval;
}


//! @brief Returns a list with all used functions in the expression
//FIXED
QStringList Expression::getFunctions() const
{
    QStringList retval;

    if(this->isAdd())
    {
        QList<Expression>::iterator it;
        Q_FOREACH(const Expression &term, mTerms)
        {
            retval.append(term.getFunctions());
        }
    }
    else if(this->isEquation())
    {
        retval.append(mpLeft->getFunctions());
        retval.append(mpRight->getFunctions());
    }
    else if(this->isMultiplyOrDivide())
    {
        Q_FOREACH(const Expression &factor, mFactors)
        {
            retval.append(factor.getFunctions());
        }
        Q_FOREACH(const Expression &divisor, mDivisors)
        {
            retval.append(divisor.getFunctions());
        }
    }
    else if(this->isPower())
    {
        retval.append(mpBase->getFunctions());
        retval.append(mpPower->getFunctions());
    }
    else if(this->isFunction())
    {
        retval.append(mFunction);
    }

    retval.removeDuplicates();

    return retval;
}


//! @brief Returns the function name, or an empty string is this is not a function
//FIXED
QString Expression::getFunctionName() const
{
    return mFunction;
}


//! @brief Returns the symbol name, or an empty string is this is not a symbol
//FIXED
QString Expression::getSymbolName() const
{
     return mString;
}


//! @brief Returns the specified function argument, or an empty string if this is not a function or if it has too few arguments
//! @param idx Index of argument to return
//FIXED
Expression Expression::getArgument(const int idx) const
{
    if(mArguments.size() > idx)
    {
        return mArguments[idx];
    }
    return Expression();
}


//! @brief Returns a list of function arguments, or an empty list if this is not a function
//FIXED
QList<Expression> Expression::getArguments() const
{
    return mArguments;
}


//! @brief Internal function, returns a list with all terms
//FIXED
QList<Expression> Expression::getTerms() const
{
    if(this->isAdd())
    {
        return mTerms;
    }
    else
    {
        return QList<Expression>() << *this;
    }
}

//! @brief Internal function, returns a list with all divisors
//FIXED
QList<Expression> Expression::getDivisors() const
{
    return mDivisors;
}

//! @brief Internal function, returns a list with all factors
//FIXED
QList<Expression> Expression::getFactors() const
{
    if(this->isMultiplyOrDivide())
    {
        return mFactors;
    }
    else
    {
        return QList<Expression>() << *this;
    }
}

//! @brief Internal function, returns a list with all exponential bases
//FIXED
Expression *Expression::getBase() const
{
    return mpBase;
}

//! @brief Internal function, returns a list with all powers
//FIXED
Expression *Expression::getPower() const
{
    return mpPower;
}

//! @brief Internal function, returns a list with all left sides
//FIXED
Expression *Expression::getLeft() const
{
    return mpLeft;
}

//! @brief Internal function, returns a list with all right sides
//FIXED
Expression *Expression::getRight() const
{
    return mpRight;
}

//! @brief Internal function, returns a list with all dividends
//FIXED
Expression *Expression::getDividends() const
{
    return mpDividend;
}




//! @brief Removes all divisors in the expression
//FIXED
void Expression::removeDivisors()
{
    mDivisors.clear();
}


//! @brief Removes specified factor in the expression
//! @param var Factor to remove
//FIXED
void Expression::removeFactor(const Expression var)
{
    if(*this == var)
    {
        this->replaceBy(Expression(1));
        return;
    }

    mFactors.removeOne(var);
    return;
}


//! @brief Replaces all expressions specified with a new expression
//! @param oldExpr Old expression to replace
//! @param newExpr New expression
//FIXED
void Expression::replace(const Expression oldExpr, const Expression newExpr)
{
    if(*this == oldExpr)
    {
        this->replaceBy(newExpr);
    }
    else
    {
        if(this->isAdd())
        {
            for(int i=0; i<mTerms.size(); ++i)
            {
                mTerms[i].replace(oldExpr, newExpr);
            }
        }
        else if(this->isEquation())
        {
            mpLeft->replace(oldExpr, newExpr);
            mpRight->replace(oldExpr, newExpr);
        }
        else if(this->isMultiplyOrDivide())
        {
            QList<Expression>::iterator it;
            for(it=mFactors.begin(); it!=mFactors.end(); ++it)
            {
                (*it).replace(oldExpr, newExpr);
            }
            for(it=mDivisors.begin(); it!=mDivisors.end(); ++it)
            {
                (*it).replace(oldExpr, newExpr);
            }
        }
        else if(this->isPower())
        {
            mpBase->replace(oldExpr, newExpr);
            mpPower->replace(oldExpr, newExpr);
        }
        else if(this->isFunction())
        {
            for(int a=0; a<mArguments.size(); ++a)
            {
                mArguments[a].replace(oldExpr, newExpr);
            }
        }

    }
    return;
}


//! @brief Expands all parentheses in the expression
//! @param simplifications Specifies the degree of simplification
//FIXED
void Expression::expand(const ExpressionSimplificationT simplifications)
{
    if(!this->isMultiplyOrDivide()) { return; }

    QList<Expression>::iterator it;
    while(mFactors.size() > 1)
    {
        Expression factor1, factor2;

        //Extract the first two factors in the factor set
        QList<Expression> extractedSet;
        it = mFactors.begin();
        factor1 = (*it);
        ++it;
        factor2 = (*it);

        //Subtract the extracted elements from the original set
        mFactors.removeOne(factor1);
        mFactors.removeOne(factor2);

        //Generate a set of terms for each extracted factor
        QList<Expression> terms1, terms2;
        terms1 = factor1.getTerms();
        terms2 = factor2.getTerms();

        //Multiply each term in the first set with each term in the second set
        QList<Expression> multipliedTerms;
        Q_FOREACH(const Expression &exp1, terms1)
        {
            Q_FOREACH(const Expression &exp2, terms2)
            {
                multipliedTerms << Expression::fromFactorsDivisors(QList<Expression>() << exp1 << exp2, QList<Expression>());    //Multiplication!
            }
        }

        //Re-insert the multiplied term into the original factor set
        mFactors << Expression::fromTerms(multipliedTerms);
    }

    _simplify(simplifications);
}


//! @brief Linearizes the expression by multiplying with all divisors until no divisors remains
//! @note Should only be used on equations (obviously)
//FIXED
void Expression::linearize()
{
    assert(this->isEquation());

    this->expand();

    //Generate list with terms from both left and right side
    QList<Expression> allTerms;
    allTerms << mpLeft->getTerms() << mpRight->getTerms();

    //Generate list with all divisors from all terms
    QList<Expression> allDivisors;
    Q_FOREACH(const Expression &term, allTerms)
    {
        QList<Expression> divisors = term.getDivisors();
        Q_FOREACH(const Expression &exp, divisors)
        {
            if(!allDivisors.contains(exp))
            {
                allDivisors << exp;
            }
            else
            {
                while(allDivisors.count(exp) < divisors.count(exp))     //Make sure the all divisors vector contains at least the same amount of each divisor as the term
                {
                    allDivisors << exp;
                }
            }
        }
    }

    //Multiply each term with each divisor (on both sides)
    QList<Expression> leftTerms = mpLeft->getTerms();
    for(int i=0; i<leftTerms.size(); ++i)
    {
        leftTerms[i].replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << leftTerms[i].getFactors() << allDivisors, leftTerms[i].getDivisors()));
    }
    mpLeft->replaceBy(Expression::fromTerms(leftTerms));
    QList<Expression> rightTerms = mpRight->getTerms();
    for(int i=0; i<rightTerms.size(); ++i)
    {
        rightTerms[i].replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << rightTerms[i].getFactors() << allDivisors, rightTerms[i].getDivisors()));
    }
    mpRight->replaceBy(Expression::fromTerms(rightTerms));

    _simplify(FullSimplification, Recursive);

    return;
}


//! @brief Moves all right side expressions to the left side, if this is an equation
//FIXED
void Expression::toLeftSided()
{
    assert(this->isEquation());
    mpLeft->replaceBy(Expression::fromTerms(QList<Expression>() << mpLeft->getTerms() << mpRight->getTerms()));
    (*mpRight) = Expression("0");
}


//! @brief Factors specified expression
//! @param var Expression to factor
//! @example Factorizing "a*b*c + a*c*d + b*c*d" for "b" => "b*(a*c + c*d) + a*c*d"
//FIXED
void Expression::factor(const Expression var)
{
    QList<Expression> terms = getTerms();
    QList<Expression> termsWithVar, termsWithoutVar;

    for(int i=0; i<terms.size(); ++i)
    {
        if(terms[i] == var || terms[i].getFactors().contains(var))
        {
            terms[i].removeFactor(var);
            termsWithVar << terms[i];
        }
        else
        {
            termsWithoutVar << terms[i];
        }
    }

    if(!termsWithVar.isEmpty())
    {
        Expression termsWithVarExpr = Expression::fromTerms(termsWithVar);
        Expression factoredTerm = Expression::fromFactorsDivisors(QList<Expression>() << var << termsWithVarExpr, QList<Expression>());

        this->replaceBy(Expression::fromTerms(QList<Expression>() << factoredTerm << termsWithoutVar));

        this->_simplify(Expression::FullSimplification, Recursive);
    }
}


//! @brief Factors the most common factor in the expression
//FIXED
void Expression::factorMostCommonFactor()
{
    if(!isAdd())
    {
        return;
    }

    QList<Expression> symbols;
    QList<int> counter;
    QList<Expression> terms = this->getTerms();
    for(int t=0; t<terms.size(); ++t)
    {
        QList<Expression> factors = terms[t].getFactors();
        for(int f=0; f<factors.size(); ++f)
        {
            if(factors[f] == Expression(-1.0) || factors[f] == Expression(1.0))       //Ignore "-1.0" and "1.0"
            {
                continue;
            }
            if(symbols.contains(factors[f]))
            {
                int idx = symbols.indexOf(factors[f]);
                counter[idx] = counter[idx]+1;
            }
            else
            {
                symbols.append(factors[f]);
                counter.append(1);
            }
        }
    }

    int max=0;
    Expression mostCommon;
    for(int s=0; s<symbols.size(); ++s)
    {
        if(counter[s] > max)
        {
            max = counter[s];
            mostCommon = symbols[s];
        }
    }

    if(max>1)
    {
        factor(mostCommon);
    }

    return;
}




//! @brief Internal function, returns the contained string
//FIXED
QString Expression::_getString() const
{
    return mString;
}


//! @brief Verifies that the expression is correct
//FIXED
bool Expression::verifyExpression()
{

    //Verify all functions
    if(!_verifyFunctions())
    {
        return false;
    }

    return true;
}


//! @brief Verifies that all functions are supported
//FIXED
bool Expression::_verifyFunctions() const
{
    bool success = true;

    QStringList functions = this->getFunctions();
    for(int i=0; i<functions.size(); ++i)
    {
        if(!getSupportedFunctionsList().contains(functions[i]) && !getCustomFunctionList().contains(functions[i]))
        {
            //QMessageBox::critical(0, "SymHop", "Function \""+functions[i]+"\" is not supported by component generator.");
            success = false;
        }
    }

    return success;
}


//! @brief Simplifies the expression
//! @param type Tells the degree of simplification to perform
//! @param recursive Tells whether or not children are to be recursively simplified
//! @note Recursion is not needed when creating new expressions, since the creator recurses all children anyway.
//FIXED
void Expression::_simplify(ExpressionSimplificationT type, const ExpressionRecursiveT recursive)
{
    if(type == NoSimplifications)
    {
        return;
    }

    if(recursive == Recursive)
    {
        for(int i=0; i<mArguments.size(); ++i)
        {
            mArguments[i]._simplify(type, recursive);
        }
        for(int i=0; i<mTerms.size(); ++i)
        {
            mTerms[i]._simplify(type, recursive);
        }
        for(int i=0; i<mFactors.size(); ++i)
        {
            mFactors[i]._simplify(type, recursive);
        }
        for(int i=0; i<mDivisors.size(); ++i)
        {
            mDivisors[i]._simplify(type, recursive);
        }
        mpBase->_simplify(type, recursive);
        mpPower->_simplify(type, recursive);
        mpLeft->_simplify(type, recursive);
        mpRight->_simplify(type, recursive);
    }

    //Trivial simplifications
    if(this->isMultiplyOrDivide())
    {
        mFactors.removeAll(Expression(1.0));    //Replace 1*x with x

        if(mFactors.contains(Expression("0")))
        {
            replaceBy(Expression(0.0));     //Replace "0*x" and "x*0" with "0.0"
        }

        int nNeg = mFactors.count(Expression("-1"))+mDivisors.count(Expression("-1"));        //Remove unnecessary negatives
        if(nNeg > 1)
        {
            mFactors.removeAll(Expression("-1"));
            mDivisors.removeAll(Expression("-1"));
            if(nNeg % 2 != 0)
            {
                mFactors << Expression("-1");
            }
        }
    }
    else if(this->isAdd())
    {
        mTerms.removeAll(Expression("0.0"));
    }
    else if(this->isPower())
    {
        if(mpPower->toDouble(new bool) == 1.0)
        {
            replaceBy(*mpBase);
        }
        else if(mpPower->isNegative())
        {
            mpPower->changeSign();
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("1"), QList<Expression>() << (*this)));
        }
        else        //Replace with number if both base and exponent are numericals
        {
            if(mpBase->isNumericalSymbol() && mpPower->isNumericalSymbol())
            {
                double value = pow(mpBase->toDouble(),mpPower->toDouble());
                this->replaceBy(Expression(QString::number(value)));
            }
        }
    }

    if(type == TrivialSimplifications)      //End of trivial simplifications, return if this was specified
    {
        return;
    }


    if(isAdd())
    {
        //Sum up all numericals to one term
        double value=0;
        bool foundOne=false;
        for(int i=0; i<mTerms.size(); ++i)
        {
            if(mTerms[i].isNumericalSymbol())
            {
                value += mTerms[i].toDouble();
                mTerms.removeAt(i);
                --i;
                foundOne = true;
            }
        }
        if(foundOne)
        {
            mTerms << Expression(QString::number(value));
        }

        mTerms.removeAll(Expression("0.0"));

        return;
    }

    if(this->isMultiplyOrDivide())
    {
        //Multiply all numericals together (except for -1)
        double value = 1;
        bool foundOne=false;
        for(int i=0; i<mFactors.size(); ++i)
        {
            if(mFactors[i].isNumericalSymbol() && !(mFactors[i] == Expression("-1")))
            {
                value *= mFactors[i].toDouble();
                mFactors.removeAt(i);
                --i;
                foundOne=true;
            }
        }
        for(int i=0; i<mDivisors.size(); ++i)
        {
            if(mDivisors[i].isNumericalSymbol() && !(mDivisors[i] == Expression("-1")))
            {
                value /= mDivisors[i].toDouble();
                mDivisors.removeAt(i);
                --i;
                foundOne = true;
            }
        }
        if(foundOne)
        {
            mFactors << Expression(QString::number(value));
        }

        //Expand power functions in factors if power is int
        for(int i=0; i<mFactors.size(); ++i)
        {
            if(mFactors[i].isPower() && mFactors[i].getPower()->isNumericalSymbol() && isWhole(mFactors[i].getPower()->toDouble()))
            {
                int n = int(mFactors[i].getPower()->toDouble());
                for(int j=0; j<n; ++j)
                {
                    mFactors << (*mFactors[i].getBase());
                }
                mFactors.removeAt(i);
                --i;
            }
        }

        //Expand power functions in divisors if power is int
        for(int i=0; i<mDivisors.size(); ++i)
        {
            if(mDivisors[i].isPower() && isWhole(mDivisors[i].getPower()->toDouble()))
            {
                int n = int(mDivisors[i].getPower()->toDouble());
                for(int j=0; j<n; ++j)
                {
                    mDivisors << (*mDivisors[i].getBase());
                }
                mDivisors.removeAt(i);
                --i;
            }
        }

        //Cancel out same factors and divisors
        restart:
        Q_FOREACH(const Expression &factor, mFactors)
        {
            if(mDivisors.contains(factor))
            {
                mDivisors.removeOne(factor);
                mFactors.removeOne(factor);
                goto restart;
            }
        }
    }
}


//FIXED
bool Expression::splitAtSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    if(subSymbols.contains(sep))
    {
        if(sep == "=")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "=")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            mpLeft = new Expression(left);
            mpRight = new Expression(right);
        }
        else if(sep == "+")
        {
            QStringList term;
            bool negative=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "+")
                {
                    if(negative) { term << "*" << "-1"; }
                    mTerms.append(Expression(term, simplifications));
                    term.clear();
                    negative == false;
                }
                else if(subSymbols[i] == "-")
                {
                    if(negative) { term << "*" << "-1"; }
                    mTerms.append(Expression(term, simplifications));
                    term.clear();
                    negative = true;
                }
                else
                {
                    term.append(subSymbols[i]);
                }
            }
            mTerms.append(Expression(term, simplifications));
        }
        else if(sep == "*")
        {
            QStringList factorOrDiv;
            bool div = false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "*")
                {
                    if(div)
                    {
                        mDivisors.append(Expression(factorOrDiv, simplifications));
                    }
                    else
                    {
                        mFactors.append(Expression(factorOrDiv, simplifications));
                    }
                    factorOrDiv.clear();
                    div = false;
                }
                else if(subSymbols[i] == "/")
                {
                    if(div)
                    {
                        mDivisors.append(Expression(factorOrDiv, simplifications));
                    }
                    else
                    {
                        mFactors.append(Expression(factorOrDiv, simplifications));
                    }
                    factorOrDiv.clear();
                    div = true;
                }
                else
                {
                    factorOrDiv << subSymbols[i];
                }
            }
            if(div)
            {
                mDivisors.append(Expression(factorOrDiv, simplifications));
            }
            else
            {
                mFactors.append(Expression(factorOrDiv, simplifications));
            }
        }
        else if(sep == "^")
        {
            bool inPower=false;
            QStringList base;
            QStringList power;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "^")
                {
                    inPower = true;
                }
                else if(!inPower)
                {
                    base.append(subSymbols[i]);
                }
                else
                {
                    power.append(subSymbols[i]);
                }
            }
            mpBase = new Expression(base);
            mpPower = new Expression(power);
        }
        else if(sep == "%")
        {
            bool inDivisor=false;
            QStringList dividend;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "%")
                {
                    inDivisor = true;
                }
                else if(!inDivisor)
                {
                    dividend.append(subSymbols[i]);
                }
                else
                {
                    mDivisors.append(subSymbols[i]);
                }
            }
            mpDividend = new Expression(dividend);
        }
        return true;
    }

    return false;
}


//! @brief Returns a list with supported functions for equation-based model genereation
//FIXED
QStringList SymHop::getSupportedFunctionsList()
{
    return QStringList() << "div" << "rem" << "mod" << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "atan2" << "sinh" << "cosh" << "tanh" << "log" << "exp" << "sqrt" << "sign" << "abs" << "der" << "onPositive" << "onNegative" << "signedSquareL" << "limit" << "integer" << "floor" << "ceil" << "pow";
}


//! @brief Returns a list of custom Hopsan functions that need to be allowed in the symbolic library
//FIXED
QStringList SymHop::getCustomFunctionList()
{
    return QStringList() << "hopsanLimit" << "hopsanDxLimit" << "onPositive" << "onNegative" << "signedSquareL" << "limit";
}


//! @brief Finds the first path through a matrix of dependencies, used to sort jacobian matrixes
//FIXED
bool SymHop::findPath(QList<int> &order, QList<QList<int> > dependencies, int level)
{
    if(level > dependencies.size()-1)
    {
        return true;
    }

    for(int i=0; i<dependencies.at(level).size(); ++i)
    {
        if(!order.contains(dependencies.at(level).at(i)))
        {
            order.append(dependencies.at(level).at(i));
            if(findPath(order, dependencies, level+1))
            {
                return true;
            }
            order.removeLast();
        }
    }
    return false;
}


//! @brief Sorts an equation system so that all diagonal elements in the jacobian matrix are non-zero
//! @param equation List of system equations
//! @param symbols List of all variables
//! @param stateVars List of state variables
//! @param limitedVariableEquations Reference to list with indexes for limitation functions
//! @param limitedDerivativeEquations Reference to list with indexes for derivative limitation functions
//FIXED
bool SymHop::sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations)
{
    qDebug() << "Jacobian:";
    for(int i=0; i<jacobian.size(); ++i)
    {
        QString line;
        for(int j=0; j<jacobian.size(); ++j)
        {
            line.append(jacobian[i][j].toString());
            line.append("  ");
        }
        qDebug() << line;
    }

    //Generate a dependency tree between equations and variables
    Expression zero = Expression(0.0);
    QList<QList<int> > dependencies;
    for(int v=0; v<stateVars.size(); ++v)
    {
        dependencies.append(QList<int>());
        for(int e=0; e<stateVars.size(); ++e)
        {
            if(!(jacobian[e][v] == zero))
            {
                dependencies[v].append(e);
            }
        }
    }

    //Recurse dependency tree to find a good sorting order
    QList<int> order;
    if(!findPath(order, dependencies))
    {
        return false;
    }

    //Sort equations to new order
    QList<Expression> sortedEquations;
    QList<QList<Expression> > sortedJacobian;
    for(int i=0; i<order.size(); ++i)
    {
        sortedEquations.append(equations.at(order[i]));
        sortedJacobian.append(jacobian.at(order[i]));
        for(int j=0; j<limitedVariableEquations.size(); ++j)    //Sort limited variable equation numbers
        {
            if(limitedVariableEquations[j] == i)
            {
                limitedVariableEquations[j] = order[i];
            }
        }
        for(int j=0; j<limitedDerivativeEquations.size(); ++j)  //Sort limited derivative equation numbers
        {
            if(limitedDerivativeEquations[j] == i)
            {
                limitedDerivativeEquations[j] = order[i];
            }
        }
    }

    jacobian = sortedJacobian;
    equations = sortedEquations;

    return true;
}


//! @brief Removes all duplicates in a list of expressions
//! @param rList Reference to the list
//FIXED
void SymHop::removeDuplicates(QList<Expression> &rSet)
{
    QList<Expression> tempSet;
    Q_FOREACH(const Expression &item, rSet)
    {
        if(!tempSet.contains(item))
        {
            tempSet.append(item);
        }
    }

    rSet = tempSet;
}


//FIXED
bool SymHop::isWhole(const double value)
{
    return (static_cast<int>(value) == value);
}


//! @brief Checks whether or no the parentheses are correct in a string
//! @param str String to verify
//FIXED
bool Expression::verifyParantheses(const QString str) const
{
    int balance = 0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '(')
        {
            ++balance;
        }
        else if(str[i] == ')')
        {
            --balance;
        }
        if(balance<0) { return false; }
    }
    return (balance==0);
}


//! @brief Splits a string at specified character, but does not split inside parentheses
//! @param str String to split
//! @param c Character to split at
//FIXED
QStringList Expression::splitWithRespectToParentheses(const QString str, const QChar c)
{
    QStringList ret;
    int parBal=0;
    int start=0;
    int len=0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '(')
        {
            ++parBal;
        }
        else if(str[i] == ')')
        {
            --parBal;
        }
        else if(str[i] == c && parBal == 0)
        {
            ret.append(str.mid(start,len));
            start=start+len+1;
            len=-1;
        }
        ++len;
    }
    ret.append(str.mid(start,len));
    return ret;
}

