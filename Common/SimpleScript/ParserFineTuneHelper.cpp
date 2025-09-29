#include "ParserFineTuneHelper.h"
#include "SlkToken.h"
#include "calc.h"

ParserFineTuneHelper* ParserFineTuneHelper::s_pForSimpleScript = nullptr;

ParserFineTuneHelper& ParserFineTuneHelper::ForSimpleScript()
{
    if (nullptr == s_pForSimpleScript) {
        s_pForSimpleScript = new ParserFineTuneHelper();

    }
    return *s_pForSimpleScript;
}

ParserFineTuneHelper::ParserFineTuneHelper() :
    m_NameTags{
        { "if", PAIR_TAG_IF },
        { "elseif", PAIR_TAG_ELSEIF },
        { "elif", PAIR_TAG_ELIF },
        { "while", PAIR_TAG_WHILE },
        { "for", PAIR_TAG_FOR },
        { "foreach", PAIR_TAG_FOREACH },
        { "loop", PAIR_TAG_LOOP },
        { "looplist", PAIR_TAG_LOOPLIST }
},
m_FirstLastKeyOfCompoundStatements{
      { "if", "else" },
      { "while", "while" },
      { "for", "for" },
      { "foreach", "foreach" },
      { "loop", "loop" },
      { "loopi", "loopi" },
      { "loopd", "loopd" },
      { "looplist", "looplist" },
      { "struct", "struct" },
      { "literalarray", "literalarray" },
      { "literalobject", "literalobject" }
},
m_SuccessorsOfCompoundStatements{
    {
        "if", { "elseif", "elif", "else" }
    },
    {
        "elseif", { "elseif", "elif", "else" }
    },
    {
        "elif", { "elseif", "elif", "else" }
    },
    {
        "else", {}
    },
    {
        "while", {}
    },
    {
        "for", {}
    },
    {
        "foreach", {}
    },
    {
        "loop", {}
    },
    {
        "looplist", {}
    },
    {
        "struct", {}
    },
    {
        "literalarray", {}
    },
    {
        "literalobject", {}
    },
    {
        "function", { "args" }
    }
},
m_CompoundStatements{
        "if", "while", "for", "foreach", "loop", "looplist", "struct", "literalarray", "literalobject", "function"
}
{
}

bool ParserFineTuneHelper::OnGetToken([[maybe_unused]] const FunctionScript::ActionApi& actionApi, SlkToken& tokenApi, char*& tok, [[maybe_unused]] short& val, int& line)const
{
    if (0 == strcmp(tok, "return")) {
        char* oldCurTok = tokenApi.getCurToken();
        char* oldLastTok = tokenApi.getLastToken();
        int index;
        char nc = tokenApi.peekNextValidChar(0, index);
        if (nc == ';')
            return false;
        tokenApi.setCurToken(const_cast<char*>("`"));
        tokenApi.setLastToken(oldCurTok);
        tokenApi.enqueueToken(tokenApi.getCurToken(), tokenApi.getOperatorTokenValue(), line);
        tokenApi.setCurToken(oldCurTok);
        tokenApi.setLastToken(oldLastTok);
        return true;
    }
    else if (0 == strcmp(tok, ")")) {
        uint32_t tag;
        if (actionApi.peekPairTypeStack(tag) == FunctionScript::FunctionData::PAIR_TYPE_PARENTHESES) {
            if (tag > 0) {
                char* oldCurTok = tokenApi.getCurToken();
                char* oldLastTok = tokenApi.getLastToken();
                int index;
                char nc = tokenApi.peekNextValidChar(0, index);
                if (nc == '{' || nc == ',' || nc == ';')
                    return false;
                //insert backtick char
                tokenApi.setCurToken(const_cast<char*>("`"));
                tokenApi.setLastToken(oldCurTok);
                tokenApi.enqueueToken(tokenApi.getCurToken(), tokenApi.getOperatorTokenValue(), line);
                tokenApi.setCurToken(oldCurTok);
                tokenApi.setLastToken(oldLastTok);
                return true;
            }
        }
    }
    return false;
}

bool ParserFineTuneHelper::OnBeforeAddFunction(const FunctionScript::ActionApi& api, FunctionScript::StatementData* sd)const
{
    //You can end the current statement here and start a new empty statement.
    (void)api, (void)sd;
    const char* fid = sd->GetId();
    auto* func = sd->GetLastFunctionRef();
    if (nullptr != func) {
        const char* lid = func->GetId();
        if (func->HaveStatement()) {
            bool match = false;
            if (fid && fid[0]) {
                auto&& it = m_FirstLastKeyOfCompoundStatements.find(fid);
                if (it != m_FirstLastKeyOfCompoundStatements.end() && it->second == lid) {
                    match = true;
                }
            }
            if (nullptr == fid || fid[0] == 0 || match) {
                //End the current statement and start a new empty statement.
                api.endStatement();
                api.beginStatement();
                return true;
            }
        }
    }
    return false;
}

bool ParserFineTuneHelper::OnSetFunctionId(const FunctionScript::ActionApi& api, const char* name, FunctionScript::StatementData* sd, FunctionScript::FunctionData* fd)const
{
    //The statement can be split here.
    (void)api, (void)name, (void)sd, (void)fd;
    auto* sid = sd->GetId();
    auto* func = sd->GetLastFunctionRef();
    if (nullptr != func && sd->GetFunctionNum() > 1) {
        bool match = false;
        if (sid && sid[0]) {
            auto it = m_SuccessorsOfCompoundStatements.find(sid);
            if (it != m_SuccessorsOfCompoundStatements.end() && it->second.find(name) == it->second.end()) {
                match = true;
            }
        }
        if (match) {
            sd->RemoveLastFunction();
            api.endStatement();
            api.beginStatement();
            auto* stm = api.getCurStatement();
            stm->AddFunction(func);
            return true;
        }
        else if (m_CompoundStatements.find(name) != m_CompoundStatements.end()) {
            sd->RemoveLastFunction();
            api.endStatement();
            api.beginStatement();
            auto* stm = api.getCurStatement();
            stm->AddFunction(func);
            return true;
        }
    }
    return false;
}
