#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>

class SlkToken;
namespace FunctionScript
{
    class ActionApi;
    class StatementData;
    class FunctionData;
}

class ParserFineTuneHelper final
{
public:
    enum PairTagEnum
    {
        PAIR_TAG_IF = 1,
        PAIR_TAG_ELSEIF = 2,
        PAIR_TAG_ELIF = 3,
        PAIR_TAG_WHILE = 4,
        PAIR_TAG_FOR = 5,
        PAIR_TAG_FOREACH = 6,
        PAIR_TAG_LOOP = 7,
        PAIR_TAG_LOOPLIST = 8,
        PAIR_TAG_LAST = 9
    };
public:
    std::unordered_map<std::string, uint32_t>& NameTags()
    {
        return m_NameTags;
    }
    const std::unordered_map<std::string, uint32_t>& NameTags()const
    {
        return m_NameTags;
    }
    std::unordered_map<std::string, std::string>& FirstLastKeyOfCompoundStatements()
    {
        return m_FirstLastKeyOfCompoundStatements;
    }
    const std::unordered_map<std::string, std::string>& FirstLastKeyOfCompoundStatements()const
    {
        return m_FirstLastKeyOfCompoundStatements;
    }
    std::unordered_map<std::string, std::unordered_set<std::string>>& SuccessorsOfCompoundStatements()
    {
        return m_SuccessorsOfCompoundStatements;
    }
    const std::unordered_map<std::string, std::unordered_set<std::string>>& SuccessorsOfCompoundStatements()const
    {
        return m_SuccessorsOfCompoundStatements;
    }
    std::unordered_set<std::string>& CompoundStatements()
    {
        return m_CompoundStatements;
    }
    const std::unordered_set<std::string>& CompoundStatements()const
    {
        return m_CompoundStatements;
    }
public:
    ParserFineTuneHelper();
public:
    bool OnGetToken(const FunctionScript::ActionApi& actionApi, SlkToken& tokenApi, char*& tok, short& val, int& line)const;
    bool OnBeforeAddFunction(const FunctionScript::ActionApi& api, FunctionScript::StatementData* sd)const;
    bool OnSetFunctionId(const FunctionScript::ActionApi& api, const char* name, FunctionScript::StatementData* sd, FunctionScript::FunctionData* fd)const;
private:
    std::unordered_map<std::string, uint32_t> m_NameTags;
    std::unordered_map<std::string, std::string> m_FirstLastKeyOfCompoundStatements;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_SuccessorsOfCompoundStatements;
    std::unordered_set<std::string> m_CompoundStatements;
public:
    static ParserFineTuneHelper& ForSimpleScript();
private:
    static ParserFineTuneHelper* s_pForSimpleScript;
};
