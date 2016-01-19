#include "tokenizer.h"
#include "compiler_state.h"
#include <string.h>
#include <iostream>

Tokenizer::Tokenizer(TILCompiler* compiler, CompilerState* currentState)
: m_compiler(compiler), m_currentState(currentState)
{
    m_reservedWords.push_back("application");
    m_reservedWords.push_back("plugin");
    
    m_reservedWords.push_back("module");
    m_reservedWords.push_back("int");
    m_reservedWords.push_back("string");
    m_reservedWords.push_back("bool");
    m_reservedWords.push_back("float");
    m_reservedWords.push_back("void");
    
    m_reservedWords.push_back("//");
    m_reservedWords.push_back("/*");
    m_reservedWords.push_back("*/");
}

bool Tokenizer::ProcessToken(char token)
{
    typeToken type = GetType(token);
    if(type == typeSymbol)
    {
        if(!m_lastReserved.empty())
        {
            m_lastString += m_lastReserved;
            m_lastReserved.clear();
        }
        
        m_lastString.push_back(token);
    }
    else if(type == typeReserved)
    {
        m_lastReserved.push_back(token);
        CompilerState::StateStatus state = m_currentState->IsNextState(m_lastReserved);
        if(state == CompilerState::StateApply)
        {
            m_currentState = m_currentState->NextState(m_lastString);
            m_lastReserved.clear();
        }
        else if(state == CompilerState::StateIgnore)
        {
            m_lastReserved.clear();
        }
    }
    else if(type == typeSeporator)
    {
        if(!m_lastReserved.empty())
        {
            m_lastString += m_lastReserved;
            m_lastReserved.clear();
        }
        
        CompilerState::StateStatus state = m_currentState->IsNextState(token);
        if(state == CompilerState::StateApply)
        {
            m_currentState = m_currentState->NextState(m_lastString);
            m_lastString.clear();
        }
        else if(state == CompilerState::StateContinue)
        {
            m_lastString.push_back(token);
        }
            
    }
    
    if(!m_currentState)
    {
        return false;
    }
    return true;
}

Tokenizer::typeToken Tokenizer::GetType(char ch)
{
    switch(ch)
    {
        case ' ':
        case '\r':
        case '\n':
        case ',':
        case ';':
        case '{':
        case '}':
        case '(':
        case ')':
            return typeSeporator;
    }
    
    std::string data = m_lastReserved + ch;
    for(std::vector<std::string>::iterator it = m_reservedWords.begin();
        it != m_reservedWords.end(); it++)
    {
        if(data.size() <= it->size() && memcmp(data.c_str(), it->c_str(), data.size()) == 0)
            return typeReserved;
    }

    return typeSymbol;
}